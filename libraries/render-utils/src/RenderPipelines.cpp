
//
//  RenderPipelines.cpp
//  render-utils/src/
//
//  Created by Zach Pomerantz on 1/28/2016.
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderPipelines.h"

#include <material-networking/TextureCache.h>
#include <render/DrawTask.h>
#include <graphics/ShaderConstants.h>
#include <procedural/ReferenceMaterial.h>

#include "render-utils/ShaderConstants.h"
#include "DeferredLightingEffect.h"

using namespace render;
using namespace gpu;

namespace gr {
    using graphics::slot::texture::Texture;
    using graphics::slot::buffer::Buffer;
}

bool RenderPipelines::bindMaterial(graphics::MaterialPointer& material, Batch& batch, Args::RenderMode renderMode, bool enableTextures) {
    graphics::MultiMaterial multiMaterial;
    multiMaterial.push(graphics::MaterialLayer(material, 0));
    return bindMaterials(multiMaterial, batch, renderMode, enableTextures);
}

void RenderPipelines::updateMultiMaterial(graphics::MultiMaterial& multiMaterial) {
    auto& drawMaterialTextures = multiMaterial.getTextureTable();
    multiMaterial.setTexturesLoading(false);
    multiMaterial.resetReferenceTexturesAndMaterials();
    multiMaterial.setisMToon(!multiMaterial.empty() && multiMaterial.top().material && multiMaterial.top().material->isMToon());
    multiMaterial.resetOutline();
    multiMaterial.resetSamplers();

    // The total list of things we need to look for
    static std::set<uint> allFlags;
    static std::once_flag once;
    std::call_once(once, [] {
        for (int i = 0; i < graphics::Material::NUM_TOTAL_FLAGS; i++) {
            // The opacity mask/map are derived from the albedo map
            // FIXME: OPACITY_MAP_MODE_BIT is supposed to support fallthrough
            if (i != graphics::MaterialKey::OPACITY_MASK_MAP_BIT &&
                    i != graphics::MaterialKey::OPACITY_TRANSLUCENT_MAP_BIT &&
                    i != graphics::MaterialKey::OPACITY_MAP_MODE_BIT) {
                allFlags.insert(i);
            }
        }
    });

    graphics::MultiMaterial materials = multiMaterial;
    graphics::MultiMaterial::Schema schema;
    graphics::MultiMaterial::MToonSchema toonSchema;
    graphics::MaterialKey schemaKey;

    std::set<uint> flagsToCheck = allFlags;
    std::set<uint> flagsToSetDefault;

    while (!materials.empty()) {
        auto material = materials.top().material;
        if (!material) {
            break;
        }
        materials.pop();

        if (material->isReference()) {
            multiMaterial.addReferenceMaterial(std::static_pointer_cast<ReferenceMaterial>(material)->getReferenceOperator());
        }

        bool defaultFallthrough = material->getDefaultFallthrough();
        const auto materialKey = material->getKey();
        const auto textureMaps = material->getTextureMaps();

        auto it = flagsToCheck.begin();
        while (it != flagsToCheck.end()) {
            auto flag = *it;
            bool fallthrough = defaultFallthrough || material->getPropertyFallthrough(flag);

            bool wasSet = false;
            bool forceDefault = false;
            if (!multiMaterial.isMToon()) {
                switch (flag) {
                    case graphics::MaterialKey::EMISSIVE_VAL_BIT:
                        if (materialKey.isEmissive()) {
                            schema._emissive = material->getEmissive(false);
                            schemaKey.setEmissive(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::UNLIT_VAL_BIT:
                        if (materialKey.isUnlit()) {
                            schemaKey.setUnlit(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_VAL_BIT:
                        if (materialKey.isAlbedo()) {
                            schema._albedo = material->getAlbedo(false);
                            schemaKey.setAlbedo(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::METALLIC_VAL_BIT:
                        if (materialKey.isMetallic()) {
                            schema._metallic = material->getMetallic();
                            schemaKey.setMetallic(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::GLOSSY_VAL_BIT:
                        if (materialKey.isRough() || materialKey.isGlossy()) {
                            schema._roughness = material->getRoughness();
                            schemaKey.setGlossy(materialKey.isGlossy());
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_VAL_BIT:
                        if (materialKey.isTranslucentFactor()) {
                            schema._opacity = material->getOpacity();
                            schemaKey.setTranslucentFactor(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_CUTOFF_VAL_BIT:
                        if (materialKey.isOpacityCutoff()) {
                            schema._opacityCutoff = material->getOpacityCutoff();
                            schemaKey.setOpacityCutoff(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::SCATTERING_VAL_BIT:
                        if (materialKey.isScattering()) {
                            schema._scattering = material->getScattering();
                            schemaKey.setScattering(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_MAP_BIT:
                        if (materialKey.isAlbedoMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ALBEDO_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    material->resetOpacityMap();
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::ALBEDO_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setAlbedoMap(true);
                            schemaKey.setOpacityMaskMap(material->getKey().isOpacityMaskMap());
                            schemaKey.setTranslucentMap(material->getKey().isTranslucentMap());
                        }
                        break;
                    case graphics::MaterialKey::METALLIC_MAP_BIT:
                        if (materialKey.isMetallicMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::METALLIC_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialMetallic, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::METALLIC_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setMetallicMap(true);
                        }
                        break;
                    case graphics::MaterialKey::ROUGHNESS_MAP_BIT:
                        if (materialKey.isRoughnessMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ROUGHNESS_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialRoughness, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::ROUGHNESS_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setRoughnessMap(true);
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (materialKey.isNormalMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::NORMAL_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::NORMAL_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setNormalMap(true);
                        }
                        break;
                    case graphics::MaterialKey::OCCLUSION_MAP_BIT:
                        if (materialKey.isOcclusionMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::OCCLUSION_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialOcclusion, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::OCCLUSION_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setOcclusionMap(true);
                        }
                        break;
                    case graphics::MaterialKey::SCATTERING_MAP_BIT:
                        if (materialKey.isScatteringMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::SCATTERING_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialScattering, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::SCATTERING_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setScatteringMap(true);
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        // Lightmap takes precendence over emissive map for legacy reasons
                        if (materialKey.isEmissiveMap() && !materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::EMISSIVE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::EMISSIVE_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setEmissiveMap(true);
                        } else if (materialKey.isLightMap()) {
                            // We'll set this later when we check the lightmap
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::LIGHT_MAP_BIT:
                        if (materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::LIGHT_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::LIGHT_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setLightMap(true);
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM0:
                        if (!fallthrough) {
                            schema._texcoordTransforms[0] = material->getTexCoordTransform(0);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM1:
                        if (!fallthrough) {
                            schema._texcoordTransforms[1] = material->getTexCoordTransform(1);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::LIGHTMAP_PARAMS:
                        if (!fallthrough) {
                            schema._lightmapParams = material->getLightmapParams();
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::MATERIAL_PARAMS:
                        if (!fallthrough) {
                            schema._materialParams = material->getMaterialParams();
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::CULL_FACE_MODE:
                        if (!fallthrough) {
                            multiMaterial.setCullFaceMode(material->getCullFaceMode());
                            wasSet = true;
                        }
                        break;
                    default:
                        break;
                }
            } else {
                switch (flag) {
                    case graphics::MaterialKey::EMISSIVE_VAL_BIT:
                        if (materialKey.isEmissive()) {
                            toonSchema._emissive = material->getEmissive(false);
                            schemaKey.setEmissive(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_VAL_BIT:
                        if (materialKey.isAlbedo()) {
                            toonSchema._albedo = material->getAlbedo(false);
                            schemaKey.setAlbedo(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_VAL_BIT:
                        if (materialKey.isTranslucentFactor()) {
                            toonSchema._opacity = material->getOpacity();
                            schemaKey.setTranslucentFactor(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_CUTOFF_VAL_BIT:
                        if (materialKey.isOpacityCutoff()) {
                            toonSchema._opacityCutoff = material->getOpacityCutoff();
                            schemaKey.setOpacityCutoff(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_MAP_BIT:
                        if (materialKey.isAlbedoMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ALBEDO_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    material->resetOpacityMap();
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::ALBEDO_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setAlbedoMap(true);
                            schemaKey.setOpacityMaskMap(material->getKey().isOpacityMaskMap());
                            schemaKey.setTranslucentMap(material->getKey().isTranslucentMap());
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (materialKey.isNormalMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::NORMAL_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::NORMAL_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setNormalMap(true);
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        // Lightmap takes precendence over emissive map for legacy reasons
                        if (materialKey.isEmissiveMap() && !materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::EMISSIVE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::EMISSIVE_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setEmissiveMap(true);
                        } else if (materialKey.isLightMap()) {
                            // We'll set this later when we check the lightmap
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM0:
                        if (!fallthrough) {
                            toonSchema._texcoordTransforms[0] = material->getTexCoordTransform(0);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM1:
                        if (!fallthrough) {
                            toonSchema._texcoordTransforms[1] = material->getTexCoordTransform(1);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::MATERIAL_PARAMS:
                        if (!fallthrough) {
                            toonSchema._materialParams = material->getMaterialParams();
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::CULL_FACE_MODE:
                        if (!fallthrough) {
                            multiMaterial.setCullFaceMode(material->getCullFaceMode());
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT]) {
                            toonSchema._shade = material->getShade();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT]) {
                            toonSchema._shadingShift = material->getShadingShift();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT]) {
                            toonSchema._shadingToony = material->getShadingToony();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT]) {
                            toonSchema._uvAnimationScrollSpeed.x = material->getUVAnimationScrollXSpeed();
                            toonSchema._uvAnimationScrollSpeed.y = material->getUVAnimationScrollYSpeed();
                            toonSchema._uvAnimationScrollRotationSpeed = material->getUVAnimationRotationSpeed();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT :
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::SHADE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialShade, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::SHADE_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::SHADING_SHIFT_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialShadingShift, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::SHADING_SHIFT_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::MATCAP_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialMatcap, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::MATCAP_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::RIM_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialRim, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::RIM_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::UV_ANIMATION_MASK_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialUVAnimationMask, itr->second->getTextureView());
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::UV_ANIMATION_MASK_MAP); });
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT]) {
                            toonSchema._matcap = material->getMatcap(false);
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT]) {
                            toonSchema._parametricRim = material->getParametricRim(false);
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT]) {
                            toonSchema._parametricRimFresnelPower = material->getParametricRimFresnelPower();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT]) {
                            toonSchema._parametricRimLift = material->getParametricRimLift();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT]) {
                            toonSchema._rimLightingMix = material->getRimLightingMix();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::OUTLINE_WIDTH_MODE_VAL_BIT:
                        if (!fallthrough) {
                            multiMaterial.setOutlineWidthMode(material->getOutlineWidthMode());
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::OUTLINE_WIDTH_VAL_BIT:
                        if (!fallthrough) {
                            multiMaterial.setOutlineWidth(material->getOutlineWidth());
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::OUTLINE_VAL_BIT:
                        if (!fallthrough) {
                            multiMaterial.setOutline(material->getOutline(false));
                            wasSet = true;
                        }
                        break;
                    default:
                        break;
                }
            }

            if (wasSet) {
                flagsToCheck.erase(it++);
            } else if (forceDefault || !fallthrough) {
                flagsToSetDefault.insert(flag);
                flagsToCheck.erase(it++);
            } else {
                ++it;
            }
        }

        if (flagsToCheck.empty()) {
            break;
        }
    }

    for (auto flagBit : flagsToCheck) {
        flagsToSetDefault.insert(flagBit);
    }

    auto textureCache = DependencyManager::get<TextureCache>();
    // Handle defaults
    for (auto flag : flagsToSetDefault) {
        if (!multiMaterial.isMToon()) {
            switch (flag) {
                case graphics::Material::CULL_FACE_MODE:
                    multiMaterial.setCullFaceMode(graphics::Material::DEFAULT_CULL_FACE_MODE);
                    break;
                case graphics::MaterialKey::ALBEDO_MAP_BIT:
                    if (schemaKey.isAlbedoMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::METALLIC_MAP_BIT:
                    if (schemaKey.isMetallicMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialMetallic, textureCache->getBlackTexture());
                    }
                    break;
                case graphics::MaterialKey::ROUGHNESS_MAP_BIT:
                    if (schemaKey.isRoughnessMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialRoughness, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::NORMAL_MAP_BIT:
                    if (schemaKey.isNormalMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
                    }
                    break;
                case graphics::MaterialKey::OCCLUSION_MAP_BIT:
                    if (schemaKey.isOcclusionMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialOcclusion, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::SCATTERING_MAP_BIT:
                    if (schemaKey.isScatteringMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialScattering, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                    if (schemaKey.isEmissiveMap() && !schemaKey.isLightMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                    }
                    break;
                case graphics::MaterialKey::LIGHT_MAP_BIT:
                    if (schemaKey.isLightMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getBlackTexture());
                    }
                    break;
                default:
                    // everything else is initialized to the correct default values in Schema()
                    break;
            }
        } else {
            switch (flag) {
                case graphics::Material::CULL_FACE_MODE:
                    multiMaterial.setCullFaceMode(graphics::Material::DEFAULT_CULL_FACE_MODE);
                    break;
                case graphics::MaterialKey::ALBEDO_MAP_BIT:
                    if (schemaKey.isAlbedoMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::NORMAL_MAP_BIT:
                    if (schemaKey.isNormalMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
                    }
                    break;
                case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                    if (schemaKey.isEmissiveMap() && !schemaKey.isLightMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialShade, textureCache->getWhiteTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialShadingShift, textureCache->getBlackTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialMatcap, textureCache->getBlackTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialRim, textureCache->getWhiteTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialUVAnimationMask, textureCache->getWhiteTexture());
                    }
                    break;
                default:
                    // everything else is initialized to the correct default values in ToonSchema()
                    break;
            }
        }
    }

    auto& schemaBuffer = multiMaterial.getSchemaBuffer();
    if (multiMaterial.isMToon()) {
        toonSchema._key = (uint32_t)schemaKey._flags.to_ulong();
        schemaBuffer.edit<graphics::MultiMaterial::MToonSchema>() = toonSchema;
    } else {
        schema._key = (uint32_t)schemaKey._flags.to_ulong();
        schemaBuffer.edit<graphics::MultiMaterial::Schema>() = schema;
    }
    multiMaterial.setNeedsUpdate(false);
    multiMaterial.setInitialized();
}

bool RenderPipelines::bindMaterials(graphics::MultiMaterial& multiMaterial, Batch& batch, Args::RenderMode renderMode, bool enableTextures) {
    if (multiMaterial.shouldUpdate()) {
        updateMultiMaterial(multiMaterial);
    }

    if (multiMaterial.isMToon()) {
        multiMaterial.setMToonTime();
    }

    multiMaterial.applySamplers();

    auto textureCache = DependencyManager::get<TextureCache>();

    static TextureTablePointer defaultMaterialTextures = std::make_shared<TextureTable>();
    static BufferView defaultMaterialSchema;
    static TextureTablePointer defaultMToonMaterialTextures = std::make_shared<TextureTable>();
    static BufferView defaultMToonMaterialSchema;
    static BufferView defaultTriplanarScale;

    static std::once_flag once;
    std::call_once(once, [textureCache] {
        graphics::MultiMaterial::Schema schema;
        defaultMaterialSchema = BufferView(std::make_shared<Buffer>(sizeof(schema), (const Byte*) &schema, sizeof(schema)));

        defaultMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialMetallic, textureCache->getBlackTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialRoughness, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialOcclusion, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialScattering, textureCache->getWhiteTexture());
        // MaterialEmissiveLightmap has to be set later

        graphics::MultiMaterial::MToonSchema toonSchema;
        defaultMToonMaterialSchema = BufferView(std::make_shared<Buffer>(sizeof(toonSchema), (const Byte*) &toonSchema, sizeof(toonSchema)));

        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialShade, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialShadingShift, textureCache->getBlackTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialMatcap, textureCache->getBlackTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialRim, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialUVAnimationMask, textureCache->getWhiteTexture());

        vec4 triplanarScale = vec4(1.0f);
        defaultTriplanarScale = BufferView(std::make_shared<Buffer>(sizeof(triplanarScale), (const Byte*) &triplanarScale, sizeof(triplanarScale)));
    });

    if (multiMaterial.size() > 0 &&
        (MaterialMappingMode)multiMaterial.top().material->getMaterialParams().x == MaterialMappingMode::TRIPLANAR) {
        batch.setUniformBuffer(gr::Buffer::TriplanarScale, defaultTriplanarScale);
    }

    // For shadows, we only need opacity mask information
    auto key = multiMaterial.getMaterialKey();
    if (renderMode != Args::RenderMode::SHADOW_RENDER_MODE || (key.isOpacityMaskMap() || key.isTranslucentMap())) {
        auto& schemaBuffer = multiMaterial.getSchemaBuffer();
        batch.setUniformBuffer(gr::Buffer::Material, schemaBuffer);
        if (enableTextures) {
            batch.setResourceTextureTable(multiMaterial.getTextureTable());
        } else {
            if (!multiMaterial.isMToon()) {
                if (renderMode != Args::RenderMode::SHADOW_RENDER_MODE) {
                    if (key.isLightMap()) {
                        defaultMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getBlackTexture());
                    } else if (key.isEmissiveMap()) {
                        defaultMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                    }
                }

                batch.setResourceTextureTable(defaultMaterialTextures);
            } else {
                batch.setResourceTextureTable(defaultMToonMaterialTextures);
            }
        }
        return true;
    } else {
        batch.setResourceTextureTable(!multiMaterial.isMToon() ? defaultMaterialTextures : defaultMToonMaterialTextures);
        batch.setUniformBuffer(gr::Buffer::Material, !multiMaterial.isMToon() ? defaultMaterialSchema : defaultMToonMaterialSchema);
        return false;
    }
}
