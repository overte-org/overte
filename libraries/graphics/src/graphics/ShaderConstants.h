// <!
//  Created by Bradley Austin Davis on 2018/05/25
//  Copyright 2013-2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
// !>

// <@if not GRAPHICS_SHADER_CONSTANTS_H@>
// <@def GRAPHICS_SHADER_CONSTANTS_H@>

// Hack comment to absorb the extra '//' scribe prepends

#ifndef GRAPHICS_SHADER_CONSTANTS_H
#define GRAPHICS_SHADER_CONSTANTS_H

#define GRAPHICS_BUFFER_SKINNING 0
#define GRAPHICS_BUFFER_MATERIAL 1
#define GRAPHICS_BUFFER_KEY_LIGHT 4
#define GRAPHICS_BUFFER_LIGHT 5
#define GRAPHICS_BUFFER_AMBIENT_LIGHT 6
#define GRAPHICS_BUFFER_TRIPLANAR_SCALE 7

#define GRAPHICS_TEXTURE_MATERIAL_ALBEDO 0
#define GRAPHICS_TEXTURE_MATERIAL_NORMAL 1
#define GRAPHICS_TEXTURE_MATERIAL_METALLIC 2
#define GRAPHICS_TEXTURE_MATERIAL_EMISSIVE_LIGHTMAP 3
#define GRAPHICS_TEXTURE_MATERIAL_ROUGHNESS 4
#define GRAPHICS_TEXTURE_MATERIAL_OCCLUSION 5
#define GRAPHICS_TEXTURE_MATERIAL_SCATTERING 6
#define GRAPHICS_TEXTURE_MATERIAL_MIRROR 1 // Mirrors use albedo textures, but nothing else

// Keep aligned with procedural/ProceduralMaterialCache.h
#define GRAPHICS_TEXTURE_MATERIAL_SHADE GRAPHICS_TEXTURE_MATERIAL_METALLIC
#define GRAPHICS_TEXTURE_MATERIAL_SHADING_SHIFT GRAPHICS_TEXTURE_MATERIAL_ROUGHNESS
#define GRAPHICS_TEXTURE_MATERIAL_MATCAP GRAPHICS_TEXTURE_MATERIAL_OCCLUSION
#define GRAPHICS_TEXTURE_MATERIAL_RIM GRAPHICS_TEXTURE_MATERIAL_SCATTERING
#define GRAPHICS_TEXTURE_MATERIAL_UV_ANIMATION_MASK 7

// Make sure these match the ones in render-utils/ShaderConstants.h
#define GRAPHICS_TEXTURE_SKYBOX 11
#define GRAPHICS_BUFFER_SKYBOX_PARAMS 5

#define GRAPHICS_BUFFER_HAZE_PARAMS 7

// <!

namespace graphics { namespace slot {

namespace buffer {
enum Buffer {
    Skinning = GRAPHICS_BUFFER_SKINNING,
    Material = GRAPHICS_BUFFER_MATERIAL,
    Light = GRAPHICS_BUFFER_LIGHT,
    KeyLight = GRAPHICS_BUFFER_KEY_LIGHT,
    AmbientLight = GRAPHICS_BUFFER_AMBIENT_LIGHT,
    SkyboxParams = GRAPHICS_BUFFER_SKYBOX_PARAMS,
    HazeParams = GRAPHICS_BUFFER_HAZE_PARAMS,
    TriplanarScale = GRAPHICS_BUFFER_TRIPLANAR_SCALE
};
} // namespace buffer

namespace texture {
enum Texture {
    MaterialAlbedo = GRAPHICS_TEXTURE_MATERIAL_ALBEDO,
    MaterialNormal = GRAPHICS_TEXTURE_MATERIAL_NORMAL,
    MaterialMetallic = GRAPHICS_TEXTURE_MATERIAL_METALLIC,
    MaterialEmissiveLightmap = GRAPHICS_TEXTURE_MATERIAL_EMISSIVE_LIGHTMAP, 
    MaterialRoughness = GRAPHICS_TEXTURE_MATERIAL_ROUGHNESS,
    MaterialOcclusion = GRAPHICS_TEXTURE_MATERIAL_OCCLUSION,
    MaterialScattering = GRAPHICS_TEXTURE_MATERIAL_SCATTERING,
    MaterialMirror = GRAPHICS_TEXTURE_MATERIAL_MIRROR,
    Skybox = GRAPHICS_TEXTURE_SKYBOX,

    MaterialShade = GRAPHICS_TEXTURE_MATERIAL_SHADE,
    MaterialShadingShift = GRAPHICS_TEXTURE_MATERIAL_SHADING_SHIFT,
    MaterialMatcap = GRAPHICS_TEXTURE_MATERIAL_MATCAP,
    MaterialRim = GRAPHICS_TEXTURE_MATERIAL_RIM,
    MaterialUVAnimationMask = GRAPHICS_TEXTURE_MATERIAL_UV_ANIMATION_MASK,
};
} // namespace texture

} } // namespace graphics::slot

// !>
// Hack Comment

#endif // GRAPHICS_SHADER_CONSTANTS_H

// <@if 1@>
// Trigger Scribe include
// <@endif@> <!def that !>

// <@endif@>

// Hack Comment
