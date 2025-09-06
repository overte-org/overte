import QtQuick.Controls as Original
//import QtQuick.Controls.Styles
import "../styles"
import "."
Original.CheckBox {
    text: "Check Box"
    style: CheckBoxStyle {
        indicator: FontAwesome {
            text: control.checked ? "\uf046" : "\uf096"
        }
        label: Text {
            text: control.text
            renderType: Text.QtRendering
        }
    }

}
