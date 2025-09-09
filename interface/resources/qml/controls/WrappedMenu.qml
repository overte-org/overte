import QtQuick.Controls

Menu {
    id: wrappedMenu
    objectName: "wrappedMenu"

    function addMenuWrap(menu) {
        return addMenu(menu);
    }
}