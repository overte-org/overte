//
//  Created by Bradley Austin Davis on 2017/06/06
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_TBBHelpers_h
#define hifi_TBBHelpers_h

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4334 )
#endif

#if !defined(Q_MOC_RUN)
// Work around https://bugreports.qt.io/browse/QTBUG-80990

// This causes a compile error in profiling.h:
// profiling.h:229:15: error: expected unqualified-id before ‘)’ token
//   229 |     void emit() { }
//
// 'emit' is defined to nothing in qt and is just syntactic sugar, so get rid of it
#undef emit

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>


// and re-add later.
#define emit

#endif

#ifdef _WIN32
#pragma warning( pop )
#endif

#endif // hifi_TBBHelpers_h
