"use strict";

function functionInInclude() {
    print("Function in include");
    functionInMainFile();
}

var TestObject = function(shouldUseEditTabletApp) {
    var requireTest = Script.require("http://oaktown.pl/scripts/004b_require_module.js");

    var onEvent = function(data) {
        that.thatFunction();
    }
    var that = {};
    that.thatFunction = function() {
        functionInMainFile();
    }
    Script.update.connect(onEvent);
}

functionInMainFile();

//Script.setInterval(functionInInclude, 1000);
