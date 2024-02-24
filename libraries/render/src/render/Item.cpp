//
//  Item.cpp
//  render/src/render
//
//  Created by Sam Gateau on 1/26/16.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "Item.h"

#include <numeric>
#include "gpu/Batch.h"

#include "TransitionStage.h"

using namespace render;

const Item::ID Item::INVALID_ITEM_ID = 0;
const ItemCell Item::INVALID_CELL = -1;

const Item::Status::Value Item::Status::Value::INVALID = Item::Status::Value();

const float Item::Status::Value::RED = 0.0f;
const float Item::Status::Value::YELLOW = 60.0f;
const float Item::Status::Value::GREEN = 120.0f;
const float Item::Status::Value::CYAN = 180.0f;
const float Item::Status::Value::BLUE = 240.0f;
const float Item::Status::Value::MAGENTA = 300.0f;

const uint32_t ItemKey::KEY_TAG_BITS_MASK = ((uint32_t) ItemKey::TAG_BITS_ALL) << FIRST_TAG_BIT;

const uint32_t ItemKey::KEY_LAYER_BITS_MASK = ((uint32_t)ItemKey::LAYER_BITS_ALL) << FIRST_LAYER_BIT;

void Item::Status::Value::setScale(float scale) {
    _scale = (std::numeric_limits<unsigned short>::max() -1) * 0.5f * (1.0f + std::max(std::min(scale, 1.0f), 0.0f));
 }

void Item::Status::Value::setColor(float hue) {
    // Convert the HUe from range [0, 360] to signed normalized value
    const float HUE_MAX = 360.0f;
    _color = (std::numeric_limits<unsigned char>::max()) * (std::max(std::min(hue, HUE_MAX), 0.0f) / HUE_MAX);
}
void Item::Status::Value::setIcon(unsigned char icon) {
    _icon = icon;
}

Item::Status::Values Item::Status::getCurrentValues() const {
    Values currentValues(_values.size());
    auto currentValue = currentValues.begin();
    for (auto& getter : _values) {
        (*currentValue) = getter();
        currentValue++;
    }
    return currentValues;
}

void Item::PayloadInterface::addStatusGetter(const Status::Getter& getter) {
    if (!_status) {
        _status = std::make_shared<Status>();
    }
    _status->addGetter(getter);
}

void Item::PayloadInterface::addStatusGetters(const Status::Getters& getters) {
    if (!_status) {
        _status = std::make_shared<Status>();
    }
    for (auto& g : getters) {
        _status->addGetter(g);
    }
}

void Item::update(const UpdateFunctorPointer& updateFunctor) {
    if (updateFunctor) {
        _payload->update(updateFunctor);
    }
    _key = _payload->getKey();
}

void Item::resetPayload(const PayloadPointer& payload) {
    if (!payload) {
        kill();
    } else {
        _payload = payload;
        _key = _payload->getKey();
    }
}

const ShapeKey Item::getShapeKey() const {
    auto shapeKey = _payload->getShapeKey();
    if (!TransitionStage::isIndexInvalid(_transitionId)) {
        // Objects that are fading are rendered double-sided to give a sense of volume
        return ShapeKey::Builder(shapeKey).withFade().withoutCullFace();
    }
    return shapeKey;
}

uint32_t Item::fetchMetaSubItemBounds(ItemBounds& subItemBounds, Scene& scene, RenderArgs* args) const {
    ItemIDs subItems;
    auto numSubs = fetchMetaSubItems(subItems);

    for (auto id : subItems) {
        // TODO: Adding an extra check here even thought we shouldn't have too.
        // We have cases when the id returned by fetchMetaSubItems is not allocated
        if (scene.isAllocatedID(id)) {
            auto& item = scene.getItem(id);
            if (item.exist()) {
                subItemBounds.emplace_back(id, item.getBound(args));
            } else {
                numSubs--;
            }
        } else {
            numSubs--;
        }
    }
    return numSubs;
}

namespace render {
    template <> const ItemKey payloadGetKey(const PayloadProxyInterface::Pointer& payload) {
        if (!payload) {
            return ItemKey::Builder::opaqueShape().withTypeMeta();
        }
        return payload->getKey();
    }

    template <> const ShapeKey shapeGetShapeKey(const PayloadProxyInterface::Pointer& payload) {
        if (!payload) {
            return ShapeKey::Builder::ownPipeline();
        }
        return payload->getShapeKey();
    }

    template <> const Item::Bound payloadGetBound(const PayloadProxyInterface::Pointer& payload, RenderArgs* args) {
        if (!payload) {
            return render::Item::Bound();
        }
        return payload->getBound(args);
    }

    template <> void payloadRender(const PayloadProxyInterface::Pointer& payload, RenderArgs* args) {
        if (!args || !payload) {
            return;
        }
        payload->render(args);
    }

    template <> uint32_t metaFetchMetaSubItems(const PayloadProxyInterface::Pointer& payload, ItemIDs& subItems) {
        if (!payload) {
            return 0;
        }
        return payload->metaFetchMetaSubItems(subItems);
    }

    template <> bool payloadPassesZoneOcclusionTest(const PayloadProxyInterface::Pointer& payload, const std::unordered_set<QUuid>& containingZones) {
        if (!payload) {
            return false;
        }
        return payload->passesZoneOcclusionTest(containingZones);
    }

    template <> HighlightStyle payloadGetOutlineStyle(const PayloadProxyInterface::Pointer& payload, const ViewFrustum& viewFrustum, const size_t height) {
        if (!payload) {
            return HighlightStyle();
        }
        return payload->getOutlineStyle(viewFrustum, height);
    }
}