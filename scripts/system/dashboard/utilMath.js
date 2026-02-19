// utilMath.js
// Created by Ada <ada@thingvellir.net> on 2025-12-06
// Copyright 2025 Overte e.V.
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
// SPDX-License-Identifier: Apache-2.0
"use strict";

/** @type {number} */
const DEGREES_TO_RADIANS = Math.PI / 180;

/** @type {number} */
const RADIANS_TO_DEGREES = 180 / Math.PI;

/**
 * @param {number} from
 * @param {number} to
 * @param {number} t
 * @returns {number}
 */
function lerp(from, to, t) {
    return from + t * (to - from);
}

/**
 * @param {number} value
 * @param {number} min
 * @param {number} max
 * @returns {number}
 */
function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

/**
 * Rounds a `Number` to N fractional digits.
 * Similar to `Number.toFixed`, but without stringifying.
 * @param {number} number
 * @param {number} digits - Fractional decimal digits
 * @returns {number}
 */
function quantize(number, digits) {
    const scale = 10 ** digits;
    return Math.round(number * scale) / scale;
}

/**
 * @param {number} a
 * @param {number} b
 * @param {number} [epsilon=1e-5]
 * @returns {boolean} `true` if `a` is roughly equal to `b`,
 * within the range specified by `epsilon`, otherwise `false`.
 */
function withinEpsilon(a, b, epsilon = 1e-5) {
    return Math.abs(a - b) <= epsilon;
}

/**
 * Converts a human-readable duration into milliseconds.
 * @param {Object} duration
 * @param {number} [duration.seconds=0]
 * @param {number} [duration.minutes=0]
 * @param {number} [duration.hours=0]
 * @param {number} [duration.weeks=0]
 * @param {number} [duration.months=0]
 * @param {number} [duration.years=0]
 * @returns {number}
 */
function durationMillis({
    seconds = 0,
    minutes = 0,
    hours = 0,
    days = 0,
    weeks = 0,
    months = 0,
    years = 0,
}) {
    const MSEC_PER_SEC = 1000;
    const MSEC_PER_MIN = MSEC_PER_SEC * 60;
    const MSEC_PER_HOUR = MSEC_PER_MIN * 60;
    const MSEC_PER_DAY = MSEC_PER_HOUR * 24;
    const MSEC_PER_WEEK = MSEC_PER_DAY * 7;
    const MSEC_PER_MONTH = MSEC_PER_DAY * 31;
    const MSEC_PER_YEAR = MSEC_PER_MONTH * 12;

    let duration = 0;
    duration += seconds * MSEC_PER_SEC;
    duration += minutes * MSEC_PER_MIN;
    duration += hours * MSEC_PER_HOUR;
    duration += days * MSEC_PER_DAY;
    duration += weeks * MSEC_PER_WEEK;
    duration += months * MSEC_PER_MONTH;
    duration += years * MSEC_PER_YEAR;

    return duration;
}

/**
 * A scalar 3-vector. Used for representing positions and Euler angles.
 *
 * Compatible with the existing {@link Vec3} type.
 */
class Vector3 {
    /**
     * `Vector3(0, 0, 0)`
     * @returns {Vector3}
     */
    static get ZERO() { return new Vector3(0, 0, 0); }
    /**
     * `Vector3(0, 1, 0)`
     * @returns {Vector3}
     */
    static get UP() { return new Vector3(0, 1, 0); }
    /**
     * `Vector3(0, -1, 0)`
     * @returns {Vector3}
     */
    static get DOWN() { return new Vector3(0, -1, 0); }
    /**
     * `Vector3(1, 0, 0)`
     * @returns {Vector3}
     */
    static get RIGHT() { return new Vector3(1, 0, 0); }
    /**
     * `Vector3(-1, 0, 0)`
     * @returns {Vector3}
     */
    static get LEFT() { return new Vector3(-1, 0, 0); }
    /**
     * `Vector3(0, 0, -1)`
     * @returns {Vector3}
     */
    static get FORWARD() { return new Vector3(0, 0, -1); }
    /**
     * `Vector3(0, 0, 1)`
     * @returns {Vector3}
     */
    static get BACKWARD() { return new Vector3(0, 0, 1); }

    /** @type {number} */ x;
    /** @type {number} */ y;
    /** @type {number} */ z;

    /**
     * Alias of `x`
     * @returns {number}
     */
    get [0]() { return this.x; }
    /**
     * Alias of `y`
     * @returns {number}
     */
    get [1]() { return this.y; }
    /**
     * Alias of `z`
     * @returns {number}
     */
    get [2]() { return this.z; }

    *[Symbol.iterator]() { yield this.x; yield this.y; yield this.z; }

    /** @returns {string} */
    toString() { return `Vector3(${this.x}, ${this.y}, ${this.z})`; }

    /**
     * @overload
     * @param {number} [x=0]
     * @param {number} [y=0]
     * @param {number} [z=0]
     */
    /**
     * @overload
     * @param {Object|Vector3} vec
     * @param {number} [vec.x=0]
     * @param {number} [vec.y=0]
     * @param {number} [vec.z=0]
     */
    /**
     * @overload
     * @param {number[]} [xyz=[0, 0, 0]]
     */
    constructor(...args) {
        if (args[0] instanceof Vector3 || typeof(args[0]) === "object") {
            const { x = 0, y = 0, z = 0 } = args[0];

            this.x = x;
            this.y = y;
            this.z = z;
        } else if (args[0] instanceof Array) {
            const [ x = 0, y = 0, z = 0 ] = args[0];

            this.x = x;
            this.y = y;
            this.z = z;
        } else {
            const [ x = 0, y = 0, z = 0 ] = args;

            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    /**
     * @param {Vector3} rhs
     * @returns {Vector3} `this` + `rhs`
     */
    add(rhs) {
        return new Vector3(this.x + rhs.x, this.y + rhs.y, this.z + rhs.z);
    }

    /**
     * @param {Vector3} rhs
     * @returns {Vector3} `this` - `rhs`
     */
    subtract(rhs) {
        return new Vector3(this.x - rhs.x, this.y - rhs.y, this.z - rhs.z);
    }

    /**
     * @param {Vector3|number} rhs
     * @returns {Vector3} `this` * `rhs`
     */
    multiply(rhs) {
        if (typeof(rhs) === "number") {
            return new Vector3(this.x * rhs, this.y * rhs, this.z * rhs);
        } else if (rhs instanceof Quaternion) {
            // The convention is for Quat * Vec,
            // but permit Vec * Quat as an
            // undocumented convenience alias
            return rhs.multiply(this);
        } else {
            return new Vector3(this.x * rhs.x, this.y * rhs.y, this.z * rhs.z);
        }
    }

    /**
     * @param {Vector3|number} rhs
     * @returns {Vector3} `this` / `rhs`
     */
    divide(rhs) {
        if (typeof(rhs) === "number") {
            return new Vector3(this.x / rhs, this.y / rhs, this.z / rhs);
        } else {
            return new Vector3(this.x / rhs.x, this.y / rhs.y, this.z / rhs.z);
        }
    }

    /**
     * Only `true` if `this` and `rhs`
     * are *exactly* equal. See {@link Vector3.withinEpsilon}
     * @param {Vector3} rhs
     * @returns {boolean} `this` === `rhs`
     */
    equal(rhs) {
        return this.x === rhs.x && this.y === rhs.y && this.z === rhs.z;
    }

    /**
     * @param {Vector3} rhs
     * @param {number} [epsilon=1e-5]
     * @returns {Vector3} `this` ≈ `rhs`
     */
    withinEpsilon(rhs, epsilon = 1e-5) {
        return (
            withinEpsilon(this.x, rhs.x, epsilon) &&
            withinEpsilon(this.y, rhs.y, epsilon) &&
            withinEpsilon(this.z, rhs.z, epsilon)
        );
    }

    /**
     * @returns {Vector3} The euclidean length of `this`
     */
    length() {
        return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
    }

    /**
     * @returns {Vector3} `this` normalized to a length of 1,
     * or `Vector3(0, 0, 0)` if the total length is zero.
     */
    normalized() {
        // prevent infinities or NaNs if we try to normalize (0, 0, 0)
        const epsilon = 1e-5;
        const length = this.length();

        if (length >= epsilon) {
            return new Vector3(this.x / length, this.y / length, this.z / length);
        } else {
            return new Vector3(0, 0, 0);
        }
    }

    /**
     * @param {Vector3} rhs
     * @returns {number} Dot product of `this` and `rhs`,
     * approaching 1 if they point in a similar direction,
     * -1 if opposite, and 0 if a right angle to eachother.
     */
    dot(rhs) {
        return this.x * rhs.x + this.y * rhs.x + this.z * rhs.z;
    }

    /**
     * @param {Vector3} rhs
     * @returns {Vector3} Cross product of `this` and `rhs`
     */
    cross(rhs) {
        const { x: ax, y: ay, z: az } = this;
        const { x: bx, y: by, z: bz } = rhs;

        return new Vector3(
            (ay * bz) - (az * by),
            (az * bx) - (ax * bz),
            (ax * by) - (ay * bx),
        );
    }

    /**
     * @param {number} digits
     * @returns {Vector3}
     */
    quantize(digits) {
        return new Vector3(
            quantize(this.x, digits),
            quantize(this.y, digits),
            quantize(this.z, digits)
        );
    }

    /**
     * @returns {Vector3}
     */
    round() {
        return new Vector3(Math.round(this.x), Math.round(this.y), Math.round(this.z));
    }

    /**
     * @returns {Vector3}
     */
    abs() {
        return new Vector3(Math.abs(this.x), Math.abs(this.y), Math.abs(this.z));
    }

    /**
     * @param {Vector3} rhs
     * @returns {Vector3}
     */
    distance(rhs) {
        const { x: lx, y: ly, z: lz } = this;
        const { x: rx, y: ry, z: rz } = rhs;
        const x = (lx - rx) * (lx - rx);
        const y = (ly - ry) * (ly - ry);
        const z = (lz - rz) * (lz - rz);

        return Math.sqrt(x + y + z);
    }

    /**
     * @param {Vector3} origin
     * @returns {Vector3} `this` - `origin`
     */
    relativeTo(origin) {
        return this.subtract(origin);
    }

    /**
     * @param {Vector3} rhs
     * @param {number} t
     * @returns {Vector3}
     */
    lerpTo(rhs, t) {
        let { x: lx, y: ly, z: lz } = this;
        let { x: rx, y: ry, z: rz } = rhs;

        let x = lerp(lx, rx, t);
        let y = lerp(ly, ry, t);
        let z = lerp(lz, rz, t);

        return new Vector3(x, y, z);
    }
}

/**
 * A rotation quaternion. Quaternions represent rotations unambiguously,
 * unlike Euler angles which can suffer from gimbal lock,
 * but aren't generally human-readable.
 *
 * See {@link Quaternion.toEulerDegrees} and {@link Quaternion.fromPitchYawRollDegrees}.
 *
 * Compatible with the existing {@link Quat} type.
 *
 * {@link https://en.wikipedia.org/wiki/Quaternion}
 */
class Quaternion {
    /**
     * The identity quaternion. Represents no rotation.
     *
     * @returns {Quaternion} `Quaternion(0, 0, 0, 1)`
     */
    static get IDENTITY() { return new Quaternion(0, 0, 0, 1); }

    /** @type {number} */ x;
    /** @type {number} */ y;
    /** @type {number} */ z;
    /** @type {number} */ w;

    /**
     * Alias of `x`
     * @returns {number}
     */
    get [0]() { return this.x; }
    /**
     * Alias of `y`
     * @returns {number}
     */
    get [1]() { return this.y; }
    /**
     * Alias of `z`
     * @returns {number}
     */
    get [2]() { return this.z; }
    /**
     * Alias of `w`
     * @returns {number}
     */
    get [3]() { return this.w; }

    *[Symbol.iterator]() { yield this.x; yield this.y; yield this.z; yield this.w; }

    /** @returns {string} */
    toString() { return `Quaternion(${this.x}, ${this.y}, ${this.z}, ${this.w})`; }

    /**
     * @overload
     * @throws Throws an error if x, y, z, or w is missing
     * @param {number} x
     * @param {number} y
     * @param {number} z
     * @param {number} w
     */
    /**
     * @overload
     * @throws Throws an error if x, y, z, or w is missing
     * @param {number[]} xyzw
     */
    /**
     * @overload
     * @throws Throws an error if x, y, z, or w is missing
     * @param {Object|Quaternion} quat
     * @param {number} quat.x
     * @param {number} quat.y
     * @param {number} quat.z
     * @param {number} quat.w
     */
    constructor(...args) {
        if (args.length === 0) {
            this.x = 0;
            this.y = 0;
            this.z = 0;
            this.w = 1;
        } else if (args[0] instanceof Quaternion || typeof(args[0]) === "object") {
            const { x, y, z, w } = args[0];

            if (
                x === undefined ||
                y === undefined ||
                z === undefined ||
                w === undefined
            ) {
                throw new Error("x, y, z, and w must all be specified");
            }

            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        } else if (args[0] instanceof Array) {
            const [ x, y, z, w ] = args[0];

            if (
                x === undefined ||
                y === undefined ||
                z === undefined ||
                w === undefined
            ) {
                throw new Error("x, y, z, and w must all be specified");
            }

            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        } else {
            const [ x, y, z, w ] = args;

            if (
                x === undefined ||
                y === undefined ||
                z === undefined ||
                w === undefined
            ) {
                throw new Error("x, y, z, and w must all be specified");
            }

            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }
    }

    /**
     * @overload
     * @param {Vector3} rhs
     * @returns {Vector3} a new `Vector3` that has been rotated by `this`.
     */
    /**
     * @overload
     * @param {Quaternion} rhs
     * @returns {Quaternion}
     */
    multiply(rhs) {
        if (rhs instanceof Quaternion) {
            const { x: ax, y: ay, z: az, w: aw } = this;
            const { x: bx, y: by, z: bz, w: bw } = rhs;

            const x = (aw * bx) + (ax * bw) + (ay * bz) - (az * by);
            const y = (aw * by) + (ay * bw) + (ay * bz) - (az * by);
            const z = (aw * bz) + (az * bw) + (ax * bx) - (ay * bx);
            const w = (aw * bw) - (ax * bx) - (ay * by) - (az * bz);

            return new Quaternion(x, y, z, w);
        } else if (rhs instanceof Vector3) {
            const real = this.w;
            const imaginary = new Vector3(this.x, this.y, this.z);
            const p = imaginary.cross(rhs);
            const pp = imaginary.cross(p);

            return rhs.add(p.multiply(real).add(pp).multiply(2));
        } else {
            throw new TypeError("rhs must be Quaternion or Vector3");
        }
    }

    /**
     * @returns {Quaternion} `this` with total length normalized to 1
     */
    normalized() {
        const { x, y, z, w } = this;
        const length = Math.sqrt(x * x + y * y + z * z + w * w);
        return new Quaternion(x / length, y / length, z / length, w / length);
    }

    /** @returns {Quaternion} */
    conjugate() {
        return new Quaternion(-this.x, -this.y, -this.z, this.w);
    }

    /** @returns {Quaternion} */
    inverse() {
        return this.conjugate().normalized();
    }

    /**
     * @param {Quaternion} rhs
     * @returns {Quaternion}
     */
    difference(rhs) {
        return this.inverse().multiply(rhs);
    }

    /**
     * @param {Quaternion} rhs
     * @returns {number}
     */
    dot(rhs) {
        return this.x * rhs.x + this.y * rhs.x + this.z * rhs.z + this.w * rhs.w;
    }

    /**
     * @param {number} pitch
     * @param {number} yaw
     * @param {number} roll
     * @returns {Quaternion}
     */
    static fromPitchYawRollRadians(pitch, yaw, roll) {
        const [x, y, z] = [pitch, yaw, roll];

        const sinX = Math.sin(x * 0.5);
        const sinY = Math.sin(y * 0.5);
        const sinZ = Math.sin(z * 0.5);
        const cosX = Math.cos(x * 0.5);
        const cosY = Math.cos(y * 0.5);
        const cosZ = Math.cos(z * 0.5);

        const qW = cosX * cosY * cosZ + sinX * sinY * sinZ;
        const qX = sinX * cosY * cosZ - cosX * sinY * sinZ;
        const qY = cosX * sinY * cosZ + sinX * cosY * sinZ;
        const qZ = cosX * cosY * sinZ - sinX * sinY * cosZ;

        return new Quaternion(qX, qY, qZ, qW);
    }

    /**
     * @param {number} pitch
     * @param {number} yaw
     * @param {number} roll
     * @returns {Quaternion}
     */
    static fromPitchYawRollDegrees(pitch, yaw, roll) {
        const [x, y, z] = [pitch, yaw, roll];

        return this.fromPitchYawRollRadians(
            x * DEGREES_TO_RADIANS,
            y * DEGREES_TO_RADIANS,
            z * DEGREES_TO_RADIANS
        );
    }

    /**
     * @param {number} x
     * @param {number} y
     * @param {number} z
     * @param {number} angle
     * @returns {Quaternion}
     */
    static fromAxisAngleRadians(x, y, z, angle) {
        return new Quaternion(
            Math.sin(angle * 0.5) * x,
            Math.sin(angle * 0.5) * y,
            Math.sin(angle * 0.5) * z,
            Math.cos(angle * 0.5)
        );
    }

    /**
     * @param {number} x
     * @param {number} y
     * @param {number} z
     * @param {number} angle
     * @returns {Quaternion}
     */
    static fromAxisAngleDegrees(x, y, z, angle) {
        return this.fromAxisAngleRadians(x, y, z, angle * DEGREES_TO_RADIANS);
    }

    /**
     * @returns {Vector3}
     */
    toEulerRadians() {
        const epsilon = 1e-5;
        const { x, y, z, w } = this;

        const singularity = Math.abs(x * y + z * w) > (0.5 - epsilon);

        const pitch = singularity ? 0 : Math.atan2(
            2 * (y * z + w * x),
            w * w - x * x - y * y + z * z
        );
        const yaw = Math.asin(-2 * (x * z - w * y));
        const roll = singularity ? 0 : Math.atan2(
            2 * (x * y + w * z),
            w * w + x * x - y * y - z * z
        );

        return new Vector3(pitch, yaw, roll);
    }

    /**
     * @returns {Vector3}
     */
    toEulerDegrees() {
        let euler = this.toEulerRadians();
        euler.x *= RADIANS_TO_DEGREES;
        euler.y *= RADIANS_TO_DEGREES;
        euler.z *= RADIANS_TO_DEGREES;

        return euler;
    }

    /**
     * @param {Quaternion} rhs
     * @param {number} t
     * @returns {Quaternion} *Note: Not normalized!*
     */
    lerpTo(rhs, t) {
        let { x: lx, y: ly, z: lz, w: lw } = this;
        let { x: rx, y: ry, z: rz, w: rw } = rhs;

        let x = lerp(lx, rx, t);
        let y = lerp(ly, ry, t);
        let z = lerp(lz, rz, t);
        let w = lerp(lw, rw, t);

        return new Quaternion(x, y, z, w);
    }

    /**
     * @param {Quaternion} rhs
     * @param {number} t - [0, 1]
     * @returns {Quaternion}
     */
    slerpTo(rhs, t) {
        // http://number-none.com/product/Understanding%20Slerp%2C%20Then%20Not%20Using%20It/
        const epsilon = 1e-3;

        const dot = this.dot(rhs);

        // Already close or on the endpoint
        if (dot > (1 - epsilon)) {
            return this.lerpTo(rhs, t).normalized();
        }

        const theta = Math.acos(dot) * t;

        const basis = new Quaternion(
            rhs.x - this.x * dot,
            rhs.y - this.y * dot,
            rhs.z - this.z * dot,
            rhs.w - this.w * dot,
        ).normalized();

        const cosTheta = Math.cos(theta);
        const sinTheta = Math.sin(theta);

        return new Quaternion(
            this.x * cosTheta + basis.x * sinTheta,
            this.y * cosTheta + basis.y * sinTheta,
            this.z * cosTheta + basis.z * sinTheta,
            this.w * cosTheta + basis.w * sinTheta
        );
    }
}

/**
 * An RGBA color with scalar channels which can be sRGB or linear.
 * Supports HDR colors and negative colors.
 */
class ColorF {
    /** @returns {ColorF} `ColorF(0, 0, 0, 0) */
    static get TRANSPARENT() { return new ColorF(0, 0, 0, 0); }
    /** @returns {ColorF} `ColorF(0, 0, 0, 1) */
    static get BLACK() { return new ColorF(0, 0, 0); }
    /** @returns {ColorF} `ColorF(1, 1, 1, 1) */
    static get WHITE() { return new ColorF(1, 1, 1); }
    /** @returns {ColorF} `ColorF(1, 0, 0, 1) */
    static get RED() { return new ColorF(1, 0, 0); }
    /** @returns {ColorF} `ColorF(0, 1, 0, 1) */
    static get GREEN() { return new ColorF(0, 1, 0); }
    /** @returns {ColorF} `ColorF(0, 0, 1, 1) */
    static get BLUE() { return new ColorF(0, 0, 1); }

    /** @type {number} */ red;
    /** @type {number} */ green;
    /** @type {number} */ blue;
    /** @type {number} */ alpha;
    /** @type {("srgb"|"linear")} */ colorSpace;

    /** @returns {number} - Alias of `red` */
    get r() { return this.red; }
    /** @returns {number} - Alias of `green` */
    get g() { return this.green; }
    /** @returns {number} - Alias of `blue` */
    get b() { return this.blue; }
    /** @returns {number} - Alias of `alpha` */
    get a() { return this.alpha; }

    /** @returns {number} - Alias of `red` */
    get [0]() { return this.red; }
    /** @returns {number} - Alias of `green` */
    get [1]() { return this.green; }
    /** @returns {number} - Alias of `blue` */
    get [2]() { return this.blue; }
    /** @returns {number} - Alias of `alpha` */
    get [3]() { return this.alpha; }

    /** @returns {ColorF} - `this` with alpha set to 1 */
    get rgb() {
        let color = new ColorF(this);
        color.alpha = 1;
        return color;
    }

    /**
     * @overload
     * @param {number} red
     * @param {number} green
     * @param {number} blue
     * @param {number} [alpha=1]
     * @param {("srgb"|"linear")} [colorSpace="srgb"]
     */
    /**
     * @overload
     * @param {number[]} rgba
     * @param {number} rgba.0 - Red
     * @param {number} rgba.1 - Green
     * @param {number} rgba.2 - Blue
     * @param {number} [rgba.3=1] - Alpha
     * @param {("srgb"|"linear")} [rgba.4="srgb"] - Color space
     */
    /**
     * @overload
     * @param {Object|ColorF} color
     * @param {number} color.red
     * @param {number} color.green
     * @param {number} color.blue
     * @param {number} [color.alpha=1]
     * @param {("srgb"|"linear")} [color.colorSpace="srgb"]
     */
    constructor(...args) {
        if (args[0] instanceof ColorF || typeof(args[0]) === "object") {
            const { red, green, blue, alpha = 1, colorSpace = "srgb" } = args[0];

            this.red = red;
            this.green = green;
            this.blue = blue;
            this.alpha = alpha;
            this.colorSpace = colorSpace;
        } else if (args[0] instanceof Array) {
            const [ r, g, b, a = 1, colorSpace = "srgb" ] = args[0];

            this.red = r;
            this.green = g;
            this.blue = b;
            this.alpha = a;
            this.colorSpace = colorSpace;
        } else {
            const [ r, g, b, a = 1, colorSpace = "srgb" ] = args;

            this.red = r;
            this.green = g;
            this.blue = b;
            this.alpha = a;
            this.colorSpace = colorSpace;
        }

        if (this.colorSpace !== "srgb" && this.colorSpace !== "linear") {
            throw new Error(`colorSpace must be "srgb" or "linear", got ${JSON.stringify(this.colorSpace)}`);
        }
    }

    /**
     * @returns {string} A printable representation of this color. If sRGB and non-HDR,
     * then the format will be `"ColorF(color-hex; sRGB)"`, otherwise
     * `"ColorF(red, green, blue, alpha; color-space)"`.
     */
    toString() {
        const normalized = (
            this.red >= 0 && this.red <= 1 &&
            this.green >= 0 && this.green <= 1 &&
            this.blue >= 0 && this.blue <= 1 &&
            this.alpha >= 0 && this.alpha <= 1
        );
        const colorSpaceName = this.colorSpace === "linear" ? "Linear" : "sRGB";

        if (normalized && this.colorSpace === "srgb") {
            return `ColorF(${this.toHex()}; ${colorSpaceName})`;
        } else {
            return `ColorF(${this.red}, ${this.green}, ${this.blue}, ${this.alpha}; ${colorSpaceName})`;
        }
    }

    /**
     * @param {ColorF} rhs
     * @returns {ColorF} `this` + `rhs`
     */
    add(rhs) {
        return new ColorF(
            this.red + rhs.red,
            this.green + rhs.green,
            this.blue + rhs.blue,
            this.alpha + rhs.alpha,
            this.colorSpace
        );
    }

    /**
     * @param {ColorF} rhs
     * @returns {ColorF} `this` - `rhs`
     */
    subtract(rhs) {
        return new ColorF(
            this.red - rhs.red,
            this.green - rhs.green,
            this.blue - rhs.blue,
            this.alpha - rhs.alpha,
            this.colorSpace
        );
    }

    /**
     * @overload
     * @param {ColorF} rhs
     * @returns {ColorF} `this` * `rhs`
     */
    /**
     * @overload
     * @param {number} rhs
     * @returns {ColorF} `this` * `rhs`
     */
    multiply(rhs) {
        if (typeof(rhs) === "number") {
            return new ColorF(
                this.red * rhs,
                this.green * rhs,
                this.blue * rhs,
                this.alpha * rhs,
                this.colorSpace
            );
        } else {
            return new ColorF(
                this.red * rhs.red,
                this.green * rhs.green,
                this.blue * rhs.blue,
                this.alpha * rhs.alpha,
                this.colorSpace
            );
        }
    }

    /**
     * @overload
     * @param {ColorF} rhs
     * @returns {ColorF} `this` / `rhs`
     */
    /**
     * @overload
     * @param {number} rhs
     * @returns {ColorF} `this` / `rhs`
     */
    divide(rhs) {
        if (typeof(rhs) === "number") {
            return new ColorF(
                this.red / rhs,
                this.green / rhs,
                this.blue / rhs,
                this.alpha / rhs,
                this.colorSpace
            );
        } else {
            return new ColorF(
                this.red / rhs.red,
                this.green / rhs.green,
                this.blue / rhs.blue,
                this.alpha / rhs.alpha,
                this.colorSpace
            );
        }
    }

    /**
     * @param {ColorF} rhs
     * @param {number} t
     * @returns {ColorF}
     */
    lerpTo(rhs, t) {
        const { red: lr, green: lg, blue: lb, alpha: la } = this;
        const { red: rr, green: rg, blue: rb, alpha: ra } = rhs;

        const r = lerp(lr, rr, t);
        const g = lerp(lg, rg, t);
        const b = lerp(lb, rb, t);
        const a = lerp(la, ra, t);

        return new ColorF(r, g, b, a, this.colorSpace);
    }

    /**
     * Transforms a linear color scalar to sRGB-gamma one.
     * @param {number} x
     * @returns {number}
     */
    static scalarLinearToSrgb(x) {
        if (x <= 0.0031308) {
            return 12.92 * x;
        } else {
            return (1.055 * Math.pow(x, 1.0 / 2.4)) - 0.055;
        }
    }

    /**
     * Transforms a sRGB-gamma color scalar to a linear one.
     * @param {number} x
     * @returns {number}
     */
    static scalarSrgbToLinear(x) {
        if (x <= 0.04045) {
            return x / 12.92;
        } else {
            return Math.pow(((x + 0.055) / 1.055), 2.4);
        }
    }

    /**
     * @returns {Color8}
     */
    toColor8() {
        let { r, g, b, a } = this.toSRGB();

        return new Color8(
            Math.floor(clamp(r * 255, 0, 255)),
            Math.floor(clamp(g * 255, 0, 255)),
            Math.floor(clamp(b * 255, 0, 255)),
            Math.floor(clamp(a * 255, 0, 255))
        );
    }

    /**
     * Converts a `ColorF` with the sRGB colorspace to linear.
     * If the color is already in linear colorspace, the color is returned unmodified.
     * @returns {ColorF}
     */
    toLinear() {
        let { r, g, b, a, colorSpace } = this;

        if (colorSpace === "linear") {
            return this;
        } else {
            return new ColorF(
                ColorF.scalarSrgbToLinear(r),
                ColorF.scalarSrgbToLinear(g),
                ColorF.scalarSrgbToLinear(b),
                a,
                "srgb"
            );
        }
    }

    /**
     * Converts a `ColorF` with the linear colorspace to sRGB.
     * If the color is already in sRGB colorspace, the color is returned unmodified.
     * @returns {ColorF}
     */
    toSRGB() {
        let { r, g, b, a, colorSpace } = this;

        if (colorSpace === "srgb") {
            return this;
        } else {
            return new ColorF(
                ColorF.scalarLinearToSrgb(r),
                ColorF.scalarLinearToSrgb(g),
                ColorF.scalarLinearToSrgb(b),
                a,
                "linear"
            );
        }
    }

    /**
     * @returns {string} A CSS-compatible 6- or 8- hex digit color string prefixed with '#'.
     * Does not support HDR colors or the linear color space. HDR colors will be clamped
     * to [0, 1] and linear colors will be converted to sRGB.
     */
    toHex() {
        return this.toColor8().toHex();
    }

    /**
     * @param {number} hue - Degrees from 0° to 360°
     * @param {number} saturation - [0, 1]
     * @param {number} value - [0, 1]
     * @param {number} [alpha=1]
     * @returns {ColorF}
     */
    static hsv(hue, saturation, value, alpha = 1) {
        const c = value * saturation;
        const h = (hue / 60) % 6;
        const x = c * (1 - Math.abs((h % 2) - 1));

        let r1, g1, b1;

        if (h < 1) { r1 = c; g1 = x; b1 = 0; }
        else if (h < 2)  { r1 = x; g1 = c; b1 = 0; }
        else if (h < 3)  { r1 = 0; g1 = c; b1 = x; }
        else if (h < 4)  { r1 = 0; g1 = x; b1 = c; }
        else if (h < 5)  { r1 = x; g1 = 0; b1 = c; }
        else if (h < 6)  { r1 = c; g1 = 0; b1 = x; }

        const m = value - c;
        const r = r1 + m;
        const g = g1 + m;
        const b = b1 + m;

        return new ColorF(r, g, b, alpha, "srgb");
    }

    /**
     * @param {number} hue - Degrees from 0° to 360°
     * @param {number} saturation - [0, 1]
     * @param {number} lightness - [0, 1]
     * @param {number} [alpha=1]
     * @returns {ColorF}
     */
    static hsl(hue, saturation, lightness, alpha = 1) {
        const c = (1 - Math.abs(2 * lightness - 1)) * saturation;
        const h = (hue / 60) % 6;
        const x = c * (1 - Math.abs((h % 2) - 1));

        let r1, g1, b1;

        if (h < 1) { r1 = c; g1 = x; b1 = 0; }
        else if (h < 2)  { r1 = x; g1 = c; b1 = 0; }
        else if (h < 3)  { r1 = 0; g1 = c; b1 = x; }
        else if (h < 4)  { r1 = 0; g1 = x; b1 = c; }
        else if (h < 5)  { r1 = x; g1 = 0; b1 = c; }
        else if (h < 6)  { r1 = c; g1 = 0; b1 = x; }

        const m = lightness - (c / 2);
        const r = r1 + m;
        const g = g1 + m;
        const b = b1 + m;

        return new ColorF(r, g, b, alpha, "srgb");
    }

    /**
     * @param {number} L - [0, 1] Lightness
     * @param {number} a - [-0.5, 0.5] Green-Red
     * @param {number} b - [-0.5, 0.5] Blue-Yellow
     * @param {number} [alpha=1]
     * @returns {ColorF}
     */
    static oklab(L, a, b, alpha = 1) {
        // https://bottosson.github.io/posts/oklab/#converting-from-linear-srgb-to-oklab
        // "The code is available in public domain, feel free to use it any way you please."
        const l_ = L + 0.3963377774 * a + 0.2158037573 * b;
        const m_ = L - 0.1055613458 * a - 0.0638541728 * b;
        const s_ = L - 0.0894841775 * a - 1.2914855480 * b;

        const l = l_ * l_ * l_;
        const m = m_ * m_ * m_;
        const s = s_ * s_ * s_;

        return new ColorF(
            +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s,
            -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s,
            -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s,
            alpha,
            "linear"
        );
    }

    /**
     * @param {number} L - [0, 1] Lightness
     * @param {number} C - [0, 0.5] Chrominance
     * @param {number} h - [0, 2π] Hue
     * @param {number} [alpha=1]
     * @returns {ColorF}
     */
    static oklch(L, C, h, alpha = 1) {
        const a = C * Math.cos(h);
        const b = C * Math.sin(h);
        return this.oklab(L, a, b, alpha);
    }
}

/**
 * An RGBA color with 8-bit sRGB channels.
 * Compatible with the existing {@link Color}
 * and {@link ColorFloat} types used by the {@link Entities} API.
 */
class Color8 {
    /** @returns {Color8} `Color8(0, 0, 0, 0) */
    static get TRANSPARENT() { return new Color8(0, 0, 0, 0); }
    /** @returns {Color8} `Color8(0, 0, 0, 255) */
    static get BLACK() { return new Color8(0, 0, 0); }
    /** @returns {Color8} `Color8(255, 255, 255, 255) */
    static get WHITE() { return new Color8(255, 255, 255); }
    /** @returns {Color8} `Color8(255, 0, 0, 255) */
    static get RED() { return new Color8(255, 0, 0); }
    /** @returns {Color8} `Color8(0, 255, 0, 255) */
    static get GREEN() { return new Color8(0, 255, 0); }
    /** @returns {Color8} `Color8(0, 0, 255, 255) */
    static get BLUE() { return new Color8(0, 0, 255); }

    /** @type {number} - [0, 255] */ red;
    /** @type {number} - [0, 255] */ green;
    /** @type {number} - [0, 255] */ blue;
    /** @type {number} - [0, 255] */ alpha;
    // sRGB only, alpha is always linear

    /** @returns {number} - Alias of `red` */
    get r() { return this.red; }
    /** @returns {number} - Alias of `green` */
    get g() { return this.green; }
    /** @returns {number} - Alias of `blue` */
    get b() { return this.blue; }
    /** @returns {number} - Alias of `alpha` */
    get a() { return this.alpha; }

    /** @returns {number} - Alias of `red` */
    get [0]() { return this.red; }
    /** @returns {number} - Alias of `green` */
    get [1]() { return this.green; }
    /** @returns {number} - Alias of `blue` */
    get [2]() { return this.blue; }
    /** @returns {number} - Alias of `alpha` */
    get [3]() { return this.alpha; }

    /** @returns {Color8} - `this` with alpha set to 255 */
    get rgb() {
        let color = new Color8(this);
        color.alpha = 255;
        return color;
    }

    /** @returns {string} */
    toString() {
        return `Color8(${this.toHex()})`;
    }

    /**
     * @overload
     * @param {number} red
     * @param {number} green
     * @param {number} blue
     * @param {number} [alpha=255]
     */
    /**
     * @overload
     * @param {number[]} rgba
     * @param {number} rgba.0 - Red
     * @param {number} rgba.1 - Green
     * @param {number} rgba.2 - Blue
     * @param {number} [rgba.3=255] - Alpha
     */
    /**
     * @overload
     * @param {Object|Color8} color
     * @param {number} color.red
     * @param {number} color.green
     * @param {number} color.blue
     * @param {number} [color.alpha=255]
     */
    constructor(...args) {
        if (args[0] instanceof Color8 || typeof(args[0]) === "object") {
            const { red, green, blue, alpha = 255 } = args[0];

            this.red = red;
            this.green = green;
            this.blue = blue;
            this.alpha = alpha;
        } else if (args[0] instanceof Array) {
            const [ r, g, b, a = 255 ] = args[0];

            this.red = r;
            this.green = g;
            this.blue = b;
            this.alpha = a;
        } else {
            const [ r = 0, g = 0, b = 0, a = 255 ] = args;

            this.red = r;
            this.green = g;
            this.blue = b;
            this.alpha = a;
        }
    }

    /** @returns {ColorF} */
    toColorF() {
        let { r, g, b, a } = this;
        return new ColorF(r / 255, g / 255, b / 255, a / 255, "srgb");
    }

    /**
     * @returns {string} A CSS-compatible 6- or 8- hex digit color string prefixed with '#'.
     */
    toHex() {
        const { red, green, blue, alpha } = this;
        const rh = red.toString(16).padStart(2, "0");
        const gh = green.toString(16).padStart(2, "0");
        const bh = blue.toString(16).padStart(2, "0");
        const ah = alpha.toString(16).padStart(2, "0");

        return `#${rh}${gh}${bh}${alpha === 255 ? "" : ah}`;
    }

    /**
     * @param {Color8} rhs
     * @returns {Color8}
     */
    add(rhs) {
        return this.toColorF().add(rhs.toColorF()).toColor8();
    }

    /**
     * @param {Color8} rhs
     * @returns {Color8}
     */
    subtract(rhs) {
        return this.toColorF().subtract(rhs.toColorF()).toColor8();
    }

    /**
     * @param {number|Color8} rhs
     * @returns {Color8}
     */
    multiply(rhs) {
        if (typeof(rhs) === "number") {
            return this.toColorF().multiply(rhs).toColor8();
        } else {
            return this.toColorF().multiply(rhs.toColorF()).toColor8();
        }
    }

    /**
     * @param {number|Color8} rhs
     * @returns {Color8}
     */
    divide(rhs) {
        if (typeof(rhs) === "number") {
            return this.toColorF().divide(rhs).toColor8();
        } else {
            return this.toColorF().divide(rhs.toColorF()).toColor8();
        }
    }

    /**
     * @param {Color8} rhs
     * @param {number} t - [0, 1]
     * @returns {Color8}
     */
    lerpTo(rhs, t) {
        return this.toColorF().lerpTo(rhs.toColorF(), t).toColor8();
    }

    /**
     * @param {number} hue - Degrees from 0° to 360°
     * @param {number} saturation - [0, 1]
     * @param {number} value - [0, 1]
     * @param {number} [alpha=255]
     * @returns {Color8}
     */
    static hsv(hue, saturation, value, alpha = 255) {
        return ColorF.hsv(hue, saturation, value, alpha / 255).toColor8();
    }

    /**
     * @param {number} hue - Degrees from 0° to 360°
     * @param {number} saturation - [0, 1]
     * @param {number} lightness - [0, 1]
     * @param {number} [alpha=255]
     * @returns {Color8}
     */
    static hsl(hue, saturation, lightness, alpha = 255) {
        return ColorF.hsl(hue, saturation, lightness, alpha / 255).toColor8();
    }

    /**
     * Convenience function that wraps {@link ColorF.oklab}
     * @param {number} L - [0, 1] Lightness
     * @param {number} a - [-0.5, 0.5] Green-Red
     * @param {number} b - [-0.5, 0.5] Blue-Yellow
     * @param {number} [alpha=255]
     * @returns {Color8}
     */
    static oklab(L, a, b, alpha = 255) {
        return ColorF.oklab(L, a, b, alpha / 255).toColor8();
    }

    /**
     * Convenience function that wraps {@link ColorF.oklch}
     * @param {number} L - [0, 1] Lightness
     * @param {number} C - [0, 0.5] Chrominance
     * @param {number} h - [0, 2π] Hue
     * @param {number} [alpha=255]
     * @returns {Color8}
     */
    static oklch(L, C, h, alpha = 255) {
        return ColorF.oklch(L, C, h, alpha / 255).toColor8();
    }
}

/**
 * Convenience wrapper for `new Vector3(x, y, z)`
 * @param {number} [x=0]
 * @param {number} [y=0]
 * @param {number} [z=0]
 * @returns {Vector3}
 */
function vec3(x = 0, y = 0, z = 0) {
    return new Vector3(x, y, z);
}

/**
 * Convenience wrapper for `new Quaternion(x, y, z, w)`
 * @param {number} x
 * @param {number} y
 * @param {number} z
 * @param {number} w
 * @returns {Quaternion}
 */
function quat(x, y, z, w) {
    return new Quaternion(x, y, z, w);
}

/**
 * Convenience wrapper for `Quaternion.fromPitchYawRollDegrees(pitch, yaw, roll)`
 * @param {number} pitch
 * @param {number} yaw
 * @param {number} roll
 * @returns {Quaternion}
 */
function euler(pitch, yaw, roll) {
    return Quaternion.fromPitchYawRollDegrees(pitch, yaw, roll);
}

/**
 * Convenience wrapper for `new Color8(red, green, blue, alpha)`
 * @param {number} red
 * @param {number} green
 * @param {number} blue
 * @param {number} [alpha=255]
 * @returns {Color8}
 */
function color(red, green, blue, alpha = 255) {
    return new Color8(red, green, blue, alpha);
}

/**
 * A library of commonly used math functions and classes.
 * @module UtilMath
 */
module.exports = {
    DEGREES_TO_RADIANS,
    RADIANS_TO_DEGREES,

    lerp,
    clamp,
    quantize,
    withinEpsilon,
    durationMillis,

    Vector3,
    Quaternion,

    ColorF,
    Color8,

    vec3,
    quat,
    euler,
    color,
};
