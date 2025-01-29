//
//  Created by Bradley Austin Davis on 2016/08/21
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Context.h"

#include <assert.h>

#include <mutex>

#include <QtCore/QDebug>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QThread>

#include <QtGui/QWindow>
#include <QtGui/QGuiApplication>

#include <shared/AbstractLoggerInterface.h>
#include <shared/GlobalAppProperties.h>
#include <GLMHelpers.h>
#include "GLLogging.h"
#include "Config.h"
#include "GLHelpers.h"
#include "QOpenGLContextWrapper.h"

#if defined(GL_CUSTOM_CONTEXT)
#include <QtPlatformHeaders/QWGLNativeContext>
#endif

using namespace gl;

#if defined(GL_CUSTOM_CONTEXT)
bool Context::USE_CUSTOM_CONTEXT { true };
#endif

bool Context::enableDebugLogger() {
    return gl::debugContextEnabled();
}

std::atomic<size_t> Context::_totalSwapchainMemoryUsage { 0 };

size_t Context::getSwapchainMemoryUsage() { return _totalSwapchainMemoryUsage.load(); }

size_t Context::evalSurfaceMemoryUsage(uint32_t width, uint32_t height, uint32_t pixelSize) {
    size_t result = width;
    result *= height;
    result *= pixelSize;
    return result;
}

void Context::updateSwapchainMemoryUsage(size_t prevSize, size_t newSize) {
    if (prevSize == newSize) {
        return;
    }
    if (newSize > prevSize) {
        _totalSwapchainMemoryUsage.fetch_add(newSize - prevSize);
    } else {
        _totalSwapchainMemoryUsage.fetch_sub(prevSize - newSize);
    }
}


Context::Context() {}

Context::Context(QWindow* window) {
    setWindow(window);
}

void Context::release() {
    doneCurrent();
    if (_qglContext) {
        destroyContext(_qglContext);
        _qglContext = nullptr;
    }

#ifdef GL_CUSTOM_CONTEXT
    if (_hglrc) {
        wglDeleteContext(_hglrc);
        _hglrc = 0;
    }

    if (_hdc) {
        ReleaseDC(_hwnd, _hdc);
        _hdc = 0;
    }
    _hwnd = 0;
#endif

    _window = nullptr;
    updateSwapchainMemoryCounter();
}

Context::~Context() {
    release();
}

void Context::updateSwapchainMemoryCounter() {
    if (_window) {
        auto newSize = _window->size();
        auto newMemSize = gl::Context::evalSurfaceMemoryUsage(newSize.width(), newSize.height(), (uint32_t) _swapchainPixelSize);
        gl::Context::updateSwapchainMemoryUsage(_swapchainMemoryUsage, newMemSize);
        _swapchainMemoryUsage = newMemSize;
    } else {
        // No window ? no more swapchain
        gl::Context::updateSwapchainMemoryUsage(_swapchainMemoryUsage, 0);
        _swapchainMemoryUsage = 0;
    }
}

void Context::setWindow(QWindow* window) {
    release();
    _window = window;

#ifdef GL_CUSTOM_CONTEXT
    _hwnd = (HWND)window->winId();
#endif

    updateSwapchainMemoryCounter();
}

void Context::clear() {
    glClearColor(0, 0, 0, 1);
    QSize windowSize = _window->size() * _window->devicePixelRatio();
    glViewport(0, 0, windowSize.width(), windowSize.height());
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// VKTODO
    swapBuffers();
}

#if defined(GL_CUSTOM_CONTEXT)

static void setupPixelFormatSimple(HDC hdc) {
    // FIXME build the PFD based on the
    static const PIXELFORMATDESCRIPTOR pfd =    // pfd Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),         // Size Of This Pixel Format Descriptor
        1,                                      // Version Number
        PFD_DRAW_TO_WINDOW |                    // Format Must Support Window
        PFD_SUPPORT_OPENGL |                    // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                       // Must Support Double Buffering
        PFD_TYPE_RGBA,                          // Request An RGBA Format
        24,                                     // Select Our Color Depth
        0, 0, 0, 0, 0, 0,                       // Color Bits Ignored
        1,                                      // Alpha Buffer
        0,                                      // Shift Bit Ignored
        0,                                      // No Accumulation Buffer
        0, 0, 0, 0,                             // Accumulation Bits Ignored
        24,                                     // 24 Bit Z-Buffer (Depth Buffer)  
        8,                                      // 8 Bit Stencil Buffer
        0,                                      // No Auxiliary Buffer
        PFD_MAIN_PLANE,                         // Main Drawing Layer
        0,                                      // Reserved
        0, 0, 0                                 // Layer Masks Ignored
    };
    auto pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0) {
        throw std::runtime_error("Unable to create initial context");
    }

    if (SetPixelFormat(hdc, pixelFormat, &pfd) == FALSE) {
        throw std::runtime_error("Unable to create initial context");
    }
}

// Pixel format arguments
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011

// Context create arguments
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

// Context create flag bits
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004

#if !defined(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB)
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB 0x8242
#endif

typedef BOOL(APIENTRYP PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC(APIENTRYP PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);

GLAPI PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
GLAPI PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

#endif


Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();

#if defined(GL_CUSTOM_CONTEXT)
bool Context::makeCurrent() {
    BOOL result = wglMakeCurrent(_hdc, _hglrc);
    assert(result);
    updateSwapchainMemoryCounter();
    return result;
}
void Context::swapBuffers() {
    SwapBuffers(_hdc);
}
void Context::doneCurrent() {
    wglMakeCurrent(0, 0);
}
#endif

void Context::create(QOpenGLContext* shareContext) {
    if (!shareContext) {
        shareContext = qt_gl_global_share_context();
    }

#if defined(GL_CUSTOM_CONTEXT)
    assert(0 != _hwnd);
    assert(0 == _hdc);
    auto hwnd = _hwnd;
    // Create a temporary context to initialize glew
    static std::once_flag once;
    std::call_once(once, [&] {
        auto hdc = GetDC(hwnd);
        setupPixelFormatSimple(hdc);
        auto glrc = wglCreateContext(hdc);
        BOOL makeCurrentResult;
        makeCurrentResult = wglMakeCurrent(hdc, glrc);
        if (!makeCurrentResult) {
            throw std::runtime_error("Unable to create initial context");
        }
        gl::initModuleGl();
        wglMakeCurrent(0, 0);
        wglDeleteContext(glrc);
        ReleaseDC(hwnd, hdc);
        if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
            USE_CUSTOM_CONTEXT = false;
        }
    });

    if (USE_CUSTOM_CONTEXT) {
        _hdc = GetDC(_hwnd);
#if defined(USE_GLES)
        _version = 0x0302;
#else
        if (gl::disableGl45()) {
            _version = 0x0401;
        } else if (GLAD_GL_VERSION_4_5) {
            _version = 0x0405;
        } else if (GLAD_GL_VERSION_4_3) {
            _version = 0x0403;
        } else {
            _version = 0x0401;
        }
#endif

        static int pixelFormat = 0;
        static PIXELFORMATDESCRIPTOR pfd;
        if (!pixelFormat) {
            memset(&pfd, 0, sizeof(pfd));
            pfd.nSize = sizeof(pfd);
            std::vector<int> formatAttribs;
            formatAttribs.push_back(WGL_DRAW_TO_WINDOW_ARB);
            formatAttribs.push_back(GL_TRUE);
            formatAttribs.push_back(WGL_SUPPORT_OPENGL_ARB);
            formatAttribs.push_back(GL_TRUE);
            formatAttribs.push_back(WGL_DOUBLE_BUFFER_ARB);
            formatAttribs.push_back(GL_TRUE);
            formatAttribs.push_back(WGL_PIXEL_TYPE_ARB);
            formatAttribs.push_back(WGL_TYPE_RGBA_ARB);
            formatAttribs.push_back(WGL_COLOR_BITS_ARB);
            formatAttribs.push_back(32);
            formatAttribs.push_back(WGL_DEPTH_BITS_ARB);
            formatAttribs.push_back(24);
            formatAttribs.push_back(WGL_STENCIL_BITS_ARB);
            formatAttribs.push_back(8);

#ifdef NATIVE_SRGB_FRAMEBUFFER
            //   formatAttribs.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
            //   formatAttribs.push_back(GL_TRUE);
#endif
            // terminate the list
            formatAttribs.push_back(0);
            UINT numFormats;
            wglChoosePixelFormatARB(_hdc, &formatAttribs[0], NULL, 1, &pixelFormat, &numFormats);
            DescribePixelFormat(_hdc, pixelFormat, sizeof(pfd), &pfd);
        }
        // The swap chain  pixel size for swap chains is : rgba32 + depth24stencil8
        // We don't apply the length of the swap chain into this pixelSize since it is not vsible for the Process (on windows).
        _swapchainPixelSize = 32 + 32;

        SetPixelFormat(_hdc, pixelFormat, &pfd);
        {
            std::vector<int> contextAttribs;
            uint32_t majorVersion = _version >> 8;
            uint32_t minorVersion = _version & 0xFF;
            contextAttribs.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
            contextAttribs.push_back(majorVersion);
            contextAttribs.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
            contextAttribs.push_back(minorVersion);
            contextAttribs.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
#if defined(USE_GLES)
            contextAttribs.push_back(WGL_CONTEXT_ES2_PROFILE_BIT_EXT);
#else
            contextAttribs.push_back(WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
#endif
            {
                int contextFlags = 0;
                if (enableDebugLogger()) {
                    contextFlags |= WGL_CONTEXT_DEBUG_BIT_ARB;
                }
#ifdef USE_KHR_ROBUSTNESS
                contextFlags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
#endif
                if (contextFlags != 0) {
                    contextAttribs.push_back(WGL_CONTEXT_FLAGS_ARB);
                    contextAttribs.push_back(contextFlags);
                }
            }
            contextAttribs.push_back(0);
            HGLRC shareHglrc = nullptr;
            if (shareContext) {
                auto nativeContextPointer = QOpenGLContextWrapper(shareContext).getNativeContext();
                shareHglrc = (HGLRC)nativeContextPointer->context();
            }
            _hglrc = wglCreateContextAttribsARB(_hdc, shareHglrc, &contextAttribs[0]);
        }

        if (_hglrc != 0) {
            createWrapperContext();
        }
    }

    if (_hglrc == 0) {
        // fallback, if the context creation failed, or USE_CUSTOM_CONTEXT is false
        qtCreate(shareContext);
    }
#else
    // For platforms where we don't prefer custom context creation, always rely on the Qt context creation
    qtCreate(shareContext);
#endif

    updateSwapchainMemoryCounter();

    if (!makeCurrent()) {
        qCWarning(glLogging) << "Could not make context current";
        return;
    }
    if (enableDebugLogger()) {
        setupDebugLogging(_qglContext);
    }
    doneCurrent();
}

OffscreenContext::~OffscreenContext() {
    _window->deleteLater();
}

void OffscreenContext::create(QOpenGLContext* shareContext) {
    if (!_window) {
        _window = new QWindow();
        _window->setFlags(Qt::MSWindowsOwnDC);
        _window->setSurfaceType(QSurface::OpenGLSurface); // VKTODO
        //_window->setSurfaceType(QSurface::VulkanSurface);
        _window->create();
        setWindow(_window);
        QSize windowSize = _window->size() * _window->devicePixelRatio();
        qCDebug(glLogging) << "New Offscreen GLContext, window size = " << windowSize.width() << " , " << windowSize.height();
        QGuiApplication::processEvents();
    }
    Parent::create(shareContext);
}
