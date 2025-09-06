//
//  lightClustering.qml
//
//  Created by Sam Gateau on 9/9/2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or https://www.apache.org/licenses/LICENSE-2.0.html
//
import QtQuick 2.5
import QtQuick.Controls 2.3
import "configSlider"
import "../lib/plotperf"

Column {
    spacing: 8
    Column {
        id: lightClustering
        spacing: 10

        Column{
            PlotPerf {
                title: "Light CLustering Timing"
                height: 50
                object: Render.getConfig("RenderMainView.LightClustering")
                valueUnit: "ms"
                valueScale: 1
                valueNumDigits: "4"
                plots: [
                    {
                       object: Render.getConfig("RenderMainView.LightClustering"),
                       prop: "cpuRunTime",
                       label: "time",
                       scale: 1,
                       color: "#FFFFFF"
                   }
                ]
            }

            PlotPerf {
                title: "Lights"
                height: 50
                object: Render.getConfig("RenderMainView.LightClustering")
                valueUnit: ""
                valueScale: 1
                valueNumDigits: "0"
                plots: [
                    {
                       object: Render.getConfig("RenderMainView.LightClustering"),
                       prop: "numClusteredLights",
                       label: "visible",
                       color: "#D959FE"
                   },
                   {
                        object: Render.getConfig("RenderMainView.LightClustering"),
                        prop: "numInputLights",
                        label: "input",
                        color: "#FED959"
                    }
                ]
            }

             PlotPerf {
                title: "Scene Lights"
                height: 80
                object: Render.getConfig("RenderMainView.LightClustering")
                valueUnit: ""
                valueScale: 1
                valueNumDigits: "0"
                plots: [
                    {
                       object: Render.getConfig("RenderMainView.LightClustering"),
                       prop: "numSceneLights",
                       label: "current",
                       color: "#00B4EF"
                   },
                   {
                        object: Render.getConfig("RenderMainView.LightClustering"),
                        prop: "numFreeSceneLights",
                        label: "free",
                        color: "#1AC567"
                    },
                   {
                        object: Render.getConfig("RenderMainView.LightClustering"),
                        prop: "numAllocatedSceneLights",
                        label: "allocated",
                        color: "#9495FF"
                    }
                ]
            }

            ConfigSlider {
                label: qsTr("Range Near [m]")
                integral: false
                config: Render.getConfig("RenderMainView.LightClustering")
                property: "rangeNear"
                max: 20.0
                min: 0.1
            }
            ConfigSlider {
                label: qsTr("Range Far [m]")
                integral: false
                config: Render.getConfig("RenderMainView.LightClustering")
                property: "rangeFar"
                max: 500.0
                min: 100.0
            }
            ConfigSlider {
                label: qsTr("Grid X")
                integral: true
                config: Render.getConfig("RenderMainView.LightClustering")
                property: "dimX"
                max: 32
                min: 1
            }
            ConfigSlider {
                label: qsTr("Grid Y")
                integral: true
                config: Render.getConfig("RenderMainView.LightClustering")
                property: "dimY"
                max: 32
                min: 1
            }
            ConfigSlider {
                label: qsTr("Grid Z")
                integral: true
                config: Render.getConfig("RenderMainView.LightClustering")
                property: "dimZ"
                max: 31
                min: 1
            }
            CheckBox {
                    text: "Freeze"
                    checked: Render.getConfig("RenderMainView.LightClustering")["freeze"]
                    onCheckedChanged: { Render.getConfig("RenderMainView.LightClustering")["freeze"] = checked }
            }
            CheckBox {
                    text: "Draw Grid"
                    checked: Render.getConfig("RenderMainView.DebugLightClusters")["doDrawGrid"]
                    onCheckedChanged: { Render.getConfig("RenderMainView.DebugLightClusters")["doDrawGrid"] = checked }
            }
            CheckBox {
                    text: "Draw Cluster From Depth"
                    checked: Render.getConfig("RenderMainView.DebugLightClusters")["doDrawClusterFromDepth"]
                    onCheckedChanged: { Render.getConfig("RenderMainView.DebugLightClusters")["doDrawClusterFromDepth"] = checked }
            }
            CheckBox {
                    text: "Draw Content"
                    checked: Render.getConfig("RenderMainView.DebugLightClusters")["doDrawContent"]
                    onCheckedChanged: { Render.getConfig("RenderMainView.DebugLightClusters")["doDrawContent"] = checked }
            }
            Label {
                text:  "Num Cluster Items = " + Render.getConfig("RenderMainView.LightClustering")["numClusteredLightReferences"].toFixed(0)
            }
            
        }
    }
}
