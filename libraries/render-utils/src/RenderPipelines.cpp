
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
    auto& drawMaterialTextures = multiMaterial.getTextureTables();
    multiMaterial.setTexturesLoading(false);
    multiMaterial.resetReferenceTexturesAndMaterials();
    if (!multiMaterial.empty() && multiMaterial.top().material) {
        multiMaterial.setisMToonAndLayers(multiMaterial.top().material->isMToon(),
            std::max(1, std::min((int)multiMaterial.size(), (int)multiMaterial.top().material->getLayers())));
    } else {    
        multiMaterial.setisMToonAndLayers(false, 1);
    }
    multiMaterial.resetOutline();
    multiMaterial.resetSamplers();

    multiMaterial.setSplatMap(nullptr);

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
    std::array<graphics::MultiMaterial::Schema, 3> schemas;
    std::array<graphics::MultiMaterial::MToonSchema, 3> toonSchemas;
    std::array<graphics::MaterialKey, 3> schemaKeys;

    std::array<std::set<uint>, 3> flagsToCheck;
    flagsToCheck.fill(allFlags);
    // We only look for splat maps on the first material
    for (int i = 1; i < 3; i++) {
        flagsToCheck[i].erase(graphics::MaterialKey::SPLAT_MAP_BIT);
    }
    std::array<std::set<uint>, 3> flagsToSetDefault;

    uint8_t layerIndex = 0;
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

        auto it = flagsToCheck[layerIndex].begin();
        while (it != flagsToCheck[layerIndex].end()) {
            auto flag = *it;
            bool fallthrough = defaultFallthrough || material->getPropertyFallthrough(flag);

            bool wasSet = false;
            bool forceDefault = false;
            if (!multiMaterial.isMToon()) {
                switch (flag) {
                    case graphics::MaterialKey::EMISSIVE_VAL_BIT:
                        if (materialKey.isEmissive()) {
                            schemas[layerIndex]._emissive = material->getEmissive(false);
                            schemaKeys[layerIndex].setEmissive(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::UNLIT_VAL_BIT:
                        if (materialKey.isUnlit()) {
                            schemaKeys[layerIndex].setUnlit(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_VAL_BIT:
                        if (materialKey.isAlbedo()) {
                            schemas[layerIndex]._albedo = material->getAlbedo(false);
                            schemaKeys[layerIndex].setAlbedo(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::METALLIC_VAL_BIT:
                        if (materialKey.isMetallic()) {
                            schemas[layerIndex]._metallic = material->getMetallic();
                            schemaKeys[layerIndex].setMetallic(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::GLOSSY_VAL_BIT:
                        if (materialKey.isRough() || materialKey.isGlossy()) {
                            schemas[layerIndex]._roughness = material->getRoughness();
                            schemaKeys[layerIndex].setGlossy(materialKey.isGlossy());
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_VAL_BIT:
                        if (materialKey.isTranslucentFactor()) {
                            schemas[layerIndex]._opacity = material->getOpacity();
                            schemaKeys[layerIndex].setTranslucentFactor(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_CUTOFF_VAL_BIT:
                        if (materialKey.isOpacityCutoff()) {
                            schemas[layerIndex]._opacityCutoff = material->getOpacityCutoff();
                            schemaKeys[layerIndex].setOpacityCutoff(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::SCATTERING_VAL_BIT:
                        if (materialKey.isScattering()) {
                            schemas[layerIndex]._scattering = material->getScattering();
                            schemaKeys[layerIndex].setScattering(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_MAP_BIT:
                        if (materialKey.isAlbedoMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ALBEDO_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    material->resetOpacityMap();
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialAlbedo, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::ALBEDO_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setAlbedoMap(true);
                            schemaKeys[layerIndex].setOpacityMaskMap(material->getKey().isOpacityMaskMap());
                            schemaKeys[layerIndex].setTranslucentMap(material->getKey().isTranslucentMap());
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialAlbedo, material->getTexCoordSet(graphics::MaterialKey::ALBEDO_MAP));
                        }
                        break;
                    case graphics::MaterialKey::METALLIC_MAP_BIT:
                        if (materialKey.isMetallicMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::METALLIC_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialMetallic, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::METALLIC_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setMetallicMap(true);
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialMetallic, material->getTexCoordSet(graphics::MaterialKey::METALLIC_MAP));
                        }
                        break;
                    case graphics::MaterialKey::ROUGHNESS_MAP_BIT:
                        if (materialKey.isRoughnessMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ROUGHNESS_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialRoughness, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::ROUGHNESS_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setRoughnessMap(true);
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialRoughness, material->getTexCoordSet(graphics::MaterialKey::ROUGHNESS_MAP));
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (materialKey.isNormalMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::NORMAL_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialNormal, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::NORMAL_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setNormalMap(true);
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialNormal, material->getTexCoordSet(graphics::MaterialKey::NORMAL_MAP));
                        }
                        break;
                    case graphics::MaterialKey::OCCLUSION_MAP_BIT:
                        if (materialKey.isOcclusionMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::OCCLUSION_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialOcclusion, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::OCCLUSION_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setOcclusionMap(true);
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialOcclusion, material->getTexCoordSet(graphics::MaterialKey::OCCLUSION_MAP));
                        }
                        break;
                    case graphics::MaterialKey::SCATTERING_MAP_BIT:
                        if (materialKey.isScatteringMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::SCATTERING_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialScattering, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::SCATTERING_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setScatteringMap(true);
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialScattering, material->getTexCoordSet(graphics::MaterialKey::SCATTERING_MAP));
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        // Lightmap takes precendence over emissive map for legacy reasons
                        if (materialKey.isEmissiveMap() && !materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::EMISSIVE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialEmissiveLightmap, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::EMISSIVE_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setEmissiveMap(true);
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialEmissiveLightmap, material->getTexCoordSet(graphics::MaterialKey::EMISSIVE_MAP));
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
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialEmissiveLightmap, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::LIGHT_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setLightMap(true);
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialEmissiveLightmap, material->getTexCoordSet(graphics::MaterialKey::LIGHT_MAP));
                        }
                        break;
                    case graphics::MaterialKey::SPLAT_MAP_BIT: {
                        auto itr = textureMaps.find(graphics::MaterialKey::SPLAT_MAP);
                        if (itr != textureMaps.end()) {
                            if (itr->second->isDefined()) {
                                auto textureView = itr->second->getTextureView();
                                Q_ASSERT(textureView);
                                multiMaterial.setSplatMap(textureView._texture);
                                multiMaterial.addSamplerFunc([=]() { material->applySampler(graphics::MaterialKey::SPLAT_MAP); });
                                if (textureView.isReference()) {
                                    multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                }
                                wasSet = true;
                            } else {
                                multiMaterial.setTexturesLoading(true);
                                forceDefault = true;
                            }
                            schemas[layerIndex].setTexCoordSet(gr::Texture::MaterialSplat, material->getTexCoordSet(graphics::MaterialKey::SPLAT_MAP));
                        } else {
                            forceDefault = true;
                        }
                        break;
                    }
                    case graphics::Material::TEXCOORDTRANSFORM0:
                        if (!fallthrough) {
                            schemas[layerIndex]._texcoordTransforms[0] = material->getTexCoordTransform(0);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM1:
                        if (!fallthrough) {
                            schemas[layerIndex]._texcoordTransforms[1] = material->getTexCoordTransform(1);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::LIGHTMAP_PARAMS:
                        if (!fallthrough) {
                            schemas[layerIndex]._lightmapParams = material->getLightmapParams();
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::MATERIAL_PARAMS:
                        if (!fallthrough) {
                            schemas[layerIndex]._materialParams = material->getMaterialParams();
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
                            toonSchemas[layerIndex]._emissive = material->getEmissive(false);
                            schemaKeys[layerIndex].setEmissive(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_VAL_BIT:
                        if (materialKey.isAlbedo()) {
                            toonSchemas[layerIndex]._albedo = material->getAlbedo(false);
                            schemaKeys[layerIndex].setAlbedo(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_VAL_BIT:
                        if (materialKey.isTranslucentFactor()) {
                            toonSchemas[layerIndex]._opacity = material->getOpacity();
                            schemaKeys[layerIndex].setTranslucentFactor(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_CUTOFF_VAL_BIT:
                        if (materialKey.isOpacityCutoff()) {
                            toonSchemas[layerIndex]._opacityCutoff = material->getOpacityCutoff();
                            schemaKeys[layerIndex].setOpacityCutoff(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_MAP_BIT:
                        if (materialKey.isAlbedoMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ALBEDO_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    material->resetOpacityMap();
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialAlbedo, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::ALBEDO_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setAlbedoMap(true);
                            schemaKeys[layerIndex].setOpacityMaskMap(material->getKey().isOpacityMaskMap());
                            schemaKeys[layerIndex].setTranslucentMap(material->getKey().isTranslucentMap());
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialAlbedo, material->getTexCoordSet(graphics::MaterialKey::ALBEDO_MAP));
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (materialKey.isNormalMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::NORMAL_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialNormal, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::NORMAL_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setNormalMap(true);
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialNormal, material->getTexCoordSet(graphics::MaterialKey::NORMAL_MAP));
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        // Lightmap takes precendence over emissive map for legacy reasons
                        if (materialKey.isEmissiveMap() && !materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::EMISSIVE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialEmissiveLightmap, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler(graphics::MaterialKey::EMISSIVE_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex].setEmissiveMap(true);
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialEmissiveLightmap, material->getTexCoordSet(graphics::MaterialKey::EMISSIVE_MAP));
                        } else if (materialKey.isLightMap()) {
                            // We'll set this later when we check the lightmap
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::SPLAT_MAP_BIT: {
                        auto itr = textureMaps.find(graphics::MaterialKey::SPLAT_MAP);
                        if (itr != textureMaps.end()) {
                            if (itr->second->isDefined()) {
                                auto textureView = itr->second->getTextureView();
                                Q_ASSERT(textureView);
                                multiMaterial.setSplatMap(textureView._texture);
                                multiMaterial.addSamplerFunc([=]() { material->applySampler(graphics::MaterialKey::SPLAT_MAP); });
                                if (textureView.isReference()) {
                                    multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                }
                                wasSet = true;
                            } else {
                                multiMaterial.setTexturesLoading(true);
                                forceDefault = true;
                            }
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialSplat, material->getTexCoordSet(graphics::MaterialKey::SPLAT_MAP));
                        } else {
                            forceDefault = true;
                        }
                        break;
                    }
                    case graphics::Material::TEXCOORDTRANSFORM0:
                        if (!fallthrough) {
                            toonSchemas[layerIndex]._texcoordTransforms[0] = material->getTexCoordTransform(0);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM1:
                        if (!fallthrough) {
                            toonSchemas[layerIndex]._texcoordTransforms[1] = material->getTexCoordTransform(1);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::MATERIAL_PARAMS:
                        if (!fallthrough) {
                            toonSchemas[layerIndex]._materialParams = material->getMaterialParams();
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
                            toonSchemas[layerIndex]._shade = material->getShade();
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT]) {
                            toonSchemas[layerIndex]._shadingShift = material->getShadingShift();
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT]) {
                            toonSchemas[layerIndex]._shadingToony = material->getShadingToony();
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT]) {
                            toonSchemas[layerIndex]._uvAnimationScrollSpeed.x = material->getUVAnimationScrollXSpeed();
                            toonSchemas[layerIndex]._uvAnimationScrollSpeed.y = material->getUVAnimationScrollYSpeed();
                            toonSchemas[layerIndex]._uvAnimationScrollRotationSpeed = material->getUVAnimationRotationSpeed();
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT :
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::SHADE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialShade, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::SHADE_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT, true);
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialShade, material->getTexCoordSet((graphics::Material::MapChannel) NetworkMToonMaterial::SHADE_MAP));
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::SHADING_SHIFT_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialShadingShift, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::SHADING_SHIFT_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT, true);
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialShadingShift, material->getTexCoordSet((graphics::Material::MapChannel) NetworkMToonMaterial::SHADING_SHIFT_MAP));
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::MATCAP_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialMatcap, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::MATCAP_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT, true);
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialMatcap, material->getTexCoordSet((graphics::Material::MapChannel) NetworkMToonMaterial::MATCAP_MAP));
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::RIM_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialRim, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::RIM_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT, true);
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialRim, material->getTexCoordSet((graphics::Material::MapChannel) NetworkMToonMaterial::RIM_MAP));
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::UV_ANIMATION_MASK_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    auto textureView = itr->second->getTextureView();
                                    Q_ASSERT(textureView);
                                    drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialUVAnimationMask, textureView);
                                    multiMaterial.addSamplerFunc([=] () { material->applySampler((graphics::Material::MapChannel) NetworkMToonMaterial::UV_ANIMATION_MASK_MAP); });
                                    if (textureView.isReference()) {
                                        multiMaterial.addReferenceTexture(textureView.getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT, true);
                            toonSchemas[layerIndex].setTexCoordSet(gr::Texture::MaterialUVAnimationMask, material->getTexCoordSet((graphics::Material::MapChannel) NetworkMToonMaterial::UV_ANIMATION_MASK_MAP));
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT]) {
                            toonSchemas[layerIndex]._matcap = material->getMatcap(false);
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT]) {
                            toonSchemas[layerIndex]._parametricRim = material->getParametricRim(false);
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT]) {
                            toonSchemas[layerIndex]._parametricRimFresnelPower = material->getParametricRimFresnelPower();
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT]) {
                            toonSchemas[layerIndex]._parametricRimLift = material->getParametricRimLift();
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT]) {
                            toonSchemas[layerIndex]._rimLightingMix = material->getRimLightingMix();
                            schemaKeys[layerIndex]._flags.set(NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT, true);
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
                flagsToCheck[layerIndex].erase(it++);
            } else if (forceDefault || !fallthrough) {
                flagsToSetDefault[layerIndex].insert(flag);
                flagsToCheck[layerIndex].erase(it++);
            } else {
                ++it;
            }
        }

        uint8_t numLayers = multiMaterial.getLayers();
        if (numLayers > 1) {
            // For layered materials, we stop when we reach the desired number of layers
            if (layerIndex == numLayers - 1) {
                break;
            }
            layerIndex++;
        } else if (flagsToCheck[layerIndex].empty()) {
            // For non-layered materials, we can stop once we've checked all the flags
            break;
        }
    }

    for (uint8_t layerIndex = 0; layerIndex < multiMaterial.getLayers(); layerIndex++) {
        for (auto flagBit : flagsToCheck[layerIndex]) {
            flagsToSetDefault[layerIndex].insert(flagBit);
        }

        auto textureCache = DependencyManager::get<TextureCache>();
        // Handle defaults
        for (auto flag : flagsToSetDefault[layerIndex]) {
            if (!multiMaterial.isMToon()) {
                switch (flag) {
                    case graphics::Material::CULL_FACE_MODE:
                        multiMaterial.setCullFaceMode(graphics::Material::DEFAULT_CULL_FACE_MODE);
                        break;
                    case graphics::MaterialKey::ALBEDO_MAP_BIT:
                        if (schemaKeys[layerIndex].isAlbedoMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
                        }
                        break;
                    case graphics::MaterialKey::METALLIC_MAP_BIT:
                        if (schemaKeys[layerIndex].isMetallicMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialMetallic, textureCache->getBlackTexture());
                        }
                        break;
                    case graphics::MaterialKey::ROUGHNESS_MAP_BIT:
                        if (schemaKeys[layerIndex].isRoughnessMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialRoughness, textureCache->getWhiteTexture());
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (schemaKeys[layerIndex].isNormalMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
                        }
                        break;
                    case graphics::MaterialKey::OCCLUSION_MAP_BIT:
                        if (schemaKeys[layerIndex].isOcclusionMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialOcclusion, textureCache->getWhiteTexture());
                        }
                        break;
                    case graphics::MaterialKey::SCATTERING_MAP_BIT:
                        if (schemaKeys[layerIndex].isScatteringMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialScattering, textureCache->getWhiteTexture());
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        if (schemaKeys[layerIndex].isEmissiveMap() && !schemaKeys[layerIndex].isLightMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                        }
                        break;
                    case graphics::MaterialKey::LIGHT_MAP_BIT:
                        if (schemaKeys[layerIndex].isLightMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getBlackTexture());
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
                        if (schemaKeys[layerIndex].isAlbedoMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (schemaKeys[layerIndex].isNormalMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        if (schemaKeys[layerIndex].isEmissiveMap() && !schemaKeys[layerIndex].isLightMap()) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT:
                        if (schemaKeys[layerIndex]._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT]) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialShade, textureCache->getWhiteTexture());
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT:
                        if (schemaKeys[layerIndex]._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT]) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialShadingShift, textureCache->getBlackTexture());
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT:
                        if (schemaKeys[layerIndex]._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT]) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialMatcap, textureCache->getBlackTexture());
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT:
                        if (schemaKeys[layerIndex]._flags[NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT]) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialRim, textureCache->getWhiteTexture());
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT:
                        if (schemaKeys[layerIndex]._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT]) {
                            drawMaterialTextures[layerIndex]->setTexture(gr::Texture::MaterialUVAnimationMask, textureCache->getWhiteTexture());
                        }
                        break;
                    default:
                        // everything else is initialized to the correct default values in ToonSchema()
                        break;
                }
            }
        }
    }

    auto& schemaBuffer = multiMaterial.getSchemaBuffer();
    uint32_t materialKey = 0;
    for (uint8_t i = 0; i < multiMaterial.getLayers(); i++) {
        uint32_t schemaKey = (uint32_t)schemaKeys[i]._flags.to_ulong();
        materialKey |= schemaKey;
        if (multiMaterial.isMToon()) {
            toonSchemas[i]._key = schemaKey;
            schemaBuffer._buffer->setSubData(i, toonSchemas[i]);
        } else {
            schemas[i]._key = schemaKey;
            schemaBuffer._buffer->setSubData(i, schemas[i]);
        }
    }
    multiMaterial.setMaterialKey(graphics::MaterialKey(materialKey));
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

    static TextureTablePointer defaultMaterialTextures = std::make_shared<TextureTable>(TEXTURE_TABLE_COUNT_1_LAYER_MATERIAL);
    static BufferView defaultMaterialSchema;
    static TextureTablePointer defaultMToonMaterialTextures = std::make_shared<TextureTable>(TEXTURE_TABLE_COUNT_1_LAYER_MATERIAL);
    static BufferView defaultMToonMaterialSchema;
    static BufferView defaultTriplanarScale;

    static std::once_flag once;
    std::call_once(once, [textureCache] {
        graphics::MultiMaterial::Schema schema;
        defaultMaterialSchema = gpu::BufferView(std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, sizeof(schema), (const gpu::Byte*) &schema, sizeof(schema)));

        defaultMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialMetallic, textureCache->getBlackTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialRoughness, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialOcclusion, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialScattering, textureCache->getWhiteTexture());
        // MaterialEmissiveLightmap has to be set later

        graphics::MultiMaterial::MToonSchema toonSchema;
        defaultMToonMaterialSchema = gpu::BufferView(std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, sizeof(toonSchema), (const gpu::Byte*) &toonSchema, sizeof(toonSchema)));

        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialShade, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialShadingShift, textureCache->getBlackTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialMatcap, textureCache->getBlackTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialRim, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialUVAnimationMask, textureCache->getWhiteTexture());

        vec4 triplanarScale = vec4(1.0f);
        defaultTriplanarScale = BufferView(std::make_shared<Buffer>(gpu::Buffer::UniformBuffer, sizeof(triplanarScale), (const Byte*) &triplanarScale, sizeof(triplanarScale)));
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
            uint8_t numLayers = multiMaterial.getLayers();
            auto& textureTables = multiMaterial.getTextureTables();
            // We are limited to 16 textures at once.  The last 3 slots are reserved for the splat map, skybox, and ambientFresnelLUT (the latter two for transparents only)
            // So for 2 layers, we are limited to 6 textures each, and for 3 layers we are limited to 4 each
            uint8_t offset = numLayers < 3 ? 6 : 4;
            for (uint8_t i = 0; i < numLayers; i++) {
                // Since material's texture table has 8 slots, for second and third material we have to make sure that these
                // won't overwrite or unbind ambientFresnelLUT texture slot.

                batch.setResourceTextureTable(textureTables[i], i * offset);
            }
            if (multiMaterial.isSplatMap()) {
                batch.setResourceTexture(gr::Texture::MaterialSplat, multiMaterial.getSplatMap());
            }
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

