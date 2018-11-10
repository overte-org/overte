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

namespace gpu { namespace vk {

class VKQuery : public VKObject<Query> {
    using Parent = gpu::vk::VKObject<Query>;
public:
    template <typename VKQueryType>
    static VKQueryType* sync(VKBackend& backend, const Query& query) {
        VKQueryType* object = Backend::getGPUObject<VKQueryType>(query);

        // need to have a gpu object?
        if (!object) {
            // All is green, assign the gpuobject to the Query
            object = new VKQueryType(backend.shared_from_this(), query);
            (void)CHECK_VK_ERROR();
            Backend::setGPUObject(query, object);
        }

        return object;
    }

    template <typename VKQueryType>
    static VKuint getId(VKBackend& backend, const QueryPointer& query) {
        if (!query) {
            return 0;
        }

        VKQuery* object = sync<VKQueryType>(backend, *query);
        if (!object) {
            return 0;
        } 

        return object->_endqo;
    }

    const VKuint& _endqo = { _id };
    const VKuint _beginqo = { 0 };
    VKuint64 _result { (VKuint64)-1 };

protected:
    VKQuery(const std::weak_ptr<VKBackend>& backend, const Query& query, VKuint endId, VKuint beginId) : Parent(backend, query, endId), _beginqo(beginId) {}
    ~VKQuery() {
        if (_id) {
            VKuint ids[2] = { _endqo, _beginqo };
            glDeleteQueries(2, ids);
        }
    }
};

} }

#endif
