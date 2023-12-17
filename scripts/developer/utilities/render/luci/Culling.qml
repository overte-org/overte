//
//  culling.qml
//  examples/utilities/render
//
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or https://www.apache.org/licenses/LICENSE-2.0.html
//
import QtQuick 2.5
import QtQuick.Controls 1.4

import "../../lib/prop" as Prop

Column {
    id: root
    spacing: 8

    anchors.left: parent.left;
    anchors.right: parent.right;

    property var sceneOctree: Render.getConfig("RenderMainView.DebugRenderDeferredTask.DrawSceneOctree");
    property var itemSelection: Render.getConfig("RenderMainView.DebugRenderDeferredTask.DrawItemSelection");

     Component.onCompleted: {
        sceneOctree.enabled = true;
        itemSelection.enabled = true;
        sceneOctree.showVisibleCells = false;
        sceneOctree.showEmptyCells = false;
        itemSelection.showInsideItems = false;
        itemSelection.showInsideSubcellItems = false;
        itemSelection.showPartialItems = false;
        itemSelection.showPartialSubcellItems = false;
    }
    Component.onDestruction: {
        sceneOctree.enabled = false;
        itemSelection.enabled = false;
        Render.getConfig("RenderMainView.DebugRenderDeferredTask.FetchSceneSelection").freezeFrustum = false;
        Render.getConfig("RenderMainView.DebugRenderDeferredTask.CullSceneSelection").freezeFrustum = false;
    }

    GroupBox {
        title: "Culling"
        
        anchors.left: parent.left;
        anchors.right: parent.right;

        Row {
            spacing: 8
            Column {
                spacing: 8

                CheckBox {
                    text: "Freeze Culling Frustum"
                    checked: false
                    onCheckedChanged: {
                        Render.getConfig("RenderMainView.DebugRenderDeferredTask.FetchSceneSelection").freezeFrustum = checked;
                        Render.getConfig("RenderMainView.DebugRenderDeferredTask.CullSceneSelection").freezeFrustum = checked;
                    }
                }
                Label {
                    text: "Octree"
                }
                CheckBox {
                    text: "Visible Cells"
                    checked: root.sceneOctree.showVisibleCells
                    onCheckedChanged: { root.sceneOctree.showVisibleCells = checked }
                }
                CheckBox {
                    text: "Empty Cells"
                    checked: false
                    onCheckedChanged: { root.sceneOctree.showEmptyCells = checked }
                }
            }
            Column {
                spacing: 8

                Label {
                    text: "Frustum Items"
                }
                CheckBox {
                    text: "Inside Items"
                    checked: false
                    onCheckedChanged: { root.itemSelection.showInsideItems = checked }
                }
                CheckBox {
                    text: "Inside Sub-cell Items"
                    checked: false
                    onCheckedChanged: { root.itemSelection.showInsideSubcellItems = checked }
                }
                CheckBox {
                    text: "Partial Items"
                    checked: false
                    onCheckedChanged: { root.itemSelection.showPartialItems = checked }
                }
                CheckBox {
                    text: "Partial Sub-cell Items"
                    checked: false
                    onCheckedChanged: { root.itemSelection.showPartialSubcellItems = checked }
                }
            }
        }
      
    }

    GroupBox {
        title: "Render Items"
        anchors.left: parent.left;
        anchors.right: parent.right;

        Column{
            anchors.left: parent.left;
            anchors.right: parent.right;
            Repeater {
                model: [ "Opaque:RenderMainView.DebugRenderDeferredTask.DrawOpaqueDeferred", "Transparent:RenderMainView.DebugRenderDeferredTask.DrawTransparentDeferred", "Light:RenderMainView.DebugRenderDeferredTask.DrawLight",
                        "Opaque InFront:RenderMainView.DebugRenderDeferredTask.DrawInFrontOpaque", "Transparent InFront:RenderMainView.DebugRenderDeferredTask.DrawInFrontTransparent",
                        "Opaque HUD:RenderMainView.DebugRenderDeferredTask.DrawHUDOpaque", "Transparent HUD:RenderMainView.DebugRenderDeferredTask.DrawHUDTransparent" ]
                Prop.PropScalar {
                    label: qsTr(modelData.split(":")[0])
                    integral: true
                    object: Render.getConfig(modelData.split(":")[1])
                    property: "maxDrawn"
                    max: object.numDrawn
                    min: -1
                }
            }
        }
    }
}
