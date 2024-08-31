vec3 getSkyboxColor() {
    vec3 normal = normalize(_normal);
    float checkSize = 0.1;
    vec3 edges = round(mod(normal, vec3(checkSize)) / checkSize);
    float checkerboard = mod(edges.x + edges.y + edges.z, 2.0);
    return mix(vec3(1, 0, 1), vec3(0.0), checkerboard);
}
