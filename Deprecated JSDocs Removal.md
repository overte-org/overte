The following is a list of every deprecated API, method, and property that was had its JSDoc removed as a part of commit 4e0bbf4df19cecc9860f4c51b7ec51edfcb9a4a5.

This list includes the removed item as well as the suggested alternative if one was present in the removed JSDoc.

| Name | Replacement |
|---|---|
| Account | AccountServices |
| AddressManager | location and Window.location |
| AnimationCache.updateTotalSize |
| Audio.devices |  |
| Audio.onContextChanged |  |
| Audio.setInputDevice |  |
| Audio.setOutputDevice |  |
| Audio.nop() |  |
| Avatar.attachmentData | Avatar Entities |
| Avatar.getAttachmentsVariant | Avatar Entities |
| Avatar.setAttachmentsVariant | Avatar Entities |
| Avatar.getAttachmentData | Avatar Entities |
| Avatar.clearAvatarEntity | Avatar Entities |
| Avatar.attach | Avatar Entities |
| Avatar.detachAll | Avatar Entities |
| Avatar.detachOne | Avatar Entities |
| Avatar.resetLastSent | |
| Avatar.sendAvatarDataPacket | |
| Avatar.sendAvatarDataPacket | |
| Avatar.sendIdentityPacket | |
| Avatar.setSessionUUID | |
| Avatar.setAttachmentData | Avatar Entities |
| Avatar.setForceFaceTrackerConnected | Avatar.hasScriptedBlendshapes or MyAvatar.hasScriptedBlendshapes |
| Avatar.setJointMappingsFromNetworkReply | |
| Avatar.update | |
| Avatar.updateAvatarEntity |  |
| AvatarBookmarks.deleteBookmark |  |
| AvatarBookmarks.updateAvatarEntities | MyAvatar |
| AvatarBookmarks.deleteBookmark | Avatar Entities |
| AvatarInputs.cameraEnabled |  |
| AvatarInputs.cameraMuted |  |
| AvatarInputs.toggleCameraMute |  |
| AvatarInputs.avatarLeftIgnoreRadius |  |
| AvatarInputs.cameraEnabledChanged |  |
| AvatarInputs.cameraMutedChanged |  |
| AvatarList.processAvatarDataPacket |  |
| AvatarList.processAvatarIdentityPacket |  |
| AvatarList.processBulkAvatarTraits |  |
| AvatarList.processKillAvatar |  |
| AvatarList.sessionUUIDChanged |  |
| AvatarManager.findParabolaIntersectionVector |  |
| AvatarManager.findRayIntersectionVector |  |
| AvatarManager.getAvatarSortCoefficient |  |
| AvatarManager.setAvatarSortCoefficient |  |
| AvatarManager.updateAvatarRenderStatus |  |
| AvatarManager.PalData.isReplicated |  |
| Controller.captureJoystick |  |
| Controller.releaseJoystick |  |
| Controller.updateRunningInputDevices |  |
| Controller.Actions.LeftHandClick |  |
| Controller.Actions.RightHandClick |  |
| Controller.Actions.Shift |  |
| Controller.Actions.PrimaryAction |  |
| Controller.Actions.SecondaryAction |  |
| Controller.Actions.LEFT_HAND | LeftHand |
| Controller.Actions.RIGHT_HAND | RightHand |
| Controller.Actions.BOOM_IN | BoomIn |
| Controller.Actions.BOOM_OUT | BoomOut  |
| Controller.Actions.CONTEXT_MENU | ContextMenu |
| Controller.Actions.TOGGLE_MUTE | ToggleMute |
| Controller.Actions.TOGGLE_PUSHTOTALK | TogglePushToTalk |
| Controller.Actions.SPRINT | Sprint |
| Controller.Actions.LONGITUDINAL_BACKWARD | Backward |
| Controller.Actions.LONGITUDINAL_FORWARD | Forward |
| Controller.Actions.LATERAL_LEFT | StrafeLeft |
| Controller.Actions.LATERAL_RIGHT | StrafeRight |
| Controller.Actions.VERTICAL_UP | Up |
| Controller.Actions.VERTICAL_DOWN | Down |
| Controller.Actions.PITCH_DOWN | PitchDown |
| Controller.Actions.PITCH_UP | PitchUp |
| Controller.Actions.YAW_LEFT | YawLeft |
| Controller.Actions.YAW_RIGHT | YawRight |
| Controller.Actions.LEFT_HAND_CLICK | Says to use LeftHandClick which is also deprecated? |
| Controller.Actions.RIGHT_HAND_CLICK | Says to use RightHandClick which is also deprecated? |
| Controller.Actions.SHIFT | Says to use Shift which is also deprecated? |
| Controller.Actions.ACTION1 | Says to use PrimaryAction which is also deprecated? |
| Controller.Actions.ACTION2 | Says to use SecondaryAction which is also deprecated? |
| Controller.Actions.TrackedObject00 |  |
| Controller.Actions.TrackedObject01 |  |
| Controller.Actions.TrackedObject02 |  |
| Controller.Actions.TrackedObject03 |  |
| Controller.Actions.TrackedObject04 |  |
| Controller.Actions.TrackedObject05 |  |
| Controller.Actions.TrackedObject06 |  |
| Controller.Actions.TrackedObject07 |  |
| Controller.Actions.TrackedObject08 |  |
| Controller.Actions.TrackedObject09 |  |
| Controller.Actions.TrackedObject10 |  |
| Controller.Actions.TrackedObject11 |  |
| Controller.Actions.TrackedObject12 |  |
| Controller.Actions.TrackedObject13 |  |
| Controller.Actions.TrackedObject14 |  |
| Controller.Actions.TrackedObject15 |  |
| Entities.mouseMoveEvent | mouseMoveOnEntity  |
| Entities.appendPoint | PolyLine Entities |
| Entities.getMeshes | Graphics API |
| Entities.setAllPoints | PolyLine Entities |
| Entities.ActionType "spring" |  |
| Entities.EntityProperties.acceleration |  |
| Entities.EntityProperties-Grid.pulse |  |
| Entities.EntityProperties-Grid.alpha |  |
| Entities.EntityProperties-Image.pulse |  |
| Entities.EntityProperties-Image.alpha |  |
| Entities.EntityProperties-Image.faceCamera |  |
| Entities.EntityProperties-Image.isFacingAvatar |  |
| Entities.EntityProperties-Line | PolyLine Entities |
| Entities.EntityProperties-Model.modelScale |  |
| Entities.EntityProperties-ParticleEffect.pulse |  |
| Entities.EntityProperties-Shape.pulse |  |
| Entities.EntityProperties-Text.pulse |  |
| Entities.EntityProperties-Text.faceCamera |  |
| Entities.EntityProperties-Text.isFacingAvatar |  |
| Entities.EntityProperties-Web.pulse |  |
| Entities.EntityProperties-Web.faceCamera |  |
| Entities.EntityProperties-Web.isFacingAvatar |  |
| EntityViewer.getBoundaryLevelAdjust |  |
| EntityViewer.getVoxelSizeScale |  |
| EntityViewer.setVoxelSizeScale |  |
| EntityViewer.setBoundaryLevelAdjust |  |
| EntityViewer.setKeyholeRadius | setCenterRadius  |
| GlobalServices | AccountServices |
| HifiAbout | About |
| LODManager.lodQualityLevel |  |
| LODManager.getBoundaryLevelAdjust |  |
| LODManager.getOctreeSizeScale |  |
| LODManager.setOctreeSizeScale |  |
| LODManager.setBoundaryLevelAdjust |  |
| LODManager.LODDecreased |  |
| LODManager.LODIncreased |  |
| LODManager.lodQualityLevelChanged |  |
| MaterialCache.updateTotalSize |  |
| Midi.midiNote | midiMessage |
| ModelCache.updateTotalSize |  |
| MyAvatar.attachmentData | Avatar Entities |
| MyAvatar.qmlPosition |  |
| MyAvatar.energy |  |
| MyAvatar.characterControllerEnabled | collisionsEnabled |
| MyAvatar.userRecenterModel |  |
| MyAvatar.isSitStandStateLocked |  |
| MyAvatar.addThrust | motorVelocity |
| MyAvatar.animGraphLoaded |  |
| MyAvatar.clearScaleRestriction |  |
| MyAvatar.getCharacterControllerEnabled | getCollisionsEnabled |
| MyAvatar.getSimulationRate |  |
| MyAvatar.getThrust | motorVelocity  |
| MyAvatar.restrictScaleFromDomainSettings |  |
| MyAvatar.rigReady |  |
| MyAvatar.rigReset |  |
| MyAvatar.safeLanding |  |
| MyAvatar.setCharacterControllerEnabled |  |
| MyAvatar.setModelScale |  |
| MyAvatar.setModelURLFinished |  |
| MyAvatar.setThrust |  |
| MyAvatar.setToggleHips |  |
| MyAvatar.attachmentsChanged |  |
| MyAvatar.energyChanged |  |
| MyAvatar.transformChanged |  |
| MyAvatar.IKTargetType.HmdHead |  |
| MyAvatar.SitStandModelType |  |
| MyAvatar.analogPlusSprintSpeed |  |
| Paths | Script.resolvePath and Script.resourcesPath |
| Picks.PICK_ENTITIES | PICK_DOMAIN_ENTITIES or PICK_AVATAR_ENTITIES property |
| Picks.PICK_OVERLAYS | PICK_LOCAL_ENTITIES  |
| Picks.INTERSECTED_OVERLAY | INTERSECTED_LOCAL_ENTITY |
| Picks.INTERSECTED_AVATAR method | INTERSECTED_AVATAR property |
| Picks.INTERSECTED_ENTITY method | INTERSECTED_ENTITY property |
| Picks.INTERSECTED_HUD method | INTERSECTED_HUD property |
| Picks.INTERSECTED_LOCAL_ENTITY method | INTERSECTED_LOCAL_ENTITY property |
| Picks.INTERSECTED_NONE method | INTERSECTED_NONE property |
| Picks.INTERSECTED_OVERLAY method | INTERSECTED_LOCAL_ENTITY property |
| Picks.PICK_ALL_INTERSECTIONS method | PICK_ALL_INTERSECTIONS property |
| Picks.PICK_AVATARS method | PICK_AVATARS property |
| Picks.PICK_AVATAR_ENTITIES method | PICK_DOMAIN_ENTITIES or PICK_AVATAR_ENTITIES property |
| Picks.PICK_COARSE method | PICK_COARSE property |
| Picks.PICK_DOMAIN_ENTITIES method | PICK_DOMAIN_ENTITIES property |
| Picks.PICK_HUD method | PICK_HUD property |
| Picks.PICK_INCLUDE_COLLIDABLE method | PICK_INCLUDE_COLLIDABLE property |
| Picks.PICK_INCLUDE_INVISIBLE method | PICK_INCLUDE_INVISIBLE property |
| Picks.PICK_INCLUDE_NONCOLLIDABLE method | PICK_INCLUDE_NONCOLLIDABLE property |
| Picks.PICK_INCLUDE_VISIBLE method | PICK_INCLUDE_VISIBLE property |
| Picks.PICK_LOCAL_ENTITIES method | PICK_LOCAL_ENTITIES property |
| Picks.PICK_OVERLAYS method | PICK_LOCAL_ENTITIES property |
| Picks.PICK_PRECISE method | PICK_PRECISE property |
| Picks.ParabolaPickProperties.scaleWithAvatar |  |
| PlatformInfo.getCPUBrand | JSON.parse(PlatformInfo.getCPU(0)).model |
| PlatformInfo.getGraphicsCardType | JSON.parse(PlatformInfo.getGPU( PlatformInfo.getMasterGPU() )).model |
| PlatformInfo.getNumLogicalCores | JSON.parse(PlatformInfo.getCPU(0)).numCores |
| PlatformInfo.getOperatingSystemType | JSON.parse({@link PlatformInfo.getComputer|PlatformInfo.getComputer()}).OS |
| PlatformInfo.getTotalSystemMemoryMB | JSON.parse(PlatformInfo.getMemory()).memTotal |
| ParabolaPointerProperties.scaleWithAvatar |  |
| RayPointerProperties.scaleWithAvatar |  |
| Script._requireResolve |  |
| Script.callEntityScriptMethod |  |
| Script.entityScriptContentAvailable |  |
| Script.executeOnScriptThread |  |
| Script.formatException |  |
| Script.generateUUID | Uuid.generate |
| Script.loadEntityScript |  |
| Script.resetModuleCache |  |
| Script.unloadAllEntityScripts |  |
| Script.unloadEntityScript |  |
| Script.clearDebugWindow |  |
| Script.entityScriptDetailsUpdated |  |
| Script.errorLoadingScript |  |
| Script.loadScript |  |
| Script.reloadScript |  |
| Script.scriptLoaded |  |
| Script.stop marshal parameter |  |
| ScriptDiscoveryService.getLocal |  |
| ScriptDiscoveryService.onClearDebugWindow |  |
| ScriptDiscoveryService.onErrorLoadingScript |  |
| ScriptDiscoveryService.onErrorMessage |  |
| ScriptDiscoveryService.onInfoMessage |  |
| ScriptDiscoveryService.onPrintedMessage |  |
| ScriptDiscoveryService.onScriptFinished |  |
| ScriptDiscoveryService.onWarningMessage |  |
| ScriptDiscoveryService.errorLoadingScript |  |
| ScriptDiscoveryService.LocalScript |  |
| ScriptDiscoveryService.PublicScript.type |  |
| ScriptDiscoveryService.PublicScript.children |  |
| SoundCache.updateTotalSize |  |
| Stats.audioAudioInboundPPS | audioInboundPPS  |
| Stats.bgColor |  |
| Stats.activeFocus |  |
| Stats.activeFocusOnTab |  |
| Stats.anchors |  |
| Stats.antialiasing |  |
| Stats.baselineOffset |  |
| Stats.children |  |
| Stats.clip |  |
| Stats.containmentMask |  |
| Stats.enabled |  |
| Stats.focus |  |
| Stats.height |  |
| Stats.implicitHeight |  |
| Stats.implicitWidth |  |
| Stats.layer |  |
| Stats.opacity |  |
| Stats.rotation |  |
| Stats.scale |  |
| Stats.smooth |  |
| Stats.state |  |
| Stats.transformOrigin |  |
| Stats.visible |  |
| Stats.width |  |
| Stats.x |  |
| Stats.y |  |
| Stats.z |  |
| Stats.childAt |  |
| Stats.contains |  |
| Stats.forceActiveFocus |  |
| Stats.grabToImage |  |
| Stats.mapFromGlobal |  |
| Stats.mapFromItem |  |
| Stats.mapToGlobal |  |
| Stats.mapToItem |  |
| Stats.nextItemInFocusChain |  |
| Stats.update |  |
| Stats.activeFocusChanged |  |
| Stats.activeFocusOnTabChanged |  |
| Stats.antialiasingChanged |  |
| Stats.baselineOffsetChanged |  |
| Stats.childrenChanged |  |
| Stats.childrenRectChanged |  |
| Stats.clipChanged |  |
| Stats.containmentMaskChanged |  |
| Stats.enabledChanged |  |
| Stats.focusChanged |  |
| Stats.heightChanged |  |
| Stats.implicitHeightChanged |  |
| Stats.implicitWidthChanged |  |
| Stats.opacityChanged |  |
| Stats.parentChanged |  |
| Stats.rotationChanged |  |
| Stats.scaleChanged |  |
| Stats.smoothChanged |  |
| Stats.stateChanged |  |
| Stats.transformOriginChanged |  |
| Stats.visibleChanged |  |
| Stats.visibleChildrenChanged |  |
| Stats.widthChanged |  |
| Stats.windowChanged |  |
| Stats.xChanged |  |
| Stats.yChanged |  |
| Stats.zChanged |  |
| Stats.audioAudioInboundPPSChanged |  |
| Stats.bgColorChanged |  |
| TextureCache.updateTotalSize |  |
| TextureCache.spectatorCameraFramebufferReset |  |
| Window.domainLoadingProgress |  |
| location.goToViewpointForPath |  |
| location.lookupShareableNameForDomainID |  |
| location.refreshPreviousLookup |  |
| GraphicsMesh.getMeshPointer |  |
| GraphicsMesh.getModelBasePointer |  |
| GraphicsMesh.getModelProviderPointer |  |
| InteractiveWindow.emitWebEvent |  |
| InteractiveWindow.qmlToScript |  |
| InteractiveWindow.scriptEventReceived |  |
| OverlayWebWindow.clearDebugWindow |  |
| OverlayWebWindow.emitWebEvent |  |
| OverlayWebWindow.getEventBridge |  |
| OverlayWebWindow.hasClosed |  |
| OverlayWebWindow.hasMoved |  |
| OverlayWebWindow.initQml |  |
| OverlayWebWindow.qmlToScript |  |
| OverlayWebWindow.sendToQML |  |
| OverlayWebWindow.fromQML |  |
| OverlayWebWindow.scriptEventReceived |  |
| ScriptAvatar.isReplicated |  |
| ScriptAvatar.attachmentData | Avatar Entities |
| ScriptAvatar.getAttachmentData | Avatar Entitites |
| ScriptAvatar.getSimulationRate |  |
| ScriptsModel.downloadFinished |  |
| ScriptsModel.reloadDefaultFiles |  |
| ScriptsModel.reloadLocalFiles |  |
| ScriptsModel.updateScriptsLocation |  |
| TabletProxy.desktopWindowClosed |  |
| TabletProxy.emitWebEvent |  |
| TabletProxy.initialScreen |  |
| TabletProxy.loadHTMLSourceOnTopImpl |  |
| TabletProxy.loadQMLOnTopImpl |  |
| TabletProxy.loadQMLSourceImpl |  |
| TabletProxy.onTabletShown |  |
| TabletProxy.returnToPreviousAppImpl |  |
| TabletProxy.gotoWebScreen loadOtherBase parameter |  |
| ToolbarProxy.addButton |  |
| ToolbarProxy.removeButton |  |