vec3 getErrorColor() {
    vec3 positionWS = iWorldOrientation * (_positionMS.xyz * iWorldScale) + iWorldPosition;
    float checkSize = 0.1;
    vec3 edges = round(mod(positionWS, vec3(checkSize)) / checkSize);
    float checkerboard = mod(edges.x + edges.y + edges.z, 2.0);
    return mix(vec3(1, 0, 1), vec3(0.0), checkerboard);
}

// version 1
vec3 getProceduralColor() {
    return getErrorColor();
}

// version 2
float getProceduralColors(inout vec3 diffuse, inout vec3 specular, inout float shininess) {
    diffuse = getErrorColor();
    return 1.0;
}

// version 3
float getProceduralFragment(inout ProceduralFragment data) {
    data.emissive = getErrorColor();
    return 1.0;
}

// version 4
float getProceduralFragmentWithPosition(inout ProceduralFragmentWithPosition data) {
    data.emissive = getErrorColor();
    return 1.0;
}
