//
//  Breakpoint.h
//
//
//  Created by Dale Glass on 5/6/2022
//  Copyright 2022 Dale Glass
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


// Software defined breakpoints, for aiding in debugging

#pragma once

#if defined(__GNUC__)
    #include <csignal>
    #define BREAKPOINT raise(SIGINT);
#elif defined(__clang__)
    #define BREAKPOINT __builtin_trap();
#elif _MSC_VER && !__INTEL_COMPILER
    #include <intrin.h>
    #define BREAKPOINT __debugbreak();
#else
    #include "CrashHelpers.h"
    #define BREAKPOINT crash::nullDeref();
#endif
