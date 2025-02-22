//
//  Application_Camera.cpp
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Application.h"

#include <glm/gtx/transform.hpp>

#include <controllers/UserInputMapper.h>
#include <PickManager.h>
#include <raypick/RayPick.h>
#include <SecondaryCamera.h>

#include "avatar/MyAvatar.h"
#include "avatar/MyHead.h"
#include "Menu.h"

static const float MIRROR_FULLSCREEN_DISTANCE = 0.789f;

void Application::copyViewFrustum(ViewFrustum& viewOut) const {
    QMutexLocker viewLocker(&_viewMutex);
    viewOut = _viewFrustum;
}

void Application::copyDisplayViewFrustum(ViewFrustum& viewOut) const {
    QMutexLocker viewLocker(&_viewMutex);
    viewOut = _displayViewFrustum;
}

void Application::updateCamera(RenderArgs& renderArgs, float deltaTime) {
    PROFILE_RANGE(render, __FUNCTION__);
    PerformanceTimer perfTimer("updateCamera");

    auto myAvatar = getMyAvatar();

    // The render mode is default or mirror if the camera is in mirror mode, assigned further below
    renderArgs._renderMode = RenderArgs::DEFAULT_RENDER_MODE;

    // Always use the default eye position, not the actual head eye position.
    // Using the latter will cause the camera to wobble with idle animations,
    // or with changes from the face tracker
    CameraMode mode = _myCamera.getMode();
    if (mode == CAMERA_MODE_FIRST_PERSON || mode == CAMERA_MODE_FIRST_PERSON_LOOK_AT) {
        _thirdPersonHMDCameraBoomValid= false;
        if (isHMDMode()) {
            mat4 camMat = myAvatar->getSensorToWorldMatrix() * myAvatar->getHMDSensorMatrix();
            _myCamera.setPosition(extractTranslation(camMat));
            _myCamera.setOrientation(glmExtractRotation(camMat));
        } else if (mode == CAMERA_MODE_FIRST_PERSON) {
            _myCamera.setPosition(myAvatar->getDefaultEyePosition());
            _myCamera.setOrientation(myAvatar->getMyHead()->getHeadOrientation());
        } else {
            _myCamera.setPosition(myAvatar->getCameraEyesPosition(deltaTime));
            _myCamera.setOrientation(myAvatar->getLookAtRotation());
        }
    } else if (mode == CAMERA_MODE_THIRD_PERSON || mode == CAMERA_MODE_LOOK_AT || mode == CAMERA_MODE_SELFIE) {
        if (isHMDMode()) {
            if (!_thirdPersonHMDCameraBoomValid) {
                const glm::vec3 CAMERA_OFFSET = glm::vec3(0.0f, 0.0f, 0.7f);
                _thirdPersonHMDCameraBoom = cancelOutRollAndPitch(myAvatar->getHMDSensorOrientation()) * CAMERA_OFFSET;
                _thirdPersonHMDCameraBoomValid = true;
            }

            glm::mat4 thirdPersonCameraSensorToWorldMatrix = myAvatar->getSensorToWorldMatrix();

            const glm::vec3 cameraPos = myAvatar->getHMDSensorPosition() + _thirdPersonHMDCameraBoom * myAvatar->getBoomLength();
            glm::mat4 sensorCameraMat = createMatFromQuatAndPos(myAvatar->getHMDSensorOrientation(), cameraPos);
            glm::mat4 worldCameraMat = thirdPersonCameraSensorToWorldMatrix * sensorCameraMat;

            _myCamera.setOrientation(glm::normalize(glmExtractRotation(worldCameraMat)));
            _myCamera.setPosition(extractTranslation(worldCameraMat));
        } else {
            float boomLength = myAvatar->getBoomLength();
            if (getCameraClippingEnabled()) {
                auto result =
                    DependencyManager::get<PickManager>()->getPrevPickResultTyped<RayPickResult>(_cameraClippingRayPickID);
                if (result && result->doesIntersect()) {
                    const float CAMERA_CLIPPING_EPSILON = 0.1f;
                    boomLength = std::min(boomLength, result->distance - CAMERA_CLIPPING_EPSILON);
                }
            }
            glm::vec3 boomOffset = myAvatar->getModelScale() * boomLength * -IDENTITY_FORWARD;
            _thirdPersonHMDCameraBoomValid = false;
            if (mode == CAMERA_MODE_THIRD_PERSON) {
                _myCamera.setOrientation(myAvatar->getHead()->getOrientation());
                if (isOptionChecked(MenuOption::CenterPlayerInView)) {
                    _myCamera.setPosition(myAvatar->getDefaultEyePosition()
                        + _myCamera.getOrientation() * boomOffset);
                } else {
                    _myCamera.setPosition(myAvatar->getDefaultEyePosition()
                        + myAvatar->getWorldOrientation() * boomOffset);
                }
            } else {
                glm::quat lookAtRotation = myAvatar->getLookAtRotation();
                if (mode == CAMERA_MODE_SELFIE) {
                    lookAtRotation = lookAtRotation * glm::angleAxis(PI, myAvatar->getWorldOrientation() * Vectors::UP);
                }
                _myCamera.setPosition(myAvatar->getLookAtPivotPoint()
                    + lookAtRotation * boomOffset);
                _myCamera.lookAt(myAvatar->getLookAtPivotPoint());
            }
        }
    } else if (mode == CAMERA_MODE_MIRROR) {
        _thirdPersonHMDCameraBoomValid= false;

        if (isHMDMode()) {
            auto mirrorBodyOrientation = myAvatar->getWorldOrientation() * glm::quat(glm::vec3(0.0f, PI + _mirrorYawOffset, 0.0f));

            glm::quat hmdRotation = extractRotation(myAvatar->getHMDSensorMatrix());
            // Mirror HMD yaw and roll
            glm::vec3 mirrorHmdEulers = glm::eulerAngles(hmdRotation);
            mirrorHmdEulers.y = -mirrorHmdEulers.y;
            mirrorHmdEulers.z = -mirrorHmdEulers.z;
            glm::quat mirrorHmdRotation = glm::quat(mirrorHmdEulers);

            glm::quat worldMirrorRotation = mirrorBodyOrientation * mirrorHmdRotation;

            _myCamera.setOrientation(worldMirrorRotation);

            glm::vec3 hmdOffset = extractTranslation(myAvatar->getHMDSensorMatrix());
            // Mirror HMD lateral offsets
            hmdOffset.x = -hmdOffset.x;

            _myCamera.setPosition(myAvatar->getDefaultEyePosition()
                + glm::vec3(0, _raiseMirror * myAvatar->getModelScale(), 0)
                + mirrorBodyOrientation * glm::vec3(0.0f, 0.0f, 1.0f) * MIRROR_FULLSCREEN_DISTANCE * _scaleMirror
                + mirrorBodyOrientation * hmdOffset);
        } else {
            auto userInputMapper = DependencyManager::get<UserInputMapper>();
            const float YAW_SPEED = TWO_PI / 5.0f;
            float deltaYaw = userInputMapper->getActionState(controller::Action::YAW) * YAW_SPEED * deltaTime;
            _mirrorYawOffset += deltaYaw;
            _myCamera.setOrientation(myAvatar->getWorldOrientation() * glm::quat(glm::vec3(0.0f, PI + _mirrorYawOffset, 0.0f)));
            _myCamera.setPosition(myAvatar->getDefaultEyePosition()
                + glm::vec3(0, _raiseMirror * myAvatar->getModelScale(), 0)
                + (myAvatar->getWorldOrientation() * glm::quat(glm::vec3(0.0f, _mirrorYawOffset, 0.0f))) *
                glm::vec3(0.0f, 0.0f, -1.0f) * myAvatar->getBoomLength() * _scaleMirror);
        }
        renderArgs._renderMode = RenderArgs::MIRROR_RENDER_MODE;
    } else if (mode == CAMERA_MODE_ENTITY) {
        _thirdPersonHMDCameraBoomValid= false;
        EntityItemPointer cameraEntity = _myCamera.getCameraEntityPointer();
        if (cameraEntity != nullptr) {
            if (isHMDMode()) {
                glm::quat hmdRotation = extractRotation(myAvatar->getHMDSensorMatrix());
                _myCamera.setOrientation(cameraEntity->getWorldOrientation() * hmdRotation);
                glm::vec3 hmdOffset = extractTranslation(myAvatar->getHMDSensorMatrix());
                _myCamera.setPosition(cameraEntity->getWorldPosition() + (hmdRotation * hmdOffset));
            } else {
                _myCamera.setOrientation(cameraEntity->getWorldOrientation());
                _myCamera.setPosition(cameraEntity->getWorldPosition());
            }
        }
    }
    // Update camera position
    if (!isHMDMode()) {
        _myCamera.update();
    }

    renderArgs._cameraMode = (int8_t)mode;

    const bool shouldEnableCameraClipping =
        (mode == CAMERA_MODE_THIRD_PERSON || mode == CAMERA_MODE_LOOK_AT || mode == CAMERA_MODE_SELFIE) && !isHMDMode() &&
        getCameraClippingEnabled();
    if (_prevCameraClippingEnabled != shouldEnableCameraClipping) {
        if (shouldEnableCameraClipping) {
            DependencyManager::get<PickManager>()->enablePick(_cameraClippingRayPickID);
        } else {
            DependencyManager::get<PickManager>()->disablePick(_cameraClippingRayPickID);
        }
        _prevCameraClippingEnabled = shouldEnableCameraClipping;
    }
}

void Application::updateSecondaryCameraViewFrustum() {
    // TODO: Fix this by modeling the way the secondary camera works on how the main camera works
    // ie. Use a camera object stored in the game logic and informs the Engine on where the secondary
    // camera should be.

    // Code based on SecondaryCameraJob
    auto renderConfig = _graphicsEngine->getRenderEngine()->getConfiguration();
    assert(renderConfig);
    auto camera = dynamic_cast<SecondaryCameraJobConfig*>(renderConfig->getConfig("SecondaryCamera"));

    if (!camera || !camera->isEnabled()) {
        return;
    }

    ViewFrustum secondaryViewFrustum;
    if (camera->portalProjection && !camera->attachedEntityId.isNull() && !camera->portalEntranceEntityId.isNull()) {
        auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
        EntityItemPointer portalEntrance = qApp->getEntities()->getTree()->findEntityByID(camera->portalEntranceEntityId);
        EntityItemPointer portalExit = qApp->getEntities()->getTree()->findEntityByID(camera->attachedEntityId);

        glm::vec3 portalEntrancePropertiesPosition = portalEntrance->getWorldPosition();
        glm::quat portalEntrancePropertiesRotation = portalEntrance->getWorldOrientation();
        glm::mat4 worldFromPortalEntranceRotation = glm::mat4_cast(portalEntrancePropertiesRotation);
        glm::mat4 worldFromPortalEntranceTranslation = glm::translate(portalEntrancePropertiesPosition);
        glm::mat4 worldFromPortalEntrance = worldFromPortalEntranceTranslation * worldFromPortalEntranceRotation;
        glm::mat4 portalEntranceFromWorld = glm::inverse(worldFromPortalEntrance);

        glm::vec3 portalExitPropertiesPosition = portalExit->getWorldPosition();
        glm::quat portalExitPropertiesRotation = portalExit->getWorldOrientation();
        glm::vec3 portalExitPropertiesDimensions = portalExit->getScaledDimensions();
        glm::vec3 halfPortalExitPropertiesDimensions = 0.5f * portalExitPropertiesDimensions;

        glm::mat4 worldFromPortalExitRotation = glm::mat4_cast(portalExitPropertiesRotation);
        glm::mat4 worldFromPortalExitTranslation = glm::translate(portalExitPropertiesPosition);
        glm::mat4 worldFromPortalExit = worldFromPortalExitTranslation * worldFromPortalExitRotation;

        glm::vec3 mainCameraPositionWorld = getCamera().getPosition();
        glm::vec3 mainCameraPositionPortalEntrance = vec3(portalEntranceFromWorld * vec4(mainCameraPositionWorld, 1.0f));
        mainCameraPositionPortalEntrance = vec3(-mainCameraPositionPortalEntrance.x, mainCameraPositionPortalEntrance.y,
            -mainCameraPositionPortalEntrance.z);
        glm::vec3 portalExitCameraPositionWorld = vec3(worldFromPortalExit * vec4(mainCameraPositionPortalEntrance, 1.0f));

        secondaryViewFrustum.setPosition(portalExitCameraPositionWorld);
        secondaryViewFrustum.setOrientation(portalExitPropertiesRotation);

        float nearClip = mainCameraPositionPortalEntrance.z + portalExitPropertiesDimensions.z * 2.0f;
        // `mainCameraPositionPortalEntrance` should technically be `mainCameraPositionPortalExit`,
        // but the values are the same.
        glm::vec3 upperRight = halfPortalExitPropertiesDimensions - mainCameraPositionPortalEntrance;
        glm::vec3 bottomLeft = -halfPortalExitPropertiesDimensions - mainCameraPositionPortalEntrance;
        glm::mat4 frustum = glm::frustum(bottomLeft.x, upperRight.x, bottomLeft.y, upperRight.y, nearClip, camera->farClipPlaneDistance);
        secondaryViewFrustum.setProjection(frustum);
    } else if (camera->mirrorProjection && !camera->attachedEntityId.isNull()) {
        auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
        auto entityProperties = entityScriptingInterface->getEntityProperties(camera->attachedEntityId);
        glm::vec3 mirrorPropertiesPosition = entityProperties.getPosition();
        glm::quat mirrorPropertiesRotation = entityProperties.getRotation();
        glm::vec3 mirrorPropertiesDimensions = entityProperties.getDimensions();
        glm::vec3 halfMirrorPropertiesDimensions = 0.5f * mirrorPropertiesDimensions;

        // setup mirror from world as inverse of world from mirror transformation using inverted x and z for mirrored image
        // TODO: we are assuming here that UP is world y-axis
        glm::mat4 worldFromMirrorRotation = glm::mat4_cast(mirrorPropertiesRotation) * glm::scale(vec3(-1.0f, 1.0f, -1.0f));
        glm::mat4 worldFromMirrorTranslation = glm::translate(mirrorPropertiesPosition);
        glm::mat4 worldFromMirror = worldFromMirrorTranslation * worldFromMirrorRotation;
        glm::mat4 mirrorFromWorld = glm::inverse(worldFromMirror);

        // get mirror camera position by reflecting main camera position's z coordinate in mirror space
        glm::vec3 mainCameraPositionWorld = getCamera().getPosition();
        glm::vec3 mainCameraPositionMirror = vec3(mirrorFromWorld * vec4(mainCameraPositionWorld, 1.0f));
        glm::vec3 mirrorCameraPositionMirror = vec3(mainCameraPositionMirror.x, mainCameraPositionMirror.y,
                                                    -mainCameraPositionMirror.z);
        glm::vec3 mirrorCameraPositionWorld = vec3(worldFromMirror * vec4(mirrorCameraPositionMirror, 1.0f));

        // set frustum position to be mirrored camera and set orientation to mirror's adjusted rotation
        glm::quat mirrorCameraOrientation = glm::quat_cast(worldFromMirrorRotation);
        secondaryViewFrustum.setPosition(mirrorCameraPositionWorld);
        secondaryViewFrustum.setOrientation(mirrorCameraOrientation);

        // build frustum using mirror space translation of mirrored camera
        float nearClip = mirrorCameraPositionMirror.z + mirrorPropertiesDimensions.z * 2.0f;
        glm::vec3 upperRight = halfMirrorPropertiesDimensions - mirrorCameraPositionMirror;
        glm::vec3 bottomLeft = -halfMirrorPropertiesDimensions - mirrorCameraPositionMirror;
        glm::mat4 frustum = glm::frustum(bottomLeft.x, upperRight.x, bottomLeft.y, upperRight.y, nearClip, camera->farClipPlaneDistance);
        secondaryViewFrustum.setProjection(frustum);
    } else {
        if (!camera->attachedEntityId.isNull()) {
            auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
            auto entityProperties = entityScriptingInterface->getEntityProperties(camera->attachedEntityId);
            secondaryViewFrustum.setPosition(entityProperties.getPosition());
            secondaryViewFrustum.setOrientation(entityProperties.getRotation());
        } else {
            secondaryViewFrustum.setPosition(camera->position);
            secondaryViewFrustum.setOrientation(camera->orientation);
        }

        float aspectRatio = (float)camera->textureWidth / (float)camera->textureHeight;
        secondaryViewFrustum.setProjection(camera->vFoV,
                                            aspectRatio,
                                            camera->nearClipPlaneDistance,
                                            camera->farClipPlaneDistance);
    }
    // Without calculating the bound planes, the secondary camera will use the same culling frustum as the main camera,
    // which is not what we want here.
    secondaryViewFrustum.calculate();

    _conicalViews.push_back(secondaryViewFrustum);
}

void Application::setFieldOfView(float fov) {
    if (fov != _fieldOfView.get()) {
        _fieldOfView.set(fov);
        resizeGL();
    }
}

void Application::setCameraClippingEnabled(bool enabled) {
    _cameraClippingEnabled.set(enabled);
    _prevCameraClippingEnabled = enabled;
    if (enabled) {
        DependencyManager::get<PickManager>()->enablePick(_cameraClippingRayPickID);
    } else {
        DependencyManager::get<PickManager>()->disablePick(_cameraClippingRayPickID);
    }
}

// Called during Application::update immediately before AvatarManager::updateMyAvatar, updating my data that is then sent
// to everyone.
// The principal result is to call updateLookAtTargetAvatar() and then setLookAtPosition().
// Note that it is called BEFORE we update position or joints based on sensors, etc.
void Application::updateMyAvatarLookAtPosition(float deltaTime) {
    PerformanceTimer perfTimer("lookAt");
    bool showWarnings = Menu::getInstance()->isOptionChecked(MenuOption::PipelineWarnings);
    PerformanceWarning warn(showWarnings, "Application::updateMyAvatarLookAtPosition()");

    auto myAvatar = getMyAvatar();
    myAvatar->updateEyesLookAtPosition(deltaTime);
}

void Application::cycleCamera() {
    auto menu = Menu::getInstance();
    if (menu->isOptionChecked(MenuOption::FirstPersonLookAt)) {

        menu->setIsOptionChecked(MenuOption::FirstPersonLookAt, false);
        menu->setIsOptionChecked(MenuOption::LookAtCamera, true);

    } else if (menu->isOptionChecked(MenuOption::LookAtCamera)) {

        menu->setIsOptionChecked(MenuOption::LookAtCamera, false);
        if (menu->getActionForOption(MenuOption::SelfieCamera)->isVisible()) {
            menu->setIsOptionChecked(MenuOption::SelfieCamera, true);
        } else {
            menu->setIsOptionChecked(MenuOption::FirstPersonLookAt, true);
        }

    } else if (menu->isOptionChecked(MenuOption::SelfieCamera)) {

        menu->setIsOptionChecked(MenuOption::SelfieCamera, false);
        menu->setIsOptionChecked(MenuOption::FirstPersonLookAt, true);

    }
    cameraMenuChanged(); // handle the menu change
}

void Application::cameraModeChanged() {
    switch (_myCamera.getMode()) {
        case CAMERA_MODE_FIRST_PERSON_LOOK_AT:
            Menu::getInstance()->setIsOptionChecked(MenuOption::FirstPersonLookAt, true);
            break;
        case CAMERA_MODE_LOOK_AT:
            Menu::getInstance()->setIsOptionChecked(MenuOption::LookAtCamera, true);
            break;
        case CAMERA_MODE_SELFIE:
            Menu::getInstance()->setIsOptionChecked(MenuOption::SelfieCamera, true);
            break;
        default:
            // we don't have menu items for the others, so just leave it alone.
            return;
    }
    cameraMenuChanged();
}

void Application::cameraMenuChanged() {
    auto menu = Menu::getInstance();
    if (menu->isOptionChecked(MenuOption::FirstPersonLookAt)) {
        if (_myCamera.getMode() != CAMERA_MODE_FIRST_PERSON_LOOK_AT) {
            _myCamera.setMode(CAMERA_MODE_FIRST_PERSON_LOOK_AT);
            getMyAvatar()->setBoomLength(MyAvatar::ZOOM_MIN);
        }
    } else if (menu->isOptionChecked(MenuOption::LookAtCamera)) {
        if (_myCamera.getMode() != CAMERA_MODE_LOOK_AT) {
            _myCamera.setMode(CAMERA_MODE_LOOK_AT);
            if (getMyAvatar()->getBoomLength() == MyAvatar::ZOOM_MIN) {
                getMyAvatar()->setBoomLength(MyAvatar::ZOOM_DEFAULT);
            }
        }
    } else if (menu->isOptionChecked(MenuOption::SelfieCamera)) {
        if (_myCamera.getMode() != CAMERA_MODE_SELFIE) {
            _myCamera.setMode(CAMERA_MODE_SELFIE);
            if (getMyAvatar()->getBoomLength() == MyAvatar::ZOOM_MIN) {
                getMyAvatar()->setBoomLength(MyAvatar::ZOOM_DEFAULT);
            }
        }
    }
}

void Application::changeViewAsNeeded(float boomLength) {
    // Switch between first and third person views as needed
    // This is called when the boom length has changed
    bool boomLengthGreaterThanMinimum = (boomLength > MyAvatar::ZOOM_MIN);

    if (_myCamera.getMode() == CAMERA_MODE_FIRST_PERSON_LOOK_AT && boomLengthGreaterThanMinimum) {
        Menu::getInstance()->setIsOptionChecked(MenuOption::FirstPersonLookAt, false);
        Menu::getInstance()->setIsOptionChecked(MenuOption::LookAtCamera, true);
        cameraMenuChanged();
    } else if (_myCamera.getMode() == CAMERA_MODE_LOOK_AT && !boomLengthGreaterThanMinimum) {
        Menu::getInstance()->setIsOptionChecked(MenuOption::FirstPersonLookAt, true);
        Menu::getInstance()->setIsOptionChecked(MenuOption::LookAtCamera, false);
        cameraMenuChanged();
    }
}
