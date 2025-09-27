//
//  SecondaryCamera.h
//  interface/src
//
//  Created by Samuel Gateau, Howard Stearns, and Zach Fox on 2017-06-08.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_SecondaryCamera_h
#define hifi_SecondaryCamera_h

#include <render/RenderFetchCullSortTask.h>
#include <TextureCache.h>
#include <ViewFrustum.h>

class SecondaryCameraJobConfig : public render::Task::Config { // Exposes secondary camera parameters to JavaScript.
    Q_OBJECT
    Q_PROPERTY(QUuid attachedEntityId MEMBER attachedEntityId NOTIFY dirty)  // entity whose properties define camera position and orientation
    Q_PROPERTY(QUuid portalEntranceEntityId MEMBER portalEntranceEntityId NOTIFY dirty)  // entity whose properties define a portal's entrance position and orientation
    Q_PROPERTY(glm::vec3 position READ getPosition WRITE setPosition)  // of viewpoint to render from
    Q_PROPERTY(glm::qua<float,glm::packed_highp> orientation READ getOrientation WRITE setOrientation)  // of viewpoint to render from
    Q_PROPERTY(float vFoV MEMBER vFoV NOTIFY dirty)  // Secondary camera's vertical field of view. In degrees.
    Q_PROPERTY(float nearClipPlaneDistance MEMBER nearClipPlaneDistance NOTIFY dirty)  // Secondary camera's near clip plane distance. In meters.
    Q_PROPERTY(float farClipPlaneDistance MEMBER farClipPlaneDistance NOTIFY dirty)  // Secondary camera's far clip plane distance. In meters.
    Q_PROPERTY(bool mirrorProjection MEMBER mirrorProjection NOTIFY dirty)  // Flag to use attached mirror entity to build frustum for the mirror and set mirrored camera position/orientation.
    Q_PROPERTY(bool portalProjection MEMBER portalProjection NOTIFY dirty)  // Flag to use attached portal entity to build frustum for the portal and set portal camera position/orientation.
public:
    QUuid attachedEntityId;
    QUuid portalEntranceEntityId;
    glm::vec3 position;
    glm::qua<float,glm::packed_highp> orientation;
    float vFoV { DEFAULT_FIELD_OF_VIEW_DEGREES };
    float nearClipPlaneDistance { DEFAULT_NEAR_CLIP };
    float farClipPlaneDistance { DEFAULT_FAR_CLIP };
    int textureWidth { TextureCache::DEFAULT_SPECTATOR_CAM_WIDTH };
    int textureHeight { TextureCache::DEFAULT_SPECTATOR_CAM_HEIGHT };
    bool mirrorProjection { false };
    bool portalProjection { false };

    SecondaryCameraJobConfig() : render::Task::Config(false) {}
signals:
    void dirty();
public slots:
    glm::vec3 getPosition() { return position; }
    void setPosition(glm::vec3 pos);
    glm::qua<float,glm::packed_highp> getOrientation() { return orientation; }
    void setOrientation(glm::qua<float,glm::packed_highp> orient);
    void enableSecondaryCameraRenderConfigs(bool enabled);
    void resetSizeSpectatorCamera(int width, int height);
};

class SecondaryCameraRenderTaskConfig : public render::Task::Config {
    Q_OBJECT
public:
    SecondaryCameraRenderTaskConfig() : render::Task::Config(false) {}
};

class SecondaryCameraRenderTask {
public:
    using Config = SecondaryCameraRenderTaskConfig;
    using JobModel = render::Task::Model<SecondaryCameraRenderTask, Config>;
    SecondaryCameraRenderTask() {}
    void configure(const Config& config) {}
    void build(JobModel& task, const render::Varying& inputs, render::Varying& outputs, render::CullFunctor cullFunctor);
};

#endif
