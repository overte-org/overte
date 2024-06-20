layout(location=2) in vec3 _normalWS;

float getProceduralFragment(inout ProceduralFragment proceduralData) {
    proceduralData.normal = normalize(_normalWS);
    proceduralData.diffuse = 0.5 * (proceduralData.normal + 1.0);
    return 0.0;
}
