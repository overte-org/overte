//
//  Created by Bradley Austin Davis 2015/10/23
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Controllers_CompositeEndpoint_h
#define hifi_Controllers_CompositeEndpoint_h

#include "../Endpoint.h"

namespace controller {
    class CompositeEndpoint : public Endpoint, Endpoint::Pair {
    public:
        static std::shared_ptr<Endpoint> newEndpoint(Endpoint::Pointer first, Endpoint::Pointer second) {
            return std::shared_ptr<Endpoint>(new CompositeEndpoint(first, second));
        };

        using Endpoint::apply;

        virtual AxisValue peek() const override;
        virtual AxisValue value() override;
        virtual void apply(AxisValue newValue, const Pointer& source) override;
        virtual bool readable() const override;

    private:
        CompositeEndpoint(Endpoint::Pointer first, Endpoint::Pointer second);
    };

}

#endif
