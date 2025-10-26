import QtQuick
import QtQuick.Layouts

import "../" as Overte
import "." as OverteKeyboard

Rectangle {
	id: keyboardRoot
	color: Overte.Theme.paletteActive.base
	border.color: Qt.lighter(color)
	border.width: 2
	radius: 12
	width: 1300
	height: 380

	PropertyAnimation on opacity {
		from: 0
		to: 1
		duration: 500
	}

	// TODO: replace this with Keybaord.layout or something
	Component.onCompleted: {
		const xhr = new XMLHttpRequest;
		xhr.open("GET", "../../keyboard_ansi.json");
		xhr.onreadystatechange = () => {
			if (xhr.readyState === XMLHttpRequest.DONE) {
				buildLayout(JSON.parse(xhr.responseText));
			}
		};
		xhr.send();
	}

	function keyPressed(key) {
		// TODO
		print(`Press ${key.legend}, ${key.unshifted_keycode}`);

		if (key.unshifted_char !== "") {
			modifiers = 0;
		}
	}

	function keyReleased(key) {
		// TODO
		print(`Release ${key.legend}, ${key.unshifted_keycode}`);

		switch (key.unshifted_keycode) {
			case Qt.Key_Shift: modifiers ^= Qt.ShiftModifier; break;
			case Qt.Key_Control: modifiers ^= Qt.ControlModifier; break;
			case Qt.Key_Alt: modifiers ^= Qt.AltModifier; break;
			case Qt.Key_Meta: modifiers ^= Qt.MetaModifier; break;
		}
	}

	property int modifiers: 0

	Component {
		id: columnLayout
		ColumnLayout {
			property real horizontalSpan: 1.0
			property real verticalSpan: 1.0

			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.horizontalStretchFactor: Math.round(horizontalSpan * 1000)
			Layout.verticalStretchFactor: Math.round(verticalSpan * 1000)
		}
	}
	Component {
		id: rowLayout
		RowLayout {
			property real horizontalSpan: 1.0
			property real verticalSpan: 1.0

			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.horizontalStretchFactor: Math.round(horizontalSpan * 1000)
			Layout.verticalStretchFactor: Math.round(verticalSpan * 1000)
		}
	}
	Component {
		id: item

		Item {
			property real horizontalSpan: 1.0
			property real verticalSpan: 1.0

			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.horizontalStretchFactor: Math.round(horizontalSpan * 1000)
			Layout.verticalStretchFactor: Math.round(verticalSpan * 1000)
			implicitWidth: 1
			implicitHeight: 1
		}
	}
	Component {
		id: keyboardKey
		KeyboardKey {
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.horizontalStretchFactor: Math.round(span * 1000)
			implicitWidth: 1
			implicitHeight: 1
		}
	}

	function buildLayout(layout) {
		for (const topColumn of layout) {
			let topColumnSpan = 1.0;
			if (!isNaN(topColumn[0])) {
				topColumnSpan = topColumn[0];
			}

			const topColumnItem = columnLayout.createObject(layoutRoot, {
				horizontalSpan: topColumnSpan,
			});

			for (const row of topColumn) {
				// the span width we handled earlier
				if (!isNaN(parseFloat(row))) { continue; }

				// vertical spacer
				if (!Array.isArray(row)) {
					item.createObject(topColumnItem, {
						verticalSpan: row.verticalSpan ?? 1.0,
					});
					continue;
				}

				const rowItem = rowLayout.createObject(topColumnItem);

				for (const key of row) {
					if (key.legend) {
						const o = keyboardKey.createObject(rowItem, {
							unshifted_keycode: Qt[key.keycode ?? key.unshifted_keycode] ?? 0,
							shifted_keycode: Qt[key.keycode ?? key.shifted_keycode] ?? 0,
							unshifted_char: key.unshifted_char ?? "",
							shifted_char: key.shifted_char ?? "",
							legend: key.legend ?? "",
							span: key.span ?? 1.0,
						});

						if (
							o.unshifted_keycode === Qt.Key_Insert ||
							o.unshifted_keycode === Qt.Key_Home ||
							o.unshifted_keycode === Qt.Key_PageUp ||
							o.unshifted_keycode === Qt.Key_Delete ||
							o.unshifted_keycode === Qt.Key_End ||
							o.unshifted_keycode === Qt.Key_PageDown
						) {
							o.font.pixelSize = Overte.Theme.fontPixelSizeSmall;
						}

						if (
							o.unshifted_keycode === Qt.Key_CapsLock ||
							o.unshifted_keycode === Qt.Key_Shift ||
							o.unshifted_keycode === Qt.Key_Control ||
							o.unshifted_keycode === Qt.Key_Alt ||
							o.unshifted_keycode === Qt.Key_Meta
						) {
							o.checkable = true;
						}
					} else {
						// horizontal spacer
						item.createObject(rowItem, {
							horizontalSpan: key.span ?? 1.0,
						});
					}
				}
			}
		}
	}

	RowLayout {
		id: layoutRoot
		anchors.fill: parent
		anchors.margins: 12
	}

	Overte.RoundButton {
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.margins: 6
		horizontalPadding: 2
		verticalPadding: 2
		width: 40
		height: 40

		backgroundColor: Overte.Theme.paletteActive.buttonDestructive
		text: "🗙"
	}
}
