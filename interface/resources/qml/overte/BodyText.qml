import QtQuick
import QtQuick.Controls

import "."

TextEdit {
	selectByMouse: true
	readOnly: true

	font.pixelSize: Theme.fontPixelSize
	font.family: Theme.bodyFontFamily
	color: Theme.paletteActive.text

	textFormat: TextEdit.PlainText
	wrapMode: TextEdit.Wrap

	selectedTextColor: Theme.paletteActive.highlightedText
	selectionColor: Theme.paletteActive.highlight

	// TODO: handle embedded links
	// Qt doesn't make it easy to theme rich text
	onLinkActivated: link => Qt.openUrlExternally(link)
}
