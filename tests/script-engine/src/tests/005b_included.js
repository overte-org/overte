"use strict";

function functionInInclude() {
    print("Function in include");
    functionInMainFile();
}

functionInMainFile();