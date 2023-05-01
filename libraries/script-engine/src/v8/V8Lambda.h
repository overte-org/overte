//
//  V8Lambda.h
//  split from ScriptEngineV8.h
//  libraries/script-engine/src/qtscript
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_V8Lambda_h
#define hifi_V8Lambda_h

#include "ScriptEngineV8.h"
#include "V8Types.h"

// Lambda helps create callable V8ScriptValues out of std::functions:
// (just meant for use from within the script engine itself)
// V8TODO: this looks like it can be safely removed?
class Lambda : public QObject {
    Q_OBJECT
public:
    Lambda(ScriptEngineV8* engine,
           std::function<V8ScriptValue(ScriptEngineV8* engine)> operation,
           V8ScriptValue data);
    ~Lambda();
public slots:
    V8ScriptValue call();
    QString toString() const;

private:
    ScriptEngineV8* _engine;
    std::function<V8ScriptValue(ScriptEngineV8* engine)> _operation;
    V8ScriptValue _data;
};

#endif
