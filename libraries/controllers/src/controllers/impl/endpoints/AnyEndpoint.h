//
//  Created by Bradley Austin Davis 2015/10/23
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Controllers_AnyEndpoint_h
#define hifi_Controllers_AnyEndpoint_h

#include "../Endpoint.h"

namespace controller {

class AnyEndpoint : public Endpoint {
    friend class UserInputMapper;
public:
    static std::shared_ptr<Endpoint> newEndpoint(Endpoint::List children) {
        return std::shared_ptr<Endpoint>(new AnyEndpoint(children));
    };

    using Endpoint::apply;
    virtual AxisValue peek() const override;
    virtual AxisValue value() override;
    virtual void apply(AxisValue newValue, const Endpoint::Pointer& source) override;
    virtual bool writeable() const override;
    virtual bool readable() const override;

private:
    AnyEndpoint(Endpoint::List children);

    Endpoint::List _children;
};

}

#endif
