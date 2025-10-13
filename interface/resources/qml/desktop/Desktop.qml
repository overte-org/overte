//
//  Desktop.qml
//
//  Created by Bradley Austin Davis on 15 Apr 2015
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick
import QtQuick.Controls

import "../dialogs"
import "../js/Utils.js" as Utils
import "../controls" as OverteControls

// This is our primary 'desktop' object to which all VR dialogs and windows are childed.
FocusScope {
    id: desktop
    objectName: "desktop"
    anchors.fill: parent

    readonly property int invalid_position: -9999;
    property rect recommendedRect: Qt.rect(0,0,0,0);
    property var expectedChildren;
    property bool repositionLocked: true
    property bool hmdHandMouseActive: false

    onRepositionLockedChanged: {
        if (!repositionLocked) {
            d.handleSizeChanged();
        }

    }
    
    onHeightChanged: d.handleSizeChanged();

    onWidthChanged: d.handleSizeChanged();

    // Controls and windows can trigger this signal to ensure the desktop becomes visible
    // when they're opened.
    signal showDesktop();

    // This is for JS/QML communication, which is unused in the Desktop,
    // but not having this here results in spurious warnings about a
    // missing signal
    signal sendToScript(var message);

    // Allows QML/JS to find the desktop through the parent chain
    property bool desktopRoot: true

    // The VR version of the primary menu
    property var rootMenu: OverteControls.WrappedMenu {
        id: rootMenuId
        objectName: "rootMenu" 

        property var exclusionGroups: ({});
        property Component exclusiveGroupMaker: Component {
            //ExclusiveGroup { //QT6TODO
            ButtonGroup {
            }
        }

        function addMenuWrap(menu) {
            return addMenu(menu);
        }

        function addExclusionGroup(qmlAction, exclusionGroup) {

            var exclusionGroupId = exclusionGroup.toString();
            if(!exclusionGroups[exclusionGroupId]) {
                exclusionGroups[exclusionGroupId] = exclusiveGroupMaker.createObject(rootMenuId);
            }

            //QT6TODO:
            qmlAction.exclusiveGroup = exclusionGroups[exclusionGroupId]
        }
    }

    // FIXME: Alpha gradients display as fuschia under QtQuick 2.5 on OSX/AMD
    //        because shaders are 4.2, and do not include #version declarations.
    property bool gradientsSupported: Qt.platform.os != "osx" && !~GL.vendor.indexOf("ATI")

    readonly property alias zLevels: zLevels
    QtObject {
        id: zLevels;
        readonly property real normal: 1 // make windows always appear higher than QML overlays and other non-window controls.
        readonly property real top: 2000
        readonly property real modal: 4000
        readonly property real menu: 8000
    }

    QtObject {
        id: d

        function handleSizeChanged() {
            if (desktop.repositionLocked) {
                return;
            }
            var oldRecommendedRect = recommendedRect;
            var newRecommendedRectJS = (typeof Controller === "undefined") ? Qt.rect(0,0,0,0) : Controller.getRecommendedHUDRect();
            var newRecommendedRect = Qt.rect(newRecommendedRectJS.x, newRecommendedRectJS.y,
                                    newRecommendedRectJS.width,
                                    newRecommendedRectJS.height);

            var oldChildren = expectedChildren;
            var newChildren = d.getRepositionChildren();
            if (oldRecommendedRect != Qt.rect(0,0,0,0) && oldRecommendedRect != Qt.rect(0,0,1,1)
                  && (oldRecommendedRect != newRecommendedRect
                      || oldChildren != newChildren)
                ) {
                expectedChildren = newChildren;
                d.repositionAll();
            }
            recommendedRect = newRecommendedRect;
        }

        function findChild(item, name) {
            for (var i = 0; i < item.children.length; ++i) {
                if (item.children[i].objectName === name) {
                    return item.children[i];
                }
            }
            return null;
        }

        function findParentMatching(item, predicate) {
            while (item) {
                if (predicate(item)) {
                    break;
                }
                item = item.parent;
            }
            return item;
        }

        function findMatchingChildren(item, predicate) {
            var results = [];
            for (var i in item.children) {
                var child = item.children[i];
                if (predicate(child)) {
                    results.push(child);
                }
            }
            return results;
        }

        function isTopLevelWindow(item) {
            return item.topLevelWindow;
        }

        function isAlwaysOnTopWindow(window) {
            return window.alwaysOnTop;
        }

        function isModalWindow(window) {
            return window.modality !== (typeof Qt !== 'undefined' ? Qt.NonModal : 0);
        }

        function getTopLevelWindows(predicate) {
            return d.findMatchingChildren(desktop, function(child) {
                return (d.isTopLevelWindow(child) && (!predicate || predicate(child)));
            });
        }

        function getDesktopWindow(item) {
            return d.findParentMatching(item, d.isTopLevelWindow)
        }

        function fixupZOrder(windows, basis, topWindow) {
            windows.sort(function(a, b){ return a.z - b.z; });

            if ((topWindow.z >= basis)  &&  (windows[windows.length - 1] === topWindow)) {
                return;
            }

            var lastZ = -1;
            var lastTargetZ = basis - 1;
            for (var i = 0; i < windows.length; ++i) {
                var window = windows[i];
                if (!window.visible) {
                    continue
                }

                if (topWindow && (topWindow === window)) {
                    continue
                }

                if (window.z > lastZ) {
                    lastZ = window.z;
                    ++lastTargetZ;
                }
                if (DebugQML) {
                    console.log("Assigning z order " + lastTargetZ + " to " + window)
                }

                window.z = lastTargetZ;
            }
            if (topWindow) {
                ++lastTargetZ;
                if (DebugQML) {
                    console.log("Assigning z order " + lastTargetZ + " to " + topWindow)
                }
                topWindow.z = lastTargetZ;
            }

            return lastTargetZ;
        }

        function raiseWindow(targetWindow) {
            var predicate;
            var zBasis;
            if (d.isModalWindow(targetWindow)) {
                predicate = d.isModalWindow;
                zBasis = zLevels.modal
            } else if (d.isAlwaysOnTopWindow(targetWindow)) {
                predicate = function(window) {
                    return (d.isAlwaysOnTopWindow(window) && !d.isModalWindow(window));
                }
                zBasis = zLevels.top
            } else {
                predicate = function(window) {
                    return (!d.isAlwaysOnTopWindow(window) && !d.isModalWindow(window));
                }
                zBasis = zLevels.normal
            }

            var windows = d.getTopLevelWindows(predicate);
            d.fixupZOrder(windows, zBasis, targetWindow);
        }

        Component.onCompleted: {
            //offscreenWindow.activeFocusItemChanged.connect(onWindowFocusChanged);
            focusHack.start();
        }

        function onWindowFocusChanged() {
            //console.log("Focus item is " + offscreenWindow.activeFocusItem);

            // FIXME this needs more testing before it can go into production
            // and I already cant produce any way to have a modal dialog lose focus
            // to a non-modal one.
            /*
            var focusedWindow = getDesktopWindow(offscreenWindow.activeFocusItem);

            if (isModalWindow(focusedWindow)) {
                return;
            }

            // new focused window is not modal... check if there are any modal windows
            var windows = getTopLevelWindows(isModalWindow);
            if (0 === windows.length) {
                return;
            }

            // There are modal windows present, force focus back to the top-most modal window
            windows.sort(function(a, b){ return a.z - b.z; });
            windows[windows.length - 1].focus = true;
            */

//            var focusedItem = offscreenWindow.activeFocusItem ;
//            if (DebugQML && focusedItem) {
//                var rect = desktop.mapFromItem(focusedItem, 0, 0, focusedItem.width, focusedItem.height);
//                focusDebugger.x = rect.x;
//                focusDebugger.y = rect.y;
//                focusDebugger.width = rect.width
//                focusDebugger.height = rect.height
//            }
        }

        function getRepositionChildren(predicate) {
            return d.findMatchingChildren(desktop, function(child) {
                return (child.shouldReposition === true && (!predicate || predicate(child)));
            });
        }

        function repositionAll() {
            if (desktop.repositionLocked) {
                return;
            }

            var oldRecommendedRect = recommendedRect;
            var oldRecommendedDimmensions = { x: oldRecommendedRect.width, y: oldRecommendedRect.height };
            var newRecommendedRect = Controller.getRecommendedHUDRect();
            var newRecommendedDimmensions = { x: newRecommendedRect.width, y: newRecommendedRect.height };
            var windows = d.getTopLevelWindows();
            for (var i = 0; i < windows.length; ++i) {
                var targetWindow = windows[i];
                if (targetWindow.visible) {
                    repositionWindow(targetWindow, true, oldRecommendedRect, oldRecommendedDimmensions, newRecommendedRect, newRecommendedDimmensions);
                }
            }

            // also reposition the other children that aren't top level windows but want to be repositioned
            var otherChildren = d.getRepositionChildren();
            for (var i = 0; i < otherChildren.length; ++i) {
                var child = otherChildren[i];
                repositionWindow(child, true, oldRecommendedRect, oldRecommendedDimmensions, newRecommendedRect, newRecommendedDimmensions);
            }

        }
    }

    property bool pinned: false
    property var hiddenChildren: []

    function togglePinned() {
        pinned = !pinned
    }

    function isPointOnWindow(point) {
        for (var i = 0; i < desktop.visibleChildren.length; i++) {
            var child = desktop.visibleChildren[i];
            if (child.hasOwnProperty("modality")) {
                var mappedPoint = mapToItem(child, point.x, point.y);
                if (child.hasOwnProperty("frame")) {
                    var outLine = child.frame.children[2];  // sizeOutline
                    var framePoint = mapToItem(outLine, point.x, point.y);
                    if (outLine.contains(framePoint)) {
                        return true;
                    }
                }

                if (child.contains(mappedPoint)) {
                    return true;
                }
            }
        }
        return false;
    }

    function hideDesktopWindows() {
        for (var index = 0; index < desktop.visibleChildren.length; index++) {
            var child = desktop.visibleChildren[index];
            if (child.topLevelWindow && child.hasOwnProperty("modality")) {
                var TOOLBAR_NAME = "com.highfidelity.interface.toolbar.system"
                if (child.objectName !== TOOLBAR_NAME) {
                    child.setShown(false);
                }
            }
        }
    }

    function setPinned(newPinned) {
        pinned = newPinned
    }

    property real unpinnedAlpha: 1.0;

    Behavior on unpinnedAlpha {
        NumberAnimation {
            easing.type: Easing.Linear;
            duration: 300
        }
    }

    state: "NORMAL"
    states: [
        State {
            name: "NORMAL"
            PropertyChanges { target: desktop; unpinnedAlpha: 1.0 }
        },
        State {
            name: "PINNED"
            PropertyChanges { target: desktop; unpinnedAlpha: 0.0 }
        }
    ]

    transitions: [
        Transition {
             NumberAnimation { properties: "unpinnedAlpha"; duration: 300 }
        }
    ]

    onPinnedChanged: {
        if (pinned) {
            d.raiseWindow(desktop);
            desktop.focus = true;
            desktop.forceActiveFocus();

            // recalculate our non-pinned children
            hiddenChildren = d.findMatchingChildren(desktop, function(child){
                return !d.isTopLevelWindow(child) && child.visible && !child.pinned;
            });

            hiddenChildren.forEach(function(child){
                child.opacity = Qt.binding(function(){ return desktop.unpinnedAlpha });
            });
        }
        state = pinned ? "PINNED" : "NORMAL"
    }

    onShowDesktop: pinned = false

    function raise(item) {
        var targetWindow = d.getDesktopWindow(item);
        if (!targetWindow) {
            console.warn("Could not find top level window for " + item);
            return;
        }

        // Fix up the Z-order (takes into account if this is a modal window)
        d.raiseWindow(targetWindow);
        var setFocus = true;
        if (!d.isModalWindow(targetWindow)) {
            var modalWindows = d.getTopLevelWindows(d.isModalWindow);
            if (modalWindows.length) {
                setFocus = false;
            }
        }

        if (setFocus) {
            targetWindow.focus = true;
        }

        showDesktop();
    }

    function ensureTitleBarVisible(targetWindow) {
        // Reposition window to ensure that title bar is vertically inside window.
        if (targetWindow.frame && targetWindow.frame.decoration) {
            var topMargin = -targetWindow.frame.decoration.anchors.topMargin;  // Frame's topMargin is a negative value.
            targetWindow.y = Math.max(targetWindow.y, topMargin);
        }
    }

    function centerOnVisible(item) {
        var targetWindow = d.getDesktopWindow(item);
        if (!targetWindow) {
            console.warn("Could not find top level window for " + item);
            return;
        }
/*
        if (typeof Controller === "undefined") {
            console.warn("Controller not yet available... can't center");
            return;
        }
*/

        var newRecommendedRectJS = (typeof Controller === "undefined") ? Qt.rect(0,0,0,0) : Controller.getRecommendedHUDRect();
        var newRecommendedRect = Qt.rect(newRecommendedRectJS.x, newRecommendedRectJS.y,
                                newRecommendedRectJS.width,
                                newRecommendedRectJS.height);
        var newRecommendedDimmensions = { x: newRecommendedRect.width, y: newRecommendedRect.height };
        var newX = newRecommendedRect.x + ((newRecommendedRect.width - targetWindow.width) / 2);
        var newY = newRecommendedRect.y + ((newRecommendedRect.height - targetWindow.height) / 2);
        targetWindow.x = newX;
        targetWindow.y = newY;

        ensureTitleBarVisible(targetWindow);

        // If we've noticed that our recommended desktop rect has changed, record that change here.
        if (recommendedRect != newRecommendedRect) {
            recommendedRect = newRecommendedRect;
        }
    }

    function repositionOnVisible(item) {
        var targetWindow = d.getDesktopWindow(item);
        if (!targetWindow) {
            console.warn("Could not find top level window for " + item);
            return;
        }
/*
        if (typeof Controller === "undefined") {
            console.warn("Controller not yet available... can't reposition targetWindow:" + targetWindow);
            return;
        }
*/

        var oldRecommendedRect = recommendedRect;
        var oldRecommendedDimmensions = { x: oldRecommendedRect.width, y: oldRecommendedRect.height };
        var newRecommendedRect = { width: 1280, height: 720, x: 0, y: 0 };
		if (typeof Controller !== "undefined") newRecommendedRect = Controller.getRecommendedHUDRect();
        var newRecommendedDimmensions = { x: newRecommendedRect.width, y: newRecommendedRect.height };
        repositionWindow(targetWindow, false, oldRecommendedRect, oldRecommendedDimmensions, newRecommendedRect, newRecommendedDimmensions);
    }

    function repositionWindow(targetWindow, forceReposition,
                    oldRecommendedRect, oldRecommendedDimmensions, newRecommendedRect, newRecommendedDimmensions) {

        if (desktop.width === 0 || desktop.height === 0) {
            return;
        }

        if (!targetWindow) {
            console.warn("Could not find top level window for " + item);
            return;
        }

        var recommended = { width: 1280, height: 720, x: 0, y: 0 };
   	    if (typeof Controller !== "undefined") recommended = Controller.getRecommendedHUDRect();
        var maxX = recommended.x + recommended.width;
        var maxY = recommended.y + recommended.height;
        var newPosition = Qt.vector2d(targetWindow.x, targetWindow.y);

        // if we asked to force reposition, or if the window is completely outside of the recommended rectangle, reposition it
        if (forceReposition || (targetWindow.x > maxX || (targetWindow.x + targetWindow.width) < recommended.x) ||
            (targetWindow.y > maxY || (targetWindow.y + targetWindow.height) < recommended.y))  {
            newPosition.x = -1
            newPosition.y = -1
        }

        if (newPosition.x === -1 && newPosition.y === -1) {
            var originRelativeX = (targetWindow.x - oldRecommendedRect.x);
            var originRelativeY = (targetWindow.y - oldRecommendedRect.y);
            if (isNaN(originRelativeX)) {
                originRelativeX = 0;
            }
            if (isNaN(originRelativeY)) {
                originRelativeY = 0;
            }
            var fractionX = Utils.clamp(originRelativeX / oldRecommendedDimmensions.x, 0, 1);
            var fractionY = Utils.clamp(originRelativeY / oldRecommendedDimmensions.y, 0, 1);
            var newX = (fractionX * newRecommendedDimmensions.x) + newRecommendedRect.x;
            var newY = (fractionY * newRecommendedDimmensions.y) + newRecommendedRect.y;
            newPosition = Qt.vector2d(newX, newY);
        }
        targetWindow.x = newPosition.x;
        targetWindow.y = newPosition.y;

        ensureTitleBarVisible(targetWindow);
    }

    Component { id: messageDialogBuilder; MessageDialog { } }
    function messageBox(properties) {
        return messageDialogBuilder.createObject(desktop, properties);
    }

    Component { id: inputDialogBuilder; QueryDialog { } }
    function inputDialog(properties) {
        return inputDialogBuilder.createObject(desktop, properties);
    }

    Component { id: customInputDialogBuilder; CustomQueryDialog { } }
    function customInputDialog(properties) {
        return customInputDialogBuilder.createObject(desktop, properties);
    }

    Component { id: fileDialogBuilder; FileDialog { } }
    function fileDialog(properties) {
        return fileDialogBuilder.createObject(desktop, properties);
    } 

    Component { id: assetDialogBuilder; Item {}}//AssetDialog { } }
    function assetDialog(properties) {
        return assetDialogBuilder.createObject(desktop, properties);
    }

    function unfocusWindows() {
        // First find the active focus item, and unfocus it, all the way
        // up the parent chain to the window
        var currentFocus = offscreenWindow.activeFocusItem;
        var targetWindow = d.getDesktopWindow(currentFocus);
        while (currentFocus) {
            if (currentFocus === targetWindow) {
                break;
            }
            currentFocus.focus = false;
            currentFocus = currentFocus.parent;
        }

        // Unfocus all windows
        var windows = d.getTopLevelWindows();
        for (var i = 0; i < windows.length; ++i) {
            windows[i].focus = false;
        }

        // For the desktop to have active focus
        desktop.focus = true;
        desktop.forceActiveFocus();
    }

    function openBrowserWindow(request, profile) {
        var component = Qt.createComponent("../Browser.qml");
        var newWindow = component.createObject(desktop);
        newWindow.webView.profile = profile;
        request.openIn(newWindow.webView);
    }

    FocusHack { id: focusHack; }

    Rectangle {
        id: focusDebugger;
        objectName: "focusDebugger"
        z: 9999; visible: false; color: "red"
        ColorAnimation on color { from: "#7fffff00"; to: "#7f0000ff"; duration: 1000; loops: 9999 }
    }

    Action {
        text: "Toggle Focus Debugger"
        shortcut: "Ctrl+Shift+F"
        enabled: DebugQML
        onTriggered: focusDebugger.visible = !focusDebugger.visible
    }

}
