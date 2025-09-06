//
//  PlotPerf.qml
//  examples/utilities/render/plotperf
//
//  Created by Sam Gateau on 3//2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or https://www.apache.org/licenses/LICENSE-2.0.html
//
import QtQuick 2.5
import QtQuick.Controls 2.3

Item {
    id: root
    width: parent.width
    height: 100

    // The title of the graph
    property string title

    // The object used as the default source object for the prop plots
    property var object

    property var backgroundOpacity: 0.6

    // Plots is an array of plot descriptor
    // a default plot descriptor expects the following object:
    //     prop: [ {
    //                object: {}             // Optional: this is the object from which the prop will be fetched, 
    //                                          if nothing than the object from root is used
    //                prop:"bufferCPUCount", // Needed the name of the property from the object to feed the plot
    //                label: "CPU",          // Optional: Label as displayed on the plot
    //                color: "#00B4EF"       // Optional: Color of the curve
    //                unit: "km/h"           // Optional: Unit added to the value displayed, if nothing then the default unit is used
    //                scale: 1               // Optional: Extra scaling used to represent the value, this scale is combined with the global scale.
    //            },
    property var plots

    // Default value scale used to define the max value of the chart
    property var valueScale: 1

    // Default value unit appended to the value displayed
    property var valueUnit: ""

    // Default number of digits displayed
    property var valueNumDigits: 0
    

    property var valueMax : 1
    property var valueMin : 0

    property var displayMinAt0 : true
    property var _displayMaxValue : 1
    property var _displayMinValue : 0

    property var _values
    property var tick : 0

    function createValues() {
        if (!_values) {
            _values = new Array();
        }
        for (var i =0; i < plots.length; i++) {

            var plot = plots[i];
            var object = plot["object"] || root.object;
            var value = plot["prop"];
            var isBinding = plot["binding"];
            if (isBinding) {
                object = root.parent;
                value = isBinding;
            }
            _values.push( {
                object: object,
                value: value,
                fromBinding: isBinding,
                valueMax: 1,
                valueMin: 0,
                numSamplesConstantMax: 0,
                numSamplesConstantMin: 0,
                valueHistory: new Array(),
                label: (plot["label"] !== undefined ? plot["label"] : ""),
                color: (plot["color"] !== undefined ? plot["color"] : "white"),
                scale: (plot["scale"] !== undefined ? plot["scale"] : 1),
                unit: (plot["unit"] !== undefined ? plot["unit"] : valueUnit)
            })
        }
        pullFreshValues();
    }

    Component.onCompleted: {
        createValues();   
    }
    function resetMax() {
        for (var i = 0; i < _values.length; i++) {
            _values[i].valueMax *= 0.25 // Fast reduce the max value  as we click                   
        }
    }
    function resetMin() {
        for (var i = 0; i < _values.length; i++) {
            _values[i].valueMin *= 0.25 // Fast reduce the min value  as we click                   
        }
    }

    function pullFreshValues() {
        // Wait until values are created to begin pulling
        if (!_values) { return; }

        var VALUE_HISTORY_SIZE = 100;
        tick++;
                
        var currentValueMax = 0
        var currentValueMin = 0
        for (var i = 0; i < _values.length; i++) {

            var currentVal = (+_values[i].object[_values[i].value]) * _values[i].scale;

            _values[i].valueHistory.push(currentVal)
            _values[i].numSamplesConstantMax++;

            if (_values[i].valueHistory.length > VALUE_HISTORY_SIZE) {
                var lostValue = _values[i].valueHistory.shift();
                if (lostValue >= _values[i].valueMax) {
                    _values[i].valueMax *= 0.99
                    _values[i].numSamplesConstantMax = 0
                }
                if (lostValue <= _values[i].valueMin) {
                    _values[i].valueMin *= 0.99
                    _values[i].numSamplesConstantMin = 0
                }
            }

            if (_values[i].valueMax < currentVal) {
                _values[i].valueMax = currentVal;
                _values[i].numSamplesConstantMax = 0 
            }                    
            if (_values[i].valueMin > currentVal) {
                _values[i].valueMin = currentVal;
                _values[i].numSamplesConstantMin = 0 
            }   

            if (_values[i].numSamplesConstantMax > VALUE_HISTORY_SIZE) {
                _values[i].numSamplesConstantMax = 0     
                _values[i].valueMax *= 0.95 // lower slowly the current max if no new above max since a while                      
            }
            if (_values[i].numSamplesConstantMin > VALUE_HISTORY_SIZE) {
                _values[i].numSamplesConstantMin = 0     
                _values[i].valueMin *= 0.95 // lower slowly the current min if no new above min since a while                      
            }

            if (currentValueMax < _values[i].valueMax) {
                currentValueMax = _values[i].valueMax
            }
            if (currentValueMin > _values[i].valueMin) {
                currentValueMin = _values[i].valueMin
            }
        }

        if ((valueMax < currentValueMax) || (tick % VALUE_HISTORY_SIZE == 0)) {
            valueMax = currentValueMax;
        }
        if ((valueMin > currentValueMin) || (tick % VALUE_HISTORY_SIZE == 0)) {
            valueMin = currentValueMin;
        }
        _displayMaxValue = valueMax;
        _displayMinValue = ( displayMinAt0 ? 0 : valueMin )

        mycanvas.requestPaint()
    }

    Timer {
        interval: 100; running: true; repeat: true
        onTriggered: pullFreshValues()
    }

    Canvas {
        id: mycanvas
        anchors.fill:parent
        
        onPaint: {
            var lineHeight = 12;

            function displayValue(val, unit) {
                 return (val / root.valueScale).toFixed(root.valueNumDigits) + " " + unit
            }

            function pixelFromVal(val, valScale) {
                return lineHeight + (height - lineHeight) * (1 - (0.99) * (val - _displayMinValue) / (_displayMaxValue - _displayMinValue));
            }
            function valueFromPixel(pixY) {
                return _displayMinValue + (((pixY - lineHeight) / (height - lineHeight) - 1) * (_displayMaxValue - _displayMinValue) / (-0.99));
            }
            function plotValueHistory(ctx, valHistory, color) {
                var widthStep= width / (valHistory.length - 1);

                ctx.beginPath();
                ctx.strokeStyle= color; // Green path
                ctx.lineWidth="2";
                ctx.moveTo(0, pixelFromVal(valHistory[0])); 
                   
                for (var i = 1; i < valHistory.length; i++) { 
                    ctx.lineTo(i * widthStep, pixelFromVal(valHistory[i])); 
                }

                ctx.stroke();
            }
            function displayValueLegend(ctx, val, num) {
                ctx.fillStyle = val.color;
                var bestValue = val.valueHistory[val.valueHistory.length -1];             
                ctx.textAlign = "right";
                ctx.fillText(displayValue(bestValue, val.unit), width, (num + 2) * lineHeight * 1);
                ctx.textAlign = "left";
                ctx.fillText(val.label, 0, (num + 2) * lineHeight * 1);
            }

            function displayTitle(ctx, text, maxVal) {
                ctx.fillStyle = "grey";
                ctx.textAlign = "right";
                ctx.fillText("max " + displayValue(_displayMaxValue, root.valueUnit), width, pixelFromVal(_displayMaxValue));
                
                ctx.fillText("min " + displayValue(_displayMinValue, root.valueUnit), width, pixelFromVal(_displayMinValue));
               
                ctx.fillStyle = "white";
                ctx.textAlign = "left";
                ctx.fillText(text, 0, lineHeight);
            }
            function displayBackground(ctx) {
                ctx.fillStyle = Qt.rgba(0, 0, 0, root.backgroundOpacity);
                ctx.fillRect(0, 0, width, height);
                
              /*  ctx.strokeStyle= "grey";
                ctx.lineWidth="2";

                ctx.beginPath();
                ctx.moveTo(0, lineHeight + 1); 
                ctx.lineTo(width, lineHeight + 1);
                ctx.moveTo(0, height); 
                ctx.lineTo(width, height); 
                ctx.stroke();*/
            }

            function displayMaxZeroMin(ctx) {
                var maxY = pixelFromVal(_displayMaxValue);
                
                ctx.strokeStyle= "LightSlateGray";
                ctx.lineWidth="1";
                ctx.beginPath();
                ctx.moveTo(0, maxY); 
                ctx.lineTo(width, maxY);
                ctx.stroke();

                if (_displayMinValue != 0) { 
                    var zeroY = pixelFromVal(0);
                    var minY = pixelFromVal(_displayMinValue);
                    ctx.beginPath();
                    ctx.moveTo(0, zeroY); 
                    ctx.lineTo(width, zeroY);
                    ctx.moveTo(0, minY); 
                    ctx.lineTo(width, minY);
                    ctx.stroke();
                } 
            }
            
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.font="12px Verdana";

            displayBackground(ctx);
                
            for (var i = 0; i < _values.length; i++) {
                plotValueHistory(ctx, _values[i].valueHistory, _values[i].color)
                displayValueLegend(ctx, _values[i], i)
            }

            displayMaxZeroMin(ctx);

            displayTitle(ctx, title, _displayMaxValue)
        }
    }

    MouseArea {
        id: hitbox
        anchors.fill: mycanvas

        onClicked: {
            resetMax();
            resetMin();
        }
    }
}
