#version 320 es

OUTPUT(0, vec4, vTexCoordLR);

void main(void) {
    const float depth = 0.0;
#ifdef VULKAN
    const vec4 UNIT_QUAD[4] = vec4[4](
        vec4(-1.0, 1.0, depth, 1.0),
        vec4(1.0, 1.0, depth, 1.0),
        vec4(-1.0, -1.0, depth, 1.0),
        vec4(1.0, -1.0, depth, 1.0)
    );
#else
    const vec4 UNIT_QUAD[4] = vec4[4](
        vec4(-1.0, -1.0, depth, 1.0),
        vec4(1.0, -1.0, depth, 1.0),
        vec4(-1.0, 1.0, depth, 1.0),
        vec4(1.0, 1.0, depth, 1.0)
    );
#endif
    vec4 pos = UNIT_QUAD[gl_VertexID];
    gl_Position = pos;
    vTexCoordLR.xy = pos.xy;
    vTexCoordLR.xy += 1.0;
    vTexCoordLR.y *= 0.5;
    vTexCoordLR.x *= 0.25;
    vTexCoordLR.zw = vTexCoordLR.xy;
    vTexCoordLR.z += 0.5;
}
