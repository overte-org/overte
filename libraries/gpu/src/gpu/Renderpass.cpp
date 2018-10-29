//
//  Created by Bradley Austin Davis on 2018/10/21
//  Copyright 2013-2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Renderpass.h"

using namespace gpu;

Attachment::Attachment() {
}

Attachment::Attachment(const Element& element, LoadOp load, StoreOp store, LoadOp stencilLoad, StoreOp stencilStore) :
    loadOp(load), storeOp(store), stencilLoadOp(stencilLoad), stencilStoreOp(stencilStore) {
}

uint32_t Attachment::getRaw() const {
    throw std::runtime_error("not implemented");
}

void Attachment::setRaw(uint32_t raw) {
    throw std::runtime_error("not implemented");
}

void Renderpass::addColorAttachment(const Element& element, LoadOp load, StoreOp store) {
    _colorAttachments.emplace_back(element, load, store);
}

void Renderpass::setDepthStencilAttachment(const Element& element,
                                           LoadOp load,
                                           StoreOp store,
                                           LoadOp stencilLoad,
                                           StoreOp stencilStore) {
    _depthStencilAttachment = Attachment{ element, load, store, stencilLoad, stencilStore };
}
