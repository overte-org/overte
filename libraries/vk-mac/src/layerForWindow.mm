//
// Created by Julian Groß on 15.05.26.
//

#include <QtGui/QWindow>
#include <qpa/qplatformnativeinterface.h>
#include <AppKit/AppKit.h>
#include <QuartzCore/QuartzCore.h>

// Get the CAMetalLayer to create a MoltenVk surface on inside a given QWindow.
CAMetalLayer* layerForWindow(QWindow *window)
{
    Q_ASSERT(window);
#ifdef Q_OS_MACOS
    NSView *view = reinterpret_cast<NSView *>(window->winId());
#else
    UIView *view = reinterpret_cast<UIView *>(window->winId());
#endif
    Q_ASSERT(view);
    return static_cast<CAMetalLayer *>(view.layer);
}
