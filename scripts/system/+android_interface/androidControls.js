"use strict";
//
//  androidControls.js
//
//  Created by keeshii on September 26th, 2023.
//  Copyright 2022-2023 Overte e.V.
//
//  This script read touch screen events and triggers mouse events.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

(function () {

  Script.include("/~/system/libraries/controllerDispatcherUtils.js");
  var DISPATCHER_TOUCH_PROPERTIES = ["id", "position", "rotation", "dimensions", "registrationPoint"];

  var TAP_DELAY = 300;

  function AndroidControls() {
    this.onTouchStartFn = null;
    this.onTouchEndFn = null;
    this.touchStartTime = 0;
  }

  AndroidControls.prototype.intersectsOverlay = function (intersection) {
    if (intersection && intersection.intersects && intersection.overlayID) {
      return true;
    }
    return false;
  };

  AndroidControls.prototype.intersectsEntity = function (intersection) {
    if (intersection && intersection.intersects && intersection.entityID) {
      return true;
    }
    return false;
  };

  AndroidControls.prototype.findRayIntersection = function (pickRay) {
    // Check 3D overlays and entities. Argument is an object with origin and direction.
    var overlayRayIntersection = Overlays.findRayIntersection(pickRay);
    var entityRayIntersection = Entities.findRayIntersection(pickRay, true);
    var isOverlayInters = this.intersectsOverlay(overlayRayIntersection);
    var isEntityInters = this.intersectsEntity(entityRayIntersection);

    if (isOverlayInters && (!isEntityInters || overlayRayIntersection.distance < entityRayIntersection.distance)) {
      return {type: 'overlay', obj: overlayRayIntersection};
    } else if (isEntityInters) {
      return {type: 'entity', obj: entityRayIntersection};
    }
    return false;
  };

  AndroidControls.prototype.createEventProperties = function (entityId, info, eventType) {
    var pointerEvent = {
      type: eventType,
      id: 1,
      pos2D: {x: 0, y: 0},
      pos3D: info.obj.intersection,
      normal: info.obj.surfaceNormal,
      direction: info.obj.direction,
      button: "Primary",
      isPrimaryButton: true,
      isLeftButton: true,
      isPrimaryHeld: eventType === 'Press',
      isSecondaryHeld: false,
      isTertiaryHeld: false,
      keyboardModifiers: 0
    };

    var properties = Entities.getEntityProperties(entityId, DISPATCHER_TOUCH_PROPERTIES);
    if (properties.id === entityId) {
      pointerEvent.pos2D = info.type === "entity"
        ? projectOntoEntityXYPlane(entityId, info.obj.intersection, properties)
        : projectOntoOverlayXYPlane(entityId, info.obj.intersection, properties);
    }

    return pointerEvent;
  };

  AndroidControls.prototype.triggerClick = function (event) {
    var info = this.findRayIntersection(Camera.computePickRay(event.x, event.y));

    if (!info) {
      return;
    }

    var entityId = info.type === "entity" ? info.obj.entityID : info.obj.overlayID;
    var pressEvent = this.createEventProperties(entityId, info, 'Press');
    var releaseEvent = this.createEventProperties(entityId, info, 'Release');

    Entities.sendMousePressOnEntity(entityId, pressEvent);
    Entities.sendClickDownOnEntity(entityId, pressEvent);

    Script.setTimeout(function () {
      Entities.sendMouseReleaseOnEntity(entityId, releaseEvent);
      Entities.sendClickReleaseOnEntity(entityId, releaseEvent);
    }, 75);
  };

  AndroidControls.prototype.onTouchStart = function (_event) {
    this.touchStartTime = Date.now();
  };

  AndroidControls.prototype.onTouchEnd = function (event) {
    var now = Date.now();
    if (now - this.touchStartTime < TAP_DELAY) {
      this.triggerClick(event);
    }
    this.touchStartTime = 0;
  };

  AndroidControls.prototype.init = function () {
    var self = this;
    this.onTouchStartFn = function (ev) {
      self.onTouchStart(ev);
    };
    this.onTouchEndFn = function (ev) {
      self.onTouchEnd(ev);
    };

    Controller.touchBeginEvent.connect(this.onTouchStartFn);
    Controller.touchEndEvent.connect(this.onTouchEndFn);
  };

  AndroidControls.prototype.ending = function () {
    if (this.onTouchStartFn) {
      Controller.touchBeginEvent.disconnect(this.onTouchStartFn);
    }
    if (this.onTouchEndFn) {
      Controller.touchEndEvent.disconnect(this.onTouchEndFn);
    }
    this.touchStartTime = 0;
    this.onTouchStartFn = null;
    this.onTouchEndFn = null;
  };

  var androidControls = new AndroidControls();

  Script.scriptEnding.connect(function () {
    androidControls.ending();
  });
  androidControls.init();

  module.exports = androidControls;
}());
