//
//  GPUConfig.h
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 12/4/14.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_gpu_GPUConfig_h
#define hifi_gpu_GPUConfig_h

#include <QtCore/QtGlobal>

#include <glad/glad.h>

#if defined(Q_OS_ANDROID)
#include <EGL/egl.h>
#else

#ifndef GL_SLUMINANCE8_EXT
#define GL_SLUMINANCE8_EXT 0x8C47
#endif

// Prevent inclusion of System GL headers
#define __glext_h_
#define __gl_h_
#define __gl3_h_

#endif

// Platform specific code to load the GL functions
namespace gl {
    void initModuleGl();
    int getSwapInterval();
    void setSwapInterval(int swapInterval);
    bool queryCurrentRendererIntegerMESA(int attr, unsigned int *value);
}

#endif // hifi_gpu_GPUConfig_h
