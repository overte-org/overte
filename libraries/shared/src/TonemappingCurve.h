//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_TonemappingCurve_h
#define hifi_TonemappingCurve_h

#include "QString"

/*@jsdoc
 * <p>The tonemapping curve applied to the final rendering.</p>
 * <table>
 *   <thead>
 *     <tr><th>Value</th><th>Description</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>"rgb"</code></td><td>No tonemapping, colors are kept in RGB.</td></tr>
 *     <tr><td><code>"srgb"</code></td><td>Colors are converted to sRGB.</td></tr>
 *     <tr><td><code>"reinhard"</code></td><td>Reinhard tonemapping is applied.</td></tr>
 *     <tr><td><code>"filmic"</code></td><td>Filmic tonemapping is applied.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {string} TonemappingCurve
 */

enum class TonemappingCurve {
    RGB = 0,
    SRGB,
    REINHARD,
    FILMIC
};

class TonemappingCurveHelpers {
public:
    static QString getNameForTonemappingCurve(TonemappingCurve curve);
};

#endif // hifi_TonemappingCurve_h
