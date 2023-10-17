//
//  OverlayConductor.h
//  interface/src/ui
//
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_OverlayConductor_h
#define hifi_OverlayConductor_h

#include <cstdint>

class OverlayConductor {
public:
    OverlayConductor();
    ~OverlayConductor();

    void update(float dt);
    void centerUI();

private:
    bool headNotCenteredInOverlay() const;

#if !defined(DISABLE_QML)
    bool _hmdMode { false };
#endif

    // This stores value of myAvatar->hasDriveInput() from previous update, so that recentering can be triggered by a rising edge of that function's output
    bool _lastHasDriveInput { false };
};

#endif
