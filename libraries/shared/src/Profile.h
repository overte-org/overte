//
//  Created by Ryan Huffman on 2016-12-14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#ifndef HIFI_PROFILE_
#define HIFI_PROFILE_

#include "Trace.h"
#include "SharedUtil.h"

// When profiling something that may happen many times per frame, use a xxx_detail category so that they may easily be filtered out of trace results
Q_DECLARE_LOGGING_CATEGORY(trace_app)
Q_DECLARE_LOGGING_CATEGORY(trace_app_detail)
Q_DECLARE_LOGGING_CATEGORY(trace_metadata)
Q_DECLARE_LOGGING_CATEGORY(trace_network)
Q_DECLARE_LOGGING_CATEGORY(trace_picks)
Q_DECLARE_LOGGING_CATEGORY(trace_render)
Q_DECLARE_LOGGING_CATEGORY(trace_render_detail)
Q_DECLARE_LOGGING_CATEGORY(trace_render_gpu)
Q_DECLARE_LOGGING_CATEGORY(trace_resource)
Q_DECLARE_LOGGING_CATEGORY(trace_resource_parse)
Q_DECLARE_LOGGING_CATEGORY(trace_resource_network)
Q_DECLARE_LOGGING_CATEGORY(trace_script)
Q_DECLARE_LOGGING_CATEGORY(trace_script_entities)
Q_DECLARE_LOGGING_CATEGORY(trace_simulation)
Q_DECLARE_LOGGING_CATEGORY(trace_simulation_detail)
Q_DECLARE_LOGGING_CATEGORY(trace_simulation_animation)
Q_DECLARE_LOGGING_CATEGORY(trace_simulation_animation_detail)
Q_DECLARE_LOGGING_CATEGORY(trace_simulation_physics)
Q_DECLARE_LOGGING_CATEGORY(trace_simulation_physics_detail)
Q_DECLARE_LOGGING_CATEGORY(trace_startup)
Q_DECLARE_LOGGING_CATEGORY(trace_workload)
Q_DECLARE_LOGGING_CATEGORY(trace_baker)

class ProfileDurationBase {

protected:
    ProfileDurationBase(const QLoggingCategory& category, const QString& name);
    const QString _name;
    const QLoggingCategory& _category;
};

class ProfileDuration : public ProfileDurationBase {
public:
    ProfileDuration(const QLoggingCategory& category, const QString& name, uint32_t argbColor = 0xff0000ff, uint64_t payload = 0, const QVariantMap& args = QVariantMap());
    ~ProfileDuration();

    static uint64_t beginRange(const QLoggingCategory& category, const char* name, uint32_t argbColor);
    static void endRange(const QLoggingCategory& category, uint64_t rangeId);
};

class ConditionalProfileDuration : public ProfileDurationBase {
public:
    ConditionalProfileDuration(const QLoggingCategory& category, const QString& name, uint32_t minTime);
    ~ConditionalProfileDuration();

private:
    const int64_t _startTime;
    const int64_t _minTime;
};


inline void syncBegin(const QLoggingCategory& category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (category.isDebugEnabled()) {
        tracing::traceEvent(category, name, tracing::DurationBegin, id, args, extra);
    }
}


inline void syncEnd(const QLoggingCategory& category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (category.isDebugEnabled()) {
        tracing::traceEvent(category, name, tracing::DurationEnd, id, args, extra);
    }
}

inline void asyncBegin(const QLoggingCategory& category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (category.isDebugEnabled()) {
        tracing::traceEvent(category, name, tracing::AsyncNestableStart, id, args, extra);
    }
}


inline void asyncEnd(const QLoggingCategory& category, const QString& name, const QString& id, const QVariantMap& args = QVariantMap(), const QVariantMap& extra = QVariantMap()) {
    if (category.isDebugEnabled()) {
        tracing::traceEvent(category, name, tracing::AsyncNestableEnd, id, args, extra);
    }
}

inline void instant(const QLoggingCategory& category, const QString& name, const QString& scope = "t", const QVariantMap& args = QVariantMap(), QVariantMap extra = QVariantMap()) {
    if (category.isDebugEnabled()) {
        extra["s"] = scope;
        tracing::traceEvent(category, name, tracing::Instant, "", args, extra);
    }
}

inline void counter(const QLoggingCategory& category, const QString& name, const QVariantMap& args, const QVariantMap& extra = QVariantMap()) {
    if (category.isDebugEnabled()) {
        tracing::traceEvent(category, name, tracing::Counter, "", args, extra);
    }
}

inline void metadata(const QString& metadataType, const QVariantMap& args) {
    tracing::traceEvent(trace_metadata(), metadataType, tracing::Metadata, "", args);
}

#define PROFILE_RANGE(category, name) ProfileDuration profileRangeThis(trace_##category(), name);
#define PROFILE_RANGE_IF_LONGER(category, name, ms) ConditionalProfileDuration profileRangeThis(trace_##category(), name, ms);
#define PROFILE_RANGE_EX(category, name, argbColor, payload, ...) ProfileDuration profileRangeThis(trace_##category(), name, argbColor, (uint64_t)payload, ##__VA_ARGS__);
#define PROFILE_RANGE_BEGIN(category, rangeId, name, argbColor) rangeId = ProfileDuration::beginRange(trace_##category(), name, argbColor)
#define PROFILE_RANGE_END(category, rangeId) ProfileDuration::endRange(trace_##category(), rangeId)
#define PROFILE_SYNC_BEGIN(category, name, id, ...) syncBegin(trace_##category(), name, id, ##__VA_ARGS__);
#define PROFILE_SYNC_END(category, name, id, ...) syncEnd(trace_##category(), name, id, ##__VA_ARGS__);
#define PROFILE_ASYNC_BEGIN(category, name, id, ...) asyncBegin(trace_##category(), name, id, ##__VA_ARGS__);
#define PROFILE_ASYNC_END(category, name, id, ...) asyncEnd(trace_##category(), name, id, ##__VA_ARGS__);
#define PROFILE_COUNTER_IF_CHANGED(category, name, type, value) { static type lastValue = 0; type newValue = value;  if (newValue != lastValue) { counter(trace_##category(), name, { { name, newValue }}); lastValue = newValue; } }
#define PROFILE_COUNTER(category, name, ...) counter(trace_##category(), name, ##__VA_ARGS__);
#define PROFILE_INSTANT(category, name, ...) instant(trace_##category(), name, ##__VA_ARGS__);
#define PROFILE_SET_THREAD_NAME(threadName) metadata("thread_name", { { "name", threadName } });

#define SAMPLE_PROFILE_RANGE(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_RANGE(category, name); }
#define SAMPLE_PROFILE_RANGE_EX(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_RANGE_EX(category, name, argbColor, payload, ##__VA_ARGS__); }
#define SAMPLE_PROFILE_COUNTER(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_COUNTER(category, name, ##__VA_ARGS__); }
#define SAMPLE_PROFILE_INSTANT(chance, category, name, ...) if (randFloat() <= chance) { PROFILE_INSTANT(category, name, ##__VA_ARGS__); }

// uncomment WANT_DETAILED_PROFILING definition to enable profiling in high-frequency contexts
//#define WANT_DETAILED_PROFILING
#ifdef WANT_DETAILED_PROFILING
#define DETAILED_PROFILE_RANGE(category, name) ProfileDuration profileRangeThis(trace_##category(), name);
#define DETAILED_PROFILE_RANGE_EX(category, name, argbColor, payload, ...) ProfileDuration profileRangeThis(trace_##category(), name, argbColor, (uint64_t)payload, ##__VA_ARGS__);
#else // WANT_DETAILED_PROFILING
#define DETAILED_PROFILE_RANGE(category, name) ; // no-op
#define DETAILED_PROFILE_RANGE_EX(category, name, argbColor, payload, ...) ; // no-op
#endif // WANT_DETAILED_PROFILING

#endif
