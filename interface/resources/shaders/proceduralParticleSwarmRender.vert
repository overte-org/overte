uniform float radius = 0.01;
uniform float lifespan = 1.0; // seconds

layout(location=2) out vec3 _normalWS;

float bezierInterpolate(float y1, float y2, float y3, float u) {
    // https://en.wikipedia.org/wiki/Bezier_curve
    return (1.0 - u) * (1.0 - u) * y1 + 2.0 * (1.0 - u) * u * y2 + u * u * y3;
}

float interpolate3Points(float y1, float y2, float y3, float u) {
    // Makes the interpolated values intersect the middle value.

    if ((u <= 0.5f && y1 == y2) || (u >= 0.5f && y2 == y3)) {
        // Flat line.
        return y2;
    }

    float halfSlope;
    if ((y2 >= y1 && y2 >= y3) || (y2 <= y1 && y2 <= y3)) {
        // U or inverted-U shape.
        // Make the slope at y2 = 0, which means that the control points half way between the value points have the value y2.
        halfSlope = 0.0f;

    } else {
        // L or inverted and/or mirrored L shape.
        // Make the slope at y2 be the slope between y1 and y3, up to a maximum of double the minimum of the slopes between y1
        // and y2, and y2 and y3. Use this slope to calculate the control points half way between the value points.
        // Note: The maximum ensures that the control points and therefore the interpolated values stay between y1 and y3.
        halfSlope = (y3 - y1) / 2.0f;
        float slope12 = y2 - y1;
        float slope23 = y3 - y2;

        {
            float check = float(abs(halfSlope) > abs(slope12));
            halfSlope = mix(halfSlope, slope12, check);
            halfSlope = mix(halfSlope, slope23, (1.0 - check) * float(abs(halfSlope) > abs(slope23)));
        }
    }

    float stepU = step(0.5f, u);  // 0.0 if u < 0.5, 1.0 otherwise.
    float slopeSign = 2.0f * stepU - 1.0f; // -1.0 if u < 0.5, 1.0 otherwise
    float start = (1.0f - stepU) * y1 + stepU * y2;  // y1 if u < 0.5, y2 otherwise
    float middle = y2 + slopeSign * halfSlope;
    float finish = (1.0f - stepU) * y2 + stepU * y3; // y2 if u < 0.5, y3 otherwise
    float v = 2.0f * u - step(0.5f, u);  // 0.0-0.5 -> 0.0-1.0 and 0.5-1.0 -> 0.0-1.0
    return bezierInterpolate(start, middle, finish, v);
}

vec3 getProceduralVertex(const int particleID) {
    vec4 positionAndAge = getParticleProperty(0, particleID);
    vec3 position = positionAndAge.xyz;

    const vec3 UP = vec3(0, 1, 0);
    vec3 forward = normalize(getParticleProperty(1, particleID).xyz);
    vec3 right = cross(forward, UP);
    vec3 up = cross(right, forward);

    const int VERTEX = gl_VertexID % 3;
    int TRIANGLE = int(gl_VertexID / 3);

    float age = positionAndAge.w;
    float particleRadius = interpolate3Points(0.0, radius, 0.0, clamp(age / lifespan, 0.0, 1.0));

    if (TRIANGLE < 3) {
        const vec3 SIDE_POINTS[3] = vec3[3](
            up,
            normalize(-up + right),
            normalize(-up - right)
        );
        position += particleRadius * (VERTEX == 2 ? forward : SIDE_POINTS[(TRIANGLE + VERTEX) % 3]);
        _normalWS = normalize(cross(forward - SIDE_POINTS[TRIANGLE], forward - SIDE_POINTS[(TRIANGLE + 1) % 3]));
    } else {
        TRIANGLE -= 3;
        vec3 backward = -2.0 * normalize(getParticleProperty(2, particleID).xyz);
        const vec3 SIDE_POINTS[3] = vec3[3](
            up,
            normalize(-up - right),
            normalize(-up + right)
        );
        position += particleRadius * (VERTEX == 2 ? backward : SIDE_POINTS[(TRIANGLE + VERTEX) % 3]);
        _normalWS = normalize(cross(backward - SIDE_POINTS[TRIANGLE], backward - SIDE_POINTS[(TRIANGLE + 1) % 3]));
    }

    return position;
}
