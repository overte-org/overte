//
//  CrashRecoveryHandler.h
//  interface/src
//
//  Created by David Rowe on 24 Aug 2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_CrashRecoveryHandler_h
#define hifi_CrashRecoveryHandler_h

#include <QString>






class CrashRecoveryHandler {

public:
    static bool checkForResetSettings(bool wasLikelyCrash, bool suppressPrompt = false);

private:
    enum Action {
        DELETE_INTERFACE_INI,
        RETAIN_IMPORTANT_INFO,
        DO_NOTHING
    };

    static bool suggestCrashReporting();
    static Action promptUserForAction(bool showCrashMessage);
    static void handleCrash(Action action);

};

#endif // hifi_CrashRecoveryHandler_h
