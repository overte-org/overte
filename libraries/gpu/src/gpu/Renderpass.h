//
//  Created by Bradley Austin Davis on 2018/10/21
//  Copyright 2013-2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once
#ifndef hifi_gpu_Renderpass_h
#define hifi_gpu_Renderpass_h

#include <assert.h>
#include <memory>
#include <functional>
#include <vector>
#include <string>

#include "Format.h"

namespace gpu {

enum class LoadOp
{
    Load = 0,
    Clear = 1,
    DontCare = 2,
};

enum class StoreOp
{
    Store = 0,
    DontCare = 1,
};

struct Attachment {
    Attachment();
    Attachment(const Element& element, LoadOp loadOp, StoreOp storeOp, LoadOp stencilLoadOp = LoadOp::DontCare, StoreOp stencilStoreOp = StoreOp::DontCare);
    Element element;
    LoadOp loadOp{ LoadOp::DontCare };
    StoreOp storeOp{ StoreOp::DontCare };
    LoadOp stencilLoadOp{ LoadOp::DontCare };
    StoreOp stencilStoreOp{ StoreOp::DontCare };

    uint32_t getRaw() const;
    void setRaw(uint32_t raw);
};

class Renderpass {
public:
    using Attachments = std::vector<Attachment>;

    Renderpass();
    virtual ~Renderpass();

    void addColorAttachment(const Element& element, LoadOp load, StoreOp store);
    void setDepthStencilAttachment(const Element& element, LoadOp load, StoreOp store, LoadOp stencilLoadOp = LoadOp::DontCare, StoreOp stencilStoreOp = StoreOp::DontCare);

protected:
    Attachments _colorAttachments;
    Attachment _depthStencilAttachment;
    // TODO if we want to start working with input attachments and multisampling resolve attachments, we'll need to extend this class 
};

}  // namespace gpu

#endif
