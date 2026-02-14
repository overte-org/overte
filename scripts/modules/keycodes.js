// keycodes.js
// Created by Ada <ada@thingvellir.net> 2026-02-14
// SPDX-License-Identifier: Apache-2.0
"use strict";

// QT6TODO: swap the "also see" link to the Qt6 docs

/**
 * @module KeyCodes
 * Helper module for common keycodes reported by {@link KeyEvent.key}.
 * Dead keys, media keys, shortcut keys, and function keys above F12 are omitted.
 *
 * Also see {@link https://doc.qt.io/qt-5/qt.html#Key-enum}, which this is translated from,
 * and has the constants for keycodes not listed in this module.
 */
module.exports = {
    space:      0x20,
    exclam:     0x21,
    quoteDbl:   0x22,
    numberSign: 0x23,
    dollar:     0x24,
    percent:    0x25,
    ampersand:  0x26,
    apostrophe: 0x27,
    parenLeft:  0x28,
    parenRight: 0x29,
    asterisk:   0x2a,
    plus:       0x2b,
    comma:      0x2c,
    minus:      0x2d,
    period:     0x2e,
    slash:      0x2f,

    zero:       0x30,
    one:        0x31,
    two:        0x32,
    three:      0x33,
    four:       0x34,
    five:       0x35,
    six:        0x36,
    seven:      0x37,
    eight:      0x38,
    nine:       0x39,

    colon:      0x3a,
    semicolon:  0x3b,
    less:       0x3c,
    equal:      0x3d,
    greater:    0x3e,
    question:   0x3f,
    at:         0x40,

    // convenience ASCII aliases
    " ":        0x20,
    "!":        0x21,
    '"':        0x22,
    "#":        0x23,
    "$":        0x24,
    "%":        0x25,
    "&":        0x26,
    "'":        0x27,
    "(":        0x28,
    ")":        0x29,
    "*":        0x2a,
    "+":        0x2b,
    ",":        0x2c,
    "-":        0x2d,
    ".":        0x2e,
    "/":        0x2f,

    "0":        0x30,
    "1":        0x31,
    "2":        0x32,
    "3":        0x33,
    "4":        0x34,
    "5":        0x35,
    "6":        0x36,
    "7":        0x37,
    "8":        0x38,
    "9":        0x39,

    ":":        0x3a,
    ";":        0x3b,
    "<":        0x3c,
    "=":        0x3d,
    ">":        0x3e,
    "?":        0x3f,
    "@":        0x40,

    a:          0x41,
    b:          0x42,
    c:          0x43,
    d:          0x44,
    e:          0x45,
    f:          0x46,
    g:          0x47,
    h:          0x48,
    i:          0x49,
    j:          0x4a,
    k:          0x4b,
    l:          0x4c,
    m:          0x4d,
    n:          0x4e,
    o:          0x4f,
    p:          0x50,
    q:          0x51,
    r:          0x52,
    s:          0x53,
    t:          0x54,
    u:          0x55,
    v:          0x56,
    w:          0x57,
    x:          0x58,
    y:          0x59,
    z:          0x5a,

    bracketLeft:  0x5b,
    backslash:    0x5c,
    bracketRight: 0x5d,
    asciiCircum:  0x5e,
    underscore:   0x5f,
    quoteLeft:    0x60,
    // 0x61-0x7a are unused, in ASCII they're the uppercase letters
    braceLeft:    0x7b,
    bar:          0x7c,
    braceRight:   0x7d,
    asciiTilde:   0x7e,

    "[":          0x5b,
    "\\":         0x5c,
    "]":          0x5d,
    "^":          0x5e,
    "_":          0x5f,
    "`":          0x60,
    "{":          0x7b,
    "|":          0x7c,
    "}":          0x7d,
    "~":          0x7e,

    escape:     0x01000000,
    tab:        0x01000001,
    backTab:    0x01000002,
    backspace:  0x01000003,
    /** The 'return' or 'enter' key on the main part of the keyboard. */
    return:     0x01000004,
    /** The 'enter' key on the numpad. */
    enter:      0x01000005,
    insert:     0x01000006,
    delete:     0x01000007,
    /** Pause/Break key */
    pause:      0x01000008,
    print:      0x01000009,
    sysReq:     0x0100000a,
    clear:      0x0100000b,
    // 0x0100000c-0x0100000f are unused
    home:       0x01000010,
    end:        0x01000011,
    /** Left arrow key */
    left:       0x01000012,
    /** Up arrow key */
    up:         0x01000013,
    /** Right arrow key */
    right:      0x01000014,
    /** Down arrow key */
    down:       0x01000015,
    pageUp:     0x01000016,
    pageDown:   0x01000017,
    /** May be either the left or right shift key. */
    shift:      0x01000020,
    /** Translated to the Command key on macOS. May be either the left or right control key. */
    control:    0x01000021,
    /** Also known as the Windows key. Translated to Control on macOS. May be either the left or right meta key. */
    meta:       0x01000022,
    /** May be either the left or right alt key. */
    alt:        0x01000023,
    capsLock:   0x01000024,
    numLock:    0x01000025,
    scrollLock: 0x01000026,

    f1:         0x01000030,
    f2:         0x01000031,
    f3:         0x01000032,
    f4:         0x01000033,
    f5:         0x01000034,
    f6:         0x01000035,
    f7:         0x01000036,
    f8:         0x01000037,
    f9:         0x01000038,
    f10:        0x01000039,
    f11:        0x0100003a,
    f12:        0x0100003b,
};
