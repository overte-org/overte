//
//  Created by Bradley Austin Davis 2015/10/23
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Controllers_JSEndpoint_h
#define hifi_Controllers_JSEndpoint_h

#include "../Endpoint.h"

#include <QtQml/QJSValue>
#include <QtQml/QJSValueList>

namespace controller {

class JSEndpoint : public Endpoint {
public:
    static std::shared_ptr<Endpoint> newEndpoint(const QJSValue& callable) {
        return std::shared_ptr<Endpoint>(new JSEndpoint(callable));
    };

    using Endpoint::apply;
    virtual AxisValue peek() const override;
    virtual void apply(AxisValue newValue, const Pointer& source) override;

private:
    JSEndpoint(const QJSValue& callable)
        : Endpoint(Input::INVALID_INPUT), _callable(callable) {
    }

    mutable QJSValue _callable;
};

}

#endif
