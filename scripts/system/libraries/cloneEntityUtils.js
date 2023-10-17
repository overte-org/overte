"use strict";

//  cloneEntity.js
//
//  Copyright 2017-2019 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/* global entityIsCloneable:true, cloneEntity:true, propsAreCloneDynamic:true, Script,
   propsAreCloneDynamic:true, Entities, Uuid */

Script.include("/~/system/libraries/controllerDispatcherUtils.js");

// Object assign  polyfill
if (typeof Object.assign !== 'function') {
    Object.assign = function(target, varArgs) {
        if (target === null) {
            throw new TypeError('Cannot convert undefined or null to object');
        }
        var to = Object(target);
        for (var index = 1; index < arguments.length; index++) {
            var nextSource = arguments[index];
            if (nextSource !== null) {
                for (var nextKey in nextSource) {
                    if (Object.prototype.hasOwnProperty.call(nextSource, nextKey)) {
                        to[nextKey] = nextSource[nextKey];
                    }
                }
            }
        }
        return to;
    };
}

var entityIsCloneable = function(props) {
    if (props) {
        return props.cloneable;
    }
    return false;
};

var propsAreCloneDynamic = function(props) {
    var cloneable = entityIsCloneable(props);
    if (cloneable) {
        return props.cloneDynamic;
    }
    return false;
};

var cloneEntity = function(props) {
    var entityIDToClone = props.id;
    if (entityIsCloneable(props)) {
        var cloneID = Entities.cloneEntity(entityIDToClone);
        return cloneID;
    }
    return null;
};
