//
//  ScriptGatekeeper.h
//  libraries/script-engine/src
//
//  Created by Kalila L. on Mar 7 2021
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef overte_ScriptGatekeeper_h
#define overte_ScriptGatekeeper_h

#include <QtCore/QObject>

/// Manages script allowlisting in a domain
class ScriptGatekeeper : public QObject {
    Q_OBJECT
public:
    void initialize();

    QString SCRIPT_ALLOWLIST_ENABLED_KEY{ "private/allowlistEnabled" };
    QString SCRIPT_ALLOWLIST_ENTRIES_KEY{ "private/settingsSafeURLS" };

private:
    bool _initialized { false };
};

#endif // overte_ScriptGatekeeper_h

/// @}
