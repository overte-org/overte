/*
mouseLook.js – mouse look switching script
by rampa3 (https://github.com/rampa3) and vegaslon (https://github.com/vegaslon)
*/
(function() { // BEGIN LOCAL_SCOPE

	var away;

	var hmd;

	var mouseLookEnabled = false;

	var oldMode;

	var tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

	var tabletUp;

	var tempOff = false;
	
	hmd = AvatarInputs.isHMD;

	if (!hmd){
		if (mouseLookEnabled) {
			if (!tablet.tabletShown){
				Window.displayAnnouncement("Mouse look: ON");
				mouseLookOn();
			} else {
				Window.displayAnnouncement("Tablet is up – mouse look temporarily off.");
			}
		}
	}

	Controller.keyPressEvent.connect(onKeyPressEvent);

	function onKeyPressEvent(event) {
		if (!hmd){
			if (event.text === 'm') {
				if (mouseLookEnabled) {
					if (!Camera.getCaptureMouse()){
						tempOff = false;
						Window.displayAnnouncement("Mouse look: ON");
						mouseLookOn();
					} else {
						tempOff = true;
						Window.displayAnnouncement("Mouse look: TEMPORARILY OFF");
						mouseLookOff();
					}
				}
			}
			if (event.text === 'M') {
				if (!mouseLookEnabled){
					Window.displayAnnouncement("Mouse look: ENABLED")
					mouseLookEnabled = true;
					mouseLookOn();
				} else {
					Window.displayAnnouncement("Mouse look: DISABLED")
					mouseLookEnabled = false;
					tempOff = false;
					mouseLookOff();		
				}
			}
		}
	}

	tablet.tabletShownChanged.connect(onTabletShownChanged);

	function onTabletShownChanged() {
		if (!hmd) {
			if (mouseLookEnabled) {
				if (!tablet.toolbarMode) {
					if (tablet.tabletShown) {
						tabletUp = true;
						if (!tempOff) {
							if (!away) {
								Window.displayAnnouncement("Tablet is up – mouse look temporarily off.");
								mouseLookOff();
							}
						}
					} else if (!tablet.tabletShown) {
						tabletUp = false;
						if (!tempOff) {
							if (!away) {
								Window.displayAnnouncement("Tablet hidden – mouse look on.");
								mouseLookOn();
							}
						}
					}
				}
			}
		}
	}

	MyAvatar.wentAway.connect(onWentAway);

	function onWentAway() {
		if (!hmd) {
			if (mouseLookEnabled) {
				away = true;
				if (!tabletUp){
					Window.displayAnnouncement("Away state ON – mouse look temporarily off.")
					tempOff = false;
					mouseLookOff()
				}
			}
		}
	}

	MyAvatar.wentActive.connect(onWentActive);

	function onWentActive() {
		if (!hmd) {
			if (mouseLookEnabled) {
				away = false;
				if (!tabletUp) {
					Window.displayAnnouncement("Away state OFF – mouse look on.");
					mouseLookOn();
				}
			}
		}
	}

	AvatarInputs.isHMDChanged.connect(onIsHMDChanged);

	function onIsHMDChanged() {
		if (mouseLookEnabled) {
			if (AvatarInputs.isHMD) {
				hmd = true;
				mouseLookOff();
			} else {
				hmd = false;
				if (!tempOff) {
					if (!tabletUp) {
						mouseLookOn();
					}
				}
			}
		}
	}

	function mouseLookOn() {
		oldMode = Camera.mode;
		Camera.mode = "first person";
		Camera.captureMouse = true;
	}

	function mouseLookOff() {
		Camera.captureMouse = false;
		Camera.mode = oldMode;
	}

	function onCameraModeUpdated(newMode) {
		if (Camera.getCaptureMouse()){
			Camera.captureMouse = false;
		}
	}

	Camera.modeUpdated.connect(onCameraModeUpdated);

	Script.scriptEnding.connect(onScriptEnding);

	function onScriptEnding() {
		Camera.captureMouse = false;
		Controller.keyPressEvent.disconnect(onKeyPressEvent);
		tablet.tabletShownChanged.disconnect(onTabletShownChanged);
		MyAvatar.wentAway.disconnect(onWentAway);
		MyAvatar.wentActive.disconnect(onWentActive);
		AvatarInputs.isHMDChanged.disconnect(onIsHMDChanged);
		Camera.modeUpdated.disconnect(onCameraModeUpdated);
		Script.scriptEnding.disconnect(onScriptEnding);
	}

}()); // END LOCAL_SCOPE