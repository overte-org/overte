//
//  Created by HifiExperiments on 8/4/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_EntityShape_h
#define hifi_EntityShape_h

#include <QString>

/*@jsdoc
 * <p>A <code>"Shape"</code>, <code>"Box"</code>, or <code>"Sphere"</code> {@link Entities.EntityType|EntityType} may
 * display as one of the following geometrical shapes:</p>
 * <table>
 *   <thead>
 *     <tr><th>Value</th><th>Dimensions</th><th>Notes</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>"Circle"</code></td><td>2D</td><td>A circle oriented in 3D.</td></tr>
 *     <tr><td><code>"Cone"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Cube"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Cylinder"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Dodecahedron"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Hexagon"</code></td><td>3D</td><td>A hexagonal prism.</td></tr>
 *     <tr><td><code>"Icosahedron"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Octagon"</code></td><td>3D</td><td>An octagonal prism.</td></tr>
 *     <tr><td><code>"Octahedron"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Quad"</code></td><td>2D</td><td>A square oriented in 3D.</td></tr>
 *     <tr><td><code>"Sphere"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Tetrahedron"</code></td><td>3D</td><td></td></tr>
 *     <tr><td><code>"Torus"</code></td><td>3D</td><td><em>Not implemented.</em></td></tr>
 *     <tr><td><code>"Triangle"</code></td><td>3D</td><td>A triangular prism.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {string} Entities.Shape
 */
enum class EntityShape {
    Triangle,
    Quad,
    Hexagon,
    Octagon,
    Circle,
    Cube,
    Sphere,
    Tetrahedron,
    Octahedron,
    Dodecahedron,
    Icosahedron,
    Torus,
    Cone,
    Cylinder,
    NUM_SHAPES,
};

class EntityShapeHelpers {
public:
    static QString getNameForEntityShape(EntityShape shape);
};

#endif // hifi_EntityShape_h
