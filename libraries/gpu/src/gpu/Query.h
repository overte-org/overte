//
//  Query.h
//  interface/src/gpu
//
//  Created by Niraj Venkat on 7/7/2015.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Query_h
#define hifi_gpu_Query_h

#include <assert.h>
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <SimpleMovingAverage.h>

#include "Format.h"

namespace gpu {

    class Batch;

    /**
     * Query is used for measuring performance on GPU side.
     */
    class Query {
    public:
        using Handler = std::function<void(const Query&)>;

        /**
         * @param returnHandler Function that is called when the query readout is ready.
         * @param name Name of the query.
         */
        Query(const Handler& returnHandler, const std::string& name = "gpu::query");
        ~Query();

        /**
         * @return GPU elapsed time in milliseconds.
         */
        double getGPUElapsedTime() const;

        /**
         * Batch time is time measured between `do_beginQuery` and `do_endQuery` measured when the batch is executed.
         * @return Batch elapsed time in milliseconds.
         */
        double getBatchElapsedTime() const;

        /**
         * @return Name of the query.
         */
        const std::string& getName() const { return _name; }

        /// Only for gpu::Context
        const GPUObjectPointer gpuObject {};

        /**
         * @brief Saves query results and calls the callback function.
         *
         * @param queryResult GPU time in nanoseconds.
         * @param batchElapsedTime Batch time in nanoseconds.
         */
        void triggerReturnHandler(uint64_t queryResult, uint64_t batchElapsedTime);
    protected:

        /// Function to call when query results are available.
        Handler _returnHandler;

        /// Name of the query.
        const std::string _name;

        /// Query result - GPU elapsed time in nanoseconds.
        uint64_t _queryResult { 0 };

        /// Batch time elapsed in nanoseconds.
        uint64_t _nsecBatchElapsedTime { 0 };

        friend class Serializer;
        friend class Deserializer;
    };
    
    typedef std::shared_ptr<Query> QueryPointer;
    typedef std::vector< QueryPointer > Queries;


    /**
     * gpu RangeTimer is just returning an estimate of the time taken by a chunk of work delimited by the
     * begin and end calls repeated for several times.
     * The result is always a late average of the time spent for that same task a few cycles ago.
     */
    class RangeTimer {
    public:
        RangeTimer(const std::string& name);

        /**
         * @brief Adds a batch command to begin query.
         *
         * @param batch Batch to add the command to.
         */
        void begin(gpu::Batch& batch);

        /**
         * @brief Adds a batch command to get query.
         *
         * @param batch Batch to add the command to.
         */
        void end(gpu::Batch& batch);

        /**
         * @return Get GPU average time in milliseconds.
         */
        double getGPUAverage() const;

        /**
         * Batch time is time measured between `do_beginQuery` and `do_endQuery`.
         * @return Get batch average time in milliseconds.
         */
        double getBatchAverage() const;

        /**
         * @return Name of this range timer.
         */
        const std::string& name() const { return _name; }

    protected:

        /// Number of query objects in this range timer.
        static const int QUERY_QUEUE_SIZE { 4 };

        /// Name of the range timer.
        const std::string _name;

        /// Query objects used for this range timer.
        gpu::Queries _timerQueries;

        /// Used for circular buffer. Index of the last query that was executed can be calculated from this using `rangeIndex()`.
        int _headIndex = -1;

        /// Used for circular buffer. Index of the last query that was executed can be calculated from this using `rangeIndex()`.
        /// Incremented by a Query object when it's ready for reading.
        int _tailIndex = -1;

        /// Average of the time measured on GPU in millseconds.
        MovingAverage<double, QUERY_QUEUE_SIZE * 2> _movingAverageGPU;

        /// Average of the time between `do_beginQuery` and `do_endQuery` measured when the batch is executed.
        MovingAverage<double, QUERY_QUEUE_SIZE * 2> _movingAverageBatch;

        /// Calculates index in the circular buffer where the queries are stored.
        int rangeIndex(int index) const { return (index % QUERY_QUEUE_SIZE); }
    };
    
    using RangeTimerPointer = std::shared_ptr<RangeTimer>;
    
};

#endif
