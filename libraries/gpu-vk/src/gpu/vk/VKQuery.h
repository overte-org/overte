//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_vk_VKQuery_h
#define hifi_gpu_vk_VKQuery_h

#include "VKShared.h"
#include "VKBackend.h"

namespace gpu { namespace vulkan {

class VKQuery : public VKObject<Query> {
    using Parent = gpu::vulkan::VKObject<Query>;
public:
    static VKQuery* sync(VKBackend& backend, const Query& query) {
        VKQuery* object = Backend::getGPUObject<VKQuery>(query);

        // need to have a gpu object?
        if (!object) {
            // All is green, assign the gpuobject to the Query
            object = new VKQuery(backend.shared_from_this(), query);
            Backend::setGPUObject(query, object);
        }

        return object;
    }

    template <typename VKQueryType>
    static uint32_t getId(VKBackend& backend, const QueryPointer& query) {
        // VKTODO Vulkan handle is used instead
        return 0;
        if (!query) {
            return 0;
        }

        /*VKQuery* object = sync<VKQueryType>(backend, *query);
        if (!object) {
            return 0;
        } 

        return object->_endqo;*/
    }

    // VKTODO Vulkan handle is used instead
    /*const uint32_t& _endqo = { _id };
    const uint32_t _beginqo = { 0 };
    uint64_t _result { (uint64_t)-1 };*/

protected:
    // VKTODO: We need a check on backend.lock(), or to pass backend reference instead
    VKQuery(const std::weak_ptr<VKBackend>& backend, const Query& query) : Parent(*backend.lock(), query) {}
    ~VKQuery() {
        // Vulkan handle is used instead
        /*if (_id) {
            // VKTODO
            uint32_t ids[2] = { _endqo, _beginqo };
            //glDeleteQueries(2, ids);
        }*/
    }
};

} }

#endif
