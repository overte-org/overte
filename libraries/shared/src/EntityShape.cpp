//
//  Created by HifiExperiments on 8/4/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "EntityShape.h"

const char* shapeNames[] = {
    "Triangle",
    "Quad",
    "Hexagon",
    "Octagon",
    "Circle",
    "Cube",
    "Sphere",
    "Tetrahedron",
    "Octahedron",
    "Dodecahedron",
    "Icosahedron",
    "Torus",  // Not implemented yet.
    "Cone",
    "Cylinder"
};

static const size_t ENTITY_SHAPE_NAMES = (sizeof(shapeNames) / sizeof(shapeNames[0]));

QString EntityShapeHelpers::getNameForEntityShape(EntityShape shape) {
    if (((int)shape <= 0) || ((int)shape >= (int)ENTITY_SHAPE_NAMES)) {
        shape = (EntityShape)0;
    }

    return shapeNames[(int)shape];
}
