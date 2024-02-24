//
//  OvenCLIApplication.h
//  tools/oven/src
//
//  Created by Stephen Birarda on 2/20/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_OvenCLIApplication_h
#define hifi_OvenCLIApplication_h

#include <QtCore/QCoreApplication>

#include "Oven.h"

class OvenCLIApplication : public QCoreApplication, public Oven {
    Q_OBJECT
public:
    OvenCLIApplication(int argc, char* argv[]);

    enum parseResult {
        GUIMode,
        CLIMode
    };

    /**
     * @brief Parses the command line arguments
     *
     * Oven can operate both in GUI and CLI mode. Depending on which arguments are passed,
     * we pick the mode to use, and this function returns it.
     *
     * Both modes can have the crash handler enabled.
     *
     * @param argc argc
     * @param argv argv
     * @param enableCrashHandler Output parameter -- whether the crash handler should be enabled.
     * @return parseResult Which application we should run
     */
    static parseResult parseCommandLine(int argc, char* argv[], bool &enableCrashHandler);

    static OvenCLIApplication* instance() { return dynamic_cast<OvenCLIApplication*>(QCoreApplication::instance()); }

private:
    static QUrl _inputUrlParameter;
    static QUrl _outputUrlParameter;
    static QString _typeParameter;
};

#endif // hifi_OvenCLIApplication_h
