//  Created by Anthony J. Thibault on 9/7/2016
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// Stick a EventBridge object in the global namespace.
var EventBridge;
(function () {
    // the TempEventBridge class queues up emitWebEvent messages and executes them when the real EventBridge is ready.
    // Similarly, it holds all scriptEventReceived callbacks, and hooks them up to the real EventBridge.
    function TempEventBridge() {
        var self = this;
        this._callbacks = [];
        this._messages = [];
        this.scriptEventReceived = {
            connect: function (callback) {
                self._callbacks.push(callback);
            }
        };
        this.emitWebEvent = function (message) {
            self._messages.push(message);
        };
    }

    EventBridge = new TempEventBridge();

    var webChannel = new QWebChannel(qt.webChannelTransport, (channel) => {
        //
        // replace the TempEventBridge with the real one.
        var tempEventBridge = EventBridge;
        EventBridge = channel.objects.eventBridge;

        // TODO: These event listeners cause issues. Most likely the function is not being exposed to this scope. This may be causing issues!
        
        // To be able to update the state of the output device selection for every element added to the DOM
        // we need to listen to events that might precede the addition of this elements.
        // A more robust hack will be to add a setInterval that look for DOM changes every 100-300 ms (low performance?)

        // window.addEventListener("load",function(event) {
        //     setTimeout(function() { 
        //         EventBridge.forceHtmlAudioOutputDeviceUpdate();
        //     }, 1200);
        // }, false);

        // document.addEventListener("click",function(){
        //     setTimeout(function() { 
        //         EventBridge.forceHtmlAudioOutputDeviceUpdate();
        //     }, 1200);
        // }, false);

        // document.addEventListener("change",function(){
        //     setTimeout(function() { 
        //         EventBridge.forceHtmlAudioOutputDeviceUpdate();
        //     }, 1200);
        // }, false);

        tempEventBridge._callbacks.forEach(function (callback) {
            EventBridge.scriptEventReceived.connect(callback);
        });
        tempEventBridge._messages.forEach(function (message) {
            EventBridge.emitWebEvent(message);
        });
    });
})();
