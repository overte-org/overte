import QtQuick
import QtQuick.Controls

import "."

// a stack view with reasonable defaults for the
// animations, respecting the theme reduced motion settings
StackView {
	pushEnter: Transition {
		PropertyAnimation {
			property: "opacity"
			from: Theme.reducedMotion ? 0 : 1
			to: 1
			duration: 80
		}

		PropertyAnimation {
			property: "x"
			from: Theme.reducedMotion ? 0 : width
			to: 0
			duration: 350
			easing.type: Easing.OutQuad
		}
	}
	pushExit: Transition {
		PropertyAnimation {
			property: "opacity"
			from: 1
			to: Theme.reducedMotion ? 0 : 1
			duration: 80
		}

		PropertyAnimation {
			property: "x"
			to: Theme.reducedMotion ? 0 : -width
			from: 0
			duration: 350
			easing.type: Easing.OutQuad
		}
	}

	popEnter: Transition {
		PropertyAnimation {
			property: "opacity"
			from: Theme.reducedMotion ? 0 : 1
			to: 1
			duration: 80
		}

		PropertyAnimation {
			property: "x"
			from: Theme.reducedMotion ? 0 : -width
			to: 0
			duration: 350
			easing.type: Easing.OutQuad
		}
	}
	popExit: Transition {
		PropertyAnimation {
			property: "opacity"
			from: 1
			to: Theme.reducedMotion ? 0 : 1
			duration: 80
		}

		PropertyAnimation {
			property: "x"
			to: Theme.reducedMotion ? 0 : width
			from: 0
			duration: 350
			easing.type: Easing.OutQuad
		}
	}
}
