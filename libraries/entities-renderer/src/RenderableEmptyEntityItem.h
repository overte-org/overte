//
//  Created by Ada <ada@thingvellir.net> on 2025-07-24
//  Copyright Overte e.V. 2025
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_RenderableEmptyEntityItem_h
#define hifi_RenderableEmptyEntityItem_h

#include "RenderableEntityItem.h"
#include <EmptyEntityItem.h>

namespace render { namespace entities {

class EmptyEntityRenderer : public TypedEntityRenderer<EmptyEntityItem> {
    using Parent = TypedEntityRenderer<EmptyEntityItem>;
    using Pointer = std::shared_ptr<EmptyEntityItem>;
public:
    EmptyEntityRenderer(const EntityItemPointer& entity) : Parent(entity) { }

private:
    void doRender(RenderArgs* args) override { }
};

} }

#endif // hifi_RenderableEmptyEntityItem_h
