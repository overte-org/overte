//
//  ApplicationOverlay.h
//  interface/src/ui/overlays
//
//  Created by Benjamin Arnold on 5/27/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ApplicationOverlay_h
#define hifi_ApplicationOverlay_h

#include <gpu/Texture.h>
#include <render/Args.h>


// Handles the drawing of the overlays to the screen
// TODO, move divide up the rendering, displaying and input handling
// facilities of this class
class ApplicationOverlay : public QObject {
    Q_OBJECT
public:
    ApplicationOverlay();
    ~ApplicationOverlay();

    void renderOverlay(RenderArgs* renderArgs);

    gpu::TexturePointer getOverlayTexture();

    bool enabled() const { return _enabled; }
    void setEnabled(bool enabled) { _enabled = enabled; }

private:
    void renderDomainConnectionStatusBorder(RenderArgs* renderArgs);
    void renderQmlUi(RenderArgs* renderArgs);
    void renderOverlays(RenderArgs* renderArgs);
    void buildFramebufferObject();

    int _domainStatusBorder;
    int _magnifierBorder;

    gpu::TexturePointer _uiTexture;
    gpu::TexturePointer _overlayDepthTexture;
    gpu::TexturePointer _overlayColorTexture;
    gpu::FramebufferPointer _overlayFramebuffer;
    int _qmlGeometryId { 0 };

    bool _enabled { true };
};

#endif // hifi_ApplicationOverlay_h
