//
//  GLBackend.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 10/27/2014.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include <gpu/gl/GLBackend.h>

#include <functional>

#include <QtCore/QProcessEnvironment>

#include <shared/GlobalAppProperties.h>
#include <gl/QOpenGLContextWrapper.h>
#include <gl/GLHelpers.h>
#include <gpu/gl/GLShader.h>

#include "../gl41/GL41Backend.h"
#include "../gl45/GL45Backend.h"
#if defined(OVERTE_USE_GLES)
#include "../gles/GLESBackend.h"
#endif

using namespace gpu;
using namespace gpu::gl;

static GLBackend* INSTANCE{ nullptr };

BackendPointer GLBackend::createBackend() {
    auto backendApi = hifi::properties::getGraphicsAPI();
    auto version = QOpenGLContextWrapper::currentContextVersion();
    std::shared_ptr<GLBackend> result;
    if (backendApi == hifi::properties::GraphicsAPI::GL45 && version >= 0x0405) {
        qCDebug(gpugllogging) << "Using OpenGL 4.5 backend";
        result = std::make_shared<gpu::gl45::GL45Backend>();
    } else if (backendApi == hifi::properties::GraphicsAPI::GL41) {
        qCDebug(gpugllogging) << "Using OpenGL 4.1 backend";
        result = std::make_shared<gpu::gl41::GL41Backend>();
    } else
#if defined(OVERTE_USE_GLES)
    if (backendApi == hifi::properties::GraphicsAPI::GLES32) {
        qDebug() << "Using OpenGLES 3.2 backend";
        result = std::make_shared<gpu::gles::GLESBackend>();
    } else
#endif
    {
        qDebug() << "Unknown OpenGL backend" << (int)backendApi;
        return nullptr;
    }

    result->initInput();
    result->initTransform();
    result->initTextureManagementStage();

    INSTANCE = result.get();
    void* voidInstance = &(*result);
    qApp->setProperty(hifi::properties::gl::BACKEND, QVariant::fromValue(voidInstance));
    return result;
}

GLBackend& getBackend() {
    if (!INSTANCE) {
        INSTANCE = static_cast<GLBackend*>(qApp->property(hifi::properties::gl::BACKEND).value<void*>());
    }
    return *INSTANCE;
}
