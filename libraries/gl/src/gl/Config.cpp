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

#include "Config.h"

#include <mutex>

#if defined(Q_OS_WIN)
#include <Windows.h>
#elif defined(Q_OS_ANDROID)
#elif defined(Q_OS_MAC)
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
#include <dlfcn.h>
#else
#include <GL/glx.h>
#include <dlfcn.h>
#endif



#if defined(Q_OS_WIN)

static void* getGlProcessAddress(const char *namez) {
    auto result = wglGetProcAddress(namez);
    if (!result) {
        static HMODULE glModule = nullptr;
        if (!glModule) {
            glModule = LoadLibraryW(L"opengl32.dll");
        }
        result = GetProcAddress(glModule, namez);
    }
    if (!result) {
        OutputDebugStringA(namez);
        OutputDebugStringA("\n");
    }
    return (void*)result;
}

typedef BOOL(APIENTRYP PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef int (APIENTRYP PFNWGLGETSWAPINTERVALEXTPROC) (void);
typedef BOOL(APIENTRYP PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC(APIENTRYP PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

#elif defined(Q_OS_ANDROID)

static void* getGlProcessAddress(const char *namez) {
    auto result = eglGetProcAddress(namez);
    return (void*)result;
}

#elif defined(Q_OS_MAC)

static void* getGlProcessAddress(const char *namez) {
    static void* GL_LIB = nullptr;
    if (nullptr == GL_LIB) {
        GL_LIB = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_NOW | RTLD_GLOBAL);
    }
    return dlsym(GL_LIB, namez);
}

#else


typedef Bool (*PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC) (int attribute, unsigned int *value);
typedef int (*PFNGLXSWAPINTERVALMESAPROC)(unsigned int interval);
typedef int (*PFNGLXGETSWAPINTERVALMESAPROC)(void);

PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC QueryCurrentRendererIntegerMESA;

// NOTE: On Linux, we can only really use MESA_swap_control.
// SGI_swap_control is useless (can't set swap interval to zero),
// and EXT_swap_control requires Xlib structures that Qt5 doesn't
// expose to us. From what I could tell, Nvidia's driver doesn't
// have this extension, so vsync will always be enabled there.
// Because Qt5 doesn't expose Xlib structures, we can't properly
// check if the GLX extension for these is supported and we have
// to hope the driver returns NULL for them.
// (QueryCurrentRenderIntegerMESA was already doing this)
PFNGLXSWAPINTERVALMESAPROC SwapIntervalMESA;
PFNGLXGETSWAPINTERVALMESAPROC GetSwapIntervalMESA;

static void* getGlProcessAddress(const char *namez) {
    return (void*)glXGetProcAddressARB((const GLubyte*)namez);
}

#endif



void gl::initModuleGl() {
    static std::once_flag once;
    std::call_once(once, [] {
#if defined(Q_OS_WIN)
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)getGlProcessAddress("wglSwapIntervalEXT");
        wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)getGlProcessAddress("wglGetSwapIntervalEXT");
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)getGlProcessAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)getGlProcessAddress("wglCreateContextAttribsARB");
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
        QueryCurrentRendererIntegerMESA = (PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC)getGlProcessAddress("glXQueryCurrentRendererIntegerMESA");
        SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)getGlProcessAddress("glXSwapIntervalMESA");
        GetSwapIntervalMESA = (PFNGLXGETSWAPINTERVALMESAPROC)getGlProcessAddress("glXGetSwapIntervalMESA");
#endif

#if defined(USE_GLES)
        gladLoadGLES2Loader(getGlProcessAddress);
#else
        gladLoadGLLoader(getGlProcessAddress);
#endif
    });
}

int gl::getSwapInterval() {
#if defined(Q_OS_WIN)
    return wglGetSwapIntervalEXT();
#elif defined(Q_OS_MAC)
    GLint interval;
    CGLGetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);
    return interval;
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (GetSwapIntervalMESA) {
        return GetSwapIntervalMESA();
    } else {
        return 1;
    }
#else
    return 1;
#endif
}

void gl::setSwapInterval(int interval) {
#if defined(Q_OS_WIN)
    wglSwapIntervalEXT(interval);
#elif defined(Q_OS_MAC)
    CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);
#elif defined(Q_OS_ANDROID)
    eglSwapInterval(eglGetCurrentDisplay(), interval);
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (SwapIntervalMESA) {
        SwapIntervalMESA(interval);
    }
#else
    Q_UNUSED(interval);
#endif
}

bool gl::queryCurrentRendererIntegerMESA(int attr, unsigned int *value) {
    #if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (QueryCurrentRendererIntegerMESA) {
        return QueryCurrentRendererIntegerMESA(attr, value);
    }
    #endif

    *value = 0;
    return false;
}
