//
//  Created by HifiExperiments on 8/17/24
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_TextVerticalAlignment_h
#define hifi_TextVerticalAlignment_h

#include "QString"

/*@jsdoc
 * <p>A {@link Entities.EntityProperties-Text|Text} entity may use one of the following vertical alignments:</p>
 * <table>
 *   <thead>
 *     <tr><th>Value</th><th>Description</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>"top"</code></td><td>Text is aligned to the top.</td></tr>
 *     <tr><td><code>"bottom"</code></td><td>Text is aligned to the bottom.</td></tr>
 *     <tr><td><code>"center"</code></td><td>Text is centered vertically.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {string} Entities.TextVerticalAlignment
 */

enum class TextVerticalAlignment : uint8_t {
    TOP = 0,
    BOTTOM,
    CENTER,
};

class TextVerticalAlignmentHelpers {
public:
    static QString getNameForTextVerticalAlignment(TextVerticalAlignment alignment);
};

#endif // hifi_TextVerticalAlignment_h