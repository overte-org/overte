//
//  Created by HifiExperiments on 6/15/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_FadeTiming_h
#define hifi_FadeTiming_h

#include "QString"

/*@jsdoc
 * <p>Different time curves to control fading.</p>
 * <table>
 *   <thead>
 *     <tr><th>Value</th><th>Description</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>"linear"</code></td><td>A linear curve.</td></tr>
 *     <tr><td><code>"easeIn"</code></td><td>An ease in curve.</td></tr>
 *     <tr><td><code>"easeOut"</code></td><td>An ease out curve.</td></tr>
 *     <tr><td><code>"easeInOut"</code></td><td>An ease in/out curve.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {string} FadeTiming
 */

enum class FadeTiming : uint8_t {
    LINEAR = 0,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT,

    TIMING_COUNT,
};

class FadeTimingHelpers {
public:
    static QString getNameForFadeTiming(FadeTiming timing);
};

#endif // hifi_FadeTiming_h
