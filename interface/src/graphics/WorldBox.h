//
//  WorldBox.h
//
//  Created by Sam Gateau on 01/07/2018.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_WorldBox_h
#define hifi_WorldBox_h

#include <PerfStat.h>

#include <gpu/Batch.h>
#include <render/Forward.h>

#include <render/Item.h>
#include <GeometryCache.h>
#include "Menu.h"

/**
 * Class used to render a box representing the octree bounds (inside which all entities need to be contained).
 */
class WorldBoxRenderData {
public:
    typedef render::Payload<WorldBoxRenderData> Payload;
    typedef Payload::DataPointer Pointer;

    /**
     * DOCTODO: doesn't seem to be used?
     */
    //int _val = 0;

    /**
     * Unique id.
     */
    static render::ItemID _item; // unique WorldBoxRenderData

    /**
     * Renders octree bounds.
     * @param args Render settings.
     * @param batch Batch to which rendering commands will be added.
     */
    static void renderWorldBox(RenderArgs* args, gpu::Batch& batch);
};

namespace render {
    /**
     * DOCTODO how do payloads work?
     * @param stuff Not used.
     * @return
     */
    template <> const ItemKey payloadGetKey(const WorldBoxRenderData::Pointer& stuff);

    /**
     * Returns a zero-initialized bounding box. It seems broken?
     *
     * @param stuff Not used.
     * @param args Not used.
     * @return A zero-initialized bounding box.
     */
    template <> const Item::Bound payloadGetBound(const WorldBoxRenderData::Pointer& stuff, RenderArgs* args);

    /**
     * Adds commands to a batch to render world box.
     * @param stuff Not used.
     * @param args Renderer and frame configuration.
     */
template <> void payloadRender(const WorldBoxRenderData::Pointer& stuff, RenderArgs* args);
}

#endif // hifi_WorldBox_h