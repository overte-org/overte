//
//  Metric.h
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 5/17/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Metric_h
#define hifi_gpu_Metric_h

#include "Forward.h"

namespace gpu {
/**
  * Used for simple, thread-safe statistics such as counting resources.
  * @tparam T Data type for the metric. Common ones are `uint32_t` and `size_t`.
  */
template <typename T>
class Metric {
    std::atomic<T> _value { 0 };
    std::atomic<T> _maximum { 0 };

public:
    /**
     * @return Returns the current value of the metric.
     */
    T getValue() { return _value; }

    /**
     * @return Returns maximum value that the metric had.
     */
    T getMaximum() { return _maximum; }

    /**
     * @brief Set metric to a new value.
     *
     * Does not reset or update maximum value.
     * @param newValue New value.
     */
    void set(T newValue) {
        _value = newValue;
    }

    /**
     * @brief Increment stored value by one.
     */
    void increment() {
        auto total = ++_value;
        if (total > _maximum.load()) {
            _maximum = total;
        }
    }

    /**
     * @brief Decrement stored value by one.
     */
    void decrement() {
        --_value;
    }

    /**
     * @brief Atomically subtract previous value from the metric and add new one.
     *
     * Update maximum value if needed.
     * @param prevValue Previous value.
     * @param newValue New value.
     */
    void update(T prevValue, T newValue) {
        if (prevValue == newValue) {
            return;
        }
        if (newValue > prevValue) {
            auto total = _value.fetch_add(newValue - prevValue);
            if (total > _maximum.load()) {
                _maximum = total;
            }
        } else {
            _value.fetch_sub(prevValue - newValue);
        }
    }

    /**
     * @brief Resets current metric value and maximum value to zero.
     */
    void reset() {
        _value = 0;
        _maximum = 0;
    }
};

using ContextMetricCount = Metric<uint32_t>;
using ContextMetricSize = Metric<Size>;

}
#endif
