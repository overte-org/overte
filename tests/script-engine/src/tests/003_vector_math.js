//
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

print(JSON.stringify(this));
var v1 = { x: 1, y: 0, z: 3 };
var v2 = { x: 1, y: 0, z: 1 };
print(JSON.stringify(Vec3.sum(v1,v2)));
console.log("Test message");
print(JSON.stringify(TREE_SCALE));
print(JSON.stringify(Vec3.sum(v1,v2)));
print(JSON.stringify(TREE_SCALE));
