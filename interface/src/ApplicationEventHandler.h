//
//  ApplicationEventHandler.h
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_ApplicationEventHandler_h
#define hifi_ApplicationEventHandler_h

#include <QtCore/QAbstractNativeEventFilter>
#include <qsystemdetection.h>

#include <MainWindow.h>

#include "Application.h"

#ifdef Q_OS_WIN
static const UINT UWM_IDENTIFY_INSTANCES =
    RegisterWindowMessage("UWM_IDENTIFY_INSTANCES_{8AB82783-B74A-4258-955B-8188C22AA0D6}_" + qgetenv("USERNAME"));
static const UINT UWM_SHOW_APPLICATION =
    RegisterWindowMessage("UWM_SHOW_APPLICATION_{71123FD6-3DA8-4DC1-9C27-8A12A6250CBA}_" + qgetenv("USERNAME"));

class MyNativeEventFilter : public QAbstractNativeEventFilter {
public:
    static MyNativeEventFilter& getInstance() {
        static MyNativeEventFilter staticInstance;
        return staticInstance;
    }

    bool nativeEventFilter(const QByteArray &eventType, void* msg, long* result) Q_DECL_OVERRIDE {
        if (eventType == "windows_generic_MSG") {
            MSG* message = (MSG*)msg;

            if (message->message == UWM_IDENTIFY_INSTANCES) {
                *result = UWM_IDENTIFY_INSTANCES;
                return true;
            }

            if (message->message == UWM_SHOW_APPLICATION) {
                MainWindow* applicationWindow = qApp->getWindow();
                if (applicationWindow->isMinimized()) {
                    applicationWindow->showNormal();  // Restores to windowed or maximized state appropriately.
                }
                qApp->setActiveWindow(applicationWindow);  // Flashes the taskbar icon if not focus.
                return true;
            }

            // Attempting to close MIDI interfaces of a hot-unplugged device can result in audio-driver deadlock.
            // Detecting MIDI devices that have been added/removed after starting Inteface has been disabled.
            // https://support.microsoft.com/en-us/help/4460006/midi-device-app-hangs-when-former-midi-api-is-used
#if 0
            if (message->message == WM_DEVICECHANGE) {
                const float MIN_DELTA_SECONDS = 2.0f; // de-bounce signal
                static float lastTriggerTime = 0.0f;
                const float deltaSeconds = secTimestampNow() - lastTriggerTime;
                lastTriggerTime = secTimestampNow();
                if (deltaSeconds > MIN_DELTA_SECONDS) {
                    Midi::USBchanged();                // re-scan the MIDI bus
                }
            }
#endif
        }
        return false;
    }
};
#endif

#endif  // hifi_ApplicationEventHandler_h
