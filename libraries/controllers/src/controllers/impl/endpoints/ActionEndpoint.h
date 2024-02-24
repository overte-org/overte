//
//  Created by Bradley Austin Davis 2015/10/23
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Controllers_ActionEndpoint_h
#define hifi_Controllers_ActionEndpoint_h

#include "../Endpoint.h"

#include "../../Actions.h"
#include <DependencyManager.h>

#include "../../UserInputMapper.h"

namespace controller {

class ActionEndpoint : public Endpoint {
public:
    static std::shared_ptr<Endpoint> newEndpoint(const Input& id = Input::INVALID_INPUT) {
        return std::shared_ptr<Endpoint>(new ActionEndpoint(id));
    };

    virtual AxisValue peek() const override { return _currentValue; }
    virtual void apply(AxisValue newValue, const Pointer& source) override;

    virtual Pose peekPose() const override { return _currentPose; }
    virtual void apply(const Pose& value, const Pointer& source) override;

    virtual void reset() override;

private:
    ActionEndpoint(const Input& id = Input::INVALID_INPUT) : Endpoint(id) { }

    AxisValue _currentValue { 0.0f, 0, false };
    Pose _currentPose{};
};

}

#endif
