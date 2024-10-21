//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AmbientOcclusionTechnique_h
#define hifi_AmbientOcclusionTechnique_h

#include "QString"

/*@jsdoc
 * <p>The technique used for calculating ambient occlusion.  Different techniques have different tradeoffs.</p>
 * <table>
 *   <thead>
 *     <tr><th>Value</th><th>Description</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>"ssao"</code></td><td>A basic screen-space AO effect.</td></tr>
 *     <tr><td><code>"hbao"</code></td><td>A form of SSAO that uses the depth buffer for better AO detail, sometimes at the expense of performance.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {string} AmbientOcclusionTechnique
 */

enum class AmbientOcclusionTechnique : uint8_t {
    SSAO = 0,
    HBAO,
};

class AmbientOcclusionTechniqueHelpers {
public:
    static QString getNameForAmbientOcclusionTechnique(AmbientOcclusionTechnique curve);
};

#endif // hifi_AmbientOcclusionTechnique_h
