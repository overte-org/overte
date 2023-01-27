"use strict";

Script.include("./005b_included.js");

//var requireTest = Script.require("http://oaktown.pl/scripts/004b_require_module.js");

var testObject = new TestObject();

function functionInMainFile () {
    print("In main file");
}

functionInInclude();