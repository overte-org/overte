//
//  ScriptEngine.h
//  libraries/script-engine/src
//
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0

#pragma once

struct NonCopyable {
protected:
    NonCopyable() {}
    ~NonCopyable() {}

public:
    NonCopyable(const NonCopyable&) = delete;
    const NonCopyable& operator=(const NonCopyable&) = delete;
};

struct NonMovable {
protected:
    NonMovable() {}
    ~NonMovable() {}

public:
    NonMovable(NonMovable&&) = delete;
    const NonMovable& operator=(NonMovable&&) = delete;
};
