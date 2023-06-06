/*
mouseLook.js – mouse look switching script
by rampa3 (https://github.com/rampa3) and vegaslon (https://github.com/vegaslon)
*/
(function() { // BEGIN LOCAL_SCOPE

	var oldMode;

	var mouseLookEnabled = false;

	mouseLookOn();

	Controller.keyPressEvent.connect(function(event) {
		if (event.text === 'm') {
			if (mouseLookEnabled) {
				if (!Camera.getCaptureMouse()){
					Window.displayAnnouncement("Mouse look: ON");
					mouseLookOn();
				} else {
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
				mouseLookOff();		
			}
		}
	});

	var tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
	tablet.tabletShownChanged.connect(function () {
		if (!tablet.toolbarMode) {
			if (tablet.tabletShown) {
				Window.displayAnnouncement("Tablet is up – mouse look temporarily off.");
				mouseLookOff();

			} else if (!tablet.tabletShown) {
				Window.displayAnnouncement("Tablet hidden – mouse look on.");
				mouseLookOn();
			}
		}
	});

	MyAvatar.wentAway.connect(function () {
		Window.displayAnnouncement("Away state ON – mouse look temporarily off.")
		mouseLookOff()
	});

	MyAvatar.wentActive.connect(function () {
		Window.displayAnnouncement("Away state OFF – mouse look on.");
		mouseLookOn();
	});

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

	Script.scriptEnding.connect(function() {
		Camera.captureMouse = false;
	});

}()); // END LOCAL_SCOPE