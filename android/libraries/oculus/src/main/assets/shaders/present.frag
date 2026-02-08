#version 320 es

precision highp float;
precision highp sampler2D;

INPUT(0, vec4, vTexCoordLR);

OUTPUT(0, vec4, FragColorL);
OUTPUT(1, vec4, FragColorR);

uniform sampler2D sampler;

// https://software.intel.com/en-us/node/503873

// sRGB ====> Linear
vec3 color_sRGBToLinear(vec3 srgb) {
    return mix(pow((srgb + vec3(0.055)) / vec3(1.055), vec3(2.4)), srgb / vec3(12.92), vec3(lessThanEqual(srgb, vec3(0.04045))));
}

vec4 color_sRGBAToLinear(vec4 srgba) {
    return vec4(color_sRGBToLinear(srgba.xyz), srgba.w);
}

// Linear ====> sRGB
vec3 color_LinearTosRGB(vec3 lrgb) {
    return mix(vec3(1.055) * pow(vec3(lrgb), vec3(0.41666)) - vec3(0.055), vec3(lrgb) * vec3(12.92), vec3(lessThan(lrgb, vec3(0.0031308))));
}

vec4 color_LinearTosRGBA(vec4 lrgba) {
    return vec4(color_LinearTosRGB(lrgba.xyz), lrgba.w);
}

// FIXME switch to texelfetch for getting from the source texture?
void main() {
    //FragColorL = color_LinearTosRGBA(texture(sampler, vTexCoordLR.xy));
    //FragColorR = color_LinearTosRGBA(texture(sampler, vTexCoordLR.zw));
    FragColorL = texture(sampler, vTexCoordLR.xy);
    FragColorR = texture(sampler, vTexCoordLR.zw);
}
