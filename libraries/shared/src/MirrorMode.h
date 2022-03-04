//
//  Created by HifiExperiments on 3/14/22.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_MirrorMode_h
#define hifi_MirrorMode_h

#include "QString"

/*@jsdoc
 * <p>If an entity is rendered as a mirror, a portal, or normally.</p>
 * <table>
 *   <thead>
 *     <tr><th>Value</th><th>Description</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>"none"</code></td><td>The entity will render normally.</td></tr>
 *     <tr><td><code>"mirror"</code></td><td>The entity will render as a mirror.</td></tr>
 *     <tr><td><code>"portal"</code></td><td>The entity will render as a portal.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {string} MirrorMode
 */

enum class MirrorMode {
    NONE = 0,
    MIRROR,
    PORTAL
};

class MirrorModeHelpers {
public:
    static QString getNameForMirrorMode(MirrorMode mode);
};

#endif // hifi_MirrorMode_h
