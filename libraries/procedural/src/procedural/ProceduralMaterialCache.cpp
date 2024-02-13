//
//  Created by Sam Gondelman on 2/9/2018
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "ProceduralMaterialCache.h"

#include "QJsonObject"
#include "QJsonDocument"
#include "QJsonArray"

#include "RegisteredMetaTypes.h"

#include "Procedural.h"
#include "ReferenceMaterial.h"

NetworkMaterialResource::NetworkMaterialResource(const QUrl& url) :
    Resource(url) {}

void NetworkMaterialResource::downloadFinished(const QByteArray& data) {
    parsedMaterials.reset();

    if (_url.toString().contains(".json")) {
        parsedMaterials = parseJSONMaterials(QJsonDocument::fromJson(data), _url);
    }

    // TODO: parse other material types

    finishedLoading(true);
}

/*@jsdoc
 * <p>An RGB or SRGB color value.</p>
 * <table>
 *   <thead>
 *     <tr><th>Index</th><th>Type</th><th>Attributes</th><th>Default</th><th>Value</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>0</code></td><td>number</td><td></td><td></td>
 *       <td>Red component value. Number in the range <code>0.0</code> &ndash; <code>1.0</code>.</td></tr>
 *     <tr><td><code>1</code></td><td>number</td><td></td><td></td>
 *       <td>Green component value. Number in the range <code>0.0</code> &ndash; <code>1.0</code>.</td></tr>
 *     <tr><td><code>2</code></td><td>number</td><td></td><td></td>
 *       <td>Blue component value. Number in the range <code>0.0</code> &ndash; <code>1.0</code>.</td></tr>
 *     <tr><td><code>3</code></td><td>boolean</td><td>&lt;optional&gt;</td><td>false</td>
 *       <td>If <code>true</code> then the color is an SRGB color.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {array} RGBS
 */
bool NetworkMaterialResource::parseJSONColor(const QJsonValue& array, glm::vec3& color, bool& isSRGB) {
    if (array.isArray()) {
        QJsonArray colorArray = array.toArray();
        if (colorArray.size() >= 3 && colorArray[0].isDouble() && colorArray[1].isDouble() && colorArray[2].isDouble()) {
            isSRGB = true;
            if (colorArray.size() >= 4) {
                if (colorArray[3].isBool()) {
                    isSRGB = colorArray[3].toBool();
                }
            }
            color = glm::vec3(colorArray[0].toDouble(), colorArray[1].toDouble(), colorArray[2].toDouble());
            return true;
        }
    } else if (array.isObject()) {
        bool toReturn;
        isSRGB = true;
        color = vec3FromVariant(array.toObject(), toReturn);
        return toReturn;
    }
    return false;
}

/*@jsdoc
 * A material or set of materials used by a {@link Entities.EntityType|Material entity}.
 * @typedef {object} Entities.MaterialResource
 * @property {number} materialVersion=1 - The version of the material. <em>Currently not used.</em>
 * @property {Entities.Material|Entities.Material[]|string} materials - The details of the material or materials, or the ID of another
 *     Material entity.
 */
NetworkMaterialResource::ParsedMaterials NetworkMaterialResource::parseJSONMaterials(const QJsonDocument& materialJSON, const QUrl& baseUrl) {
    ParsedMaterials toReturn;
    if (!materialJSON.isNull() && materialJSON.isObject()) {
        QJsonObject materialJSONObject = materialJSON.object();
        for (auto& key : materialJSONObject.keys()) {
            if (key == "materialVersion") {
                auto value = materialJSONObject.value(key);
                if (value.isDouble()) {
                    toReturn.version = (uint)value.toInt();
                }
            } else if (key == "materials") {
                auto materialsValue = materialJSONObject.value(key);
                if (materialsValue.isArray()) {
                    QJsonArray materials = materialsValue.toArray();
                    for (auto material : materials) {
                        if (!material.isNull() && (material.isObject() || material.isString())) {
                            auto parsedMaterial = parseJSONMaterial(material, baseUrl);
                            toReturn.networkMaterials[parsedMaterial.first] = parsedMaterial.second;
                            toReturn.names.push_back(parsedMaterial.first);
                        }
                    }
                } else if (materialsValue.isObject() || materialsValue.isString()) {
                    auto parsedMaterial = parseJSONMaterial(materialsValue, baseUrl);
                    toReturn.networkMaterials[parsedMaterial.first] = parsedMaterial.second;
                    toReturn.names.push_back(parsedMaterial.first);
                }
            }
        }
    }

    return toReturn;
}

NetworkMaterialResource::ParsedMaterials NetworkMaterialResource::parseMaterialForUUID(const QJsonValue& entityIDJSON) {
    ParsedMaterials toReturn;
    if (!entityIDJSON.isNull() && entityIDJSON.isString()) {
        auto parsedMaterial = parseJSONMaterial(entityIDJSON);
        toReturn.networkMaterials[parsedMaterial.first] = parsedMaterial.second;
        toReturn.names.push_back(parsedMaterial.first);
    }

    return toReturn;
}

/*@jsdoc
 * A material used in a {@link Entities.MaterialResource|MaterialResource}.
 * @typedef {object} Entities.Material
 * @property {string} name="" - A name for the material. Supported by all material models.
 * @property {string} model="hifi_pbr" - Different material models support different properties and rendering modes.
 *     Supported models are: <code>"hifi_pbr"</code>, <code>"hifi_shader_simple"</code>, and <code>"vrm_mtoon"</code>.
 * @property {ColorFloat|RGBS|string} emissive - The emissive color, i.e., the color that the material emits. A 
 *     {@link ColorFloat} value is treated as sRGB and must have component values in the range <code>0.0</code> &ndash; 
 *     <code>1.0</code>. A {@link RGBS} value can be either RGB or sRGB. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>,
 *     <code>"vrm_mtoon"</code>.
 * @property {number|string} opacity=1.0 - The opacity, range <code>0.0</code> &ndash; <code>1.0</code>.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: all.
 * @property {boolean|string} unlit=false - <code>true</code> if the material is unaffected by lighting, <code>false</code> if 
 *     it is lit by the key light and local lights.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {ColorFloat|RGBS|string} albedo - The albedo color. A {@link ColorFloat} value is treated as sRGB and must have
 *     component values in the range <code>0.0</code> &ndash; <code>1.0</code>. A {@link RGBS} value can be either RGB or sRGB.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: all.
 * @property {number|string} roughness - The roughness, range <code>0.0</code> &ndash; <code>1.0</code>. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {number|string} metallic - The metallicness, range <code>0.0</code> &ndash; <code>1.0</code>. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {number|string} scattering - The scattering, range <code>0.0</code> &ndash; <code>1.0</code>. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {string} emissiveMap - The URL of the emissive texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.  Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>,
 *     <code>"vrm_mtoon"</code>.
 * @property {string} albedoMap - The URL of the albedo texture image, or an entity ID.  An entity ID may be that of an Image
 *     or Web entity.  Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>,
 *     <code>"vrm_mtoon"</code>.
 * @property {string} opacityMap - The URL of the opacity texture image, or an entity ID.  An entity ID may be that of an Image
 *     or Web entity.  Set the value the same as the <code>albedoMap</code> value for transparency.  Supported models: <code>"hifi_pbr"</code>,
 *     <code>"vrm_mtoon"</code>.
 * @property {string} opacityMapMode - The mode defining the interpretation of the opacity map. Values can be:
 *     <ul>
 *         <li><code>"OPACITY_MAP_OPAQUE"</code> for ignoring the opacity map information.</li>
 *         <li><code>"OPACITY_MAP_MASK"</code> for using the <code>opacityMap</code> as a mask, where only the texel greater 
 *         than <code>opacityCutoff</code> are visible and rendered opaque.</li>
 *         <li><code>"OPACITY_MAP_BLEND"</code> for using the <code>opacityMap</code> for alpha blending the material surface 
 *         with the background.</li>
 *     </ul>
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: all.
 * @property {number|string} opacityCutoff - The opacity cutoff threshold used to determine the opaque texels of the 
 *     <code>opacityMap</code> when <code>opacityMapMode</code> is <code>"OPACITY_MAP_MASK"</code>. Range <code>0.0</code> 
 *     &ndash; <code>1.0</code>.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: all.
 * @property {string} cullFaceMode="CULL_BACK" - The mode defining which side of the geometry should be rendered. Values can be:
 *     <ul>
 *         <li><code>"CULL_NONE"</code> to render both sides of the geometry.</li>
 *         <li><code>"CULL_FRONT"</code> to cull the front faces of the geometry.</li>
 *         <li><code>"CULL_BACK"</code> (the default) to cull the back faces of the geometry.</li>
 *     </ul>
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: all.
 * @property {string} roughnessMap - The URL of the roughness texture image. You can use this or <code>glossMap</code>, but not 
 *     both. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {string} glossMap - The URL of the gloss texture image. You can use this or <code>roughnessMap</code>, but not 
 *     both. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {string} metallicMap - The URL of the metallic texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.  You can use this or <code>specularMap</code>, but not both.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {string} specularMap - The URL of the specular texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.  You can use this or <code>metallicMap</code>, but not both.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {string} normalMap - The URL of the normal texture image, or an entity ID.  An entity ID may be that of an Image
 *     or Web entity.  You can use this or <code>bumpMap</code>, but not both. Set to <code>"fallthrough"</code> to fall
 *     through to the material below. Supported models: <code>"hifi_pbr"</code>, <code>"vrm_mtoon"</code>.
 * @property {string} bumpMap - The URL of the bump texture image, or an entity ID.  An entity ID may be that of an Image
 *     or Web entity.  You can use this or <code>normalMap</code>, but not both. Set to <code>"fallthrough"</code> to
 *     fall through to the material below. Supported models: <code>"hifi_pbr"</code>, <code>"vrm_mtoon"</code>.
 * @property {string} occlusionMap - The URL of the occlusion texture image, or an entity ID.  An entity ID may be that of
 *     an Image or Web entity.  Set to <code>"fallthrough"</code> to fall through to the material below.
 *     Supported models: <code>"hifi_pbr"</code>.
 * @property {string} scatteringMap - The URL of the scattering texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.  Only used if <code>normalMap</code> or <code>bumpMap</code> is specified.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {string} lightMap - The URL of the light map texture image, or an entity ID.  An entity ID may be that of an Image
 *     or Web entity.  Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>.
 * @property {Mat4|string} texCoordTransform0 - The transform to use for all of the maps apart from <code>occlusionMap</code> 
 *     and <code>lightMap</code>. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>, <code>"vrm_mtoon"</code>.
 * @property {Mat4|string} texCoordTransform1 - The transform to use for <code>occlusionMap</code> and <code>lightMap</code>. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>, <code>"vrm_mtoon"</code>.
 * @property {string} lightmapParams - Parameters for controlling how <code>lightMap</code> is used. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>. 
 *     <p><em>Currently not used.</em></p>
 * @property {string} materialParams - Parameters for controlling the material projection and repetition. 
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"hifi_pbr"</code>, <code>"vrm_mtoon"</code>.
 *     <p><em>Currently not used.</em></p>
 * @property {boolean} defaultFallthrough=false - <code>true</code> if all properties fall through to the material below 
 *     unless they are set, <code>false</code> if properties respect their individual fall-through settings. 
 *     Supported models: all.
 * @property {ProceduralData} procedural - The definition of a procedural shader material.  Supported models: <code>"hifi_shader_simple"</code>.
 * @property {ColorFloat|RGBS|string} shade - The shade color. A {@link ColorFloat} value is treated as sRGB and must have
 *     component values in the range <code>0.0</code> &ndash; <code>1.0</code>. A {@link RGBS} value can be either RGB or sRGB.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {string} shadeMap - The URL of the shade texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} shadingShift - The shading shift.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {string} shadingShiftMap - The URL of the shading shift texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} shadingToony - The shading toony factor. Range <code>0.0</code> &ndash; <code>1.0</code>.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {ColorFloat|RGBS|string} matcap - The matcap color. A {@link ColorFloat} value is treated as sRGB and must have
 *     component values in the range <code>0.0</code> &ndash; <code>1.0</code>. A {@link RGBS} value can be either RGB or sRGB.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {string} matcapMap - The URL of the matcap texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {ColorFloat|RGBS|string} parametricRim - The rim color. A {@link ColorFloat} value is treated as sRGB and must have
 *     component values in the range <code>0.0</code> &ndash; <code>1.0</code>. A {@link RGBS} value can be either RGB or sRGB.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} parametricRimFresnelPower - The parametric rim fresnel exponent.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} parametricRimLift - The parametric rim lift factor.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {string} rimMap - The URL of the rim texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} rimLightingMix - How much to mix between the rim color and normal lighting. Range <code>0.0</code>
 *     &ndash; <code>1.0</code>.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {string} outlineWidthMode="none" - The mode defining how to render the outline. Values can be:
 *     <ul>
 *         <li><code>"none"</code> (the default) to not render an outline.</li>
 *         <li><code>"worldCoordinates"</code> to render an outline with a constant world size, i.e. its apparent size depends on distance.</li>
 *         <li><code>"screenCoordinates"</code> to render an outline with a constant screen size, i.e. its apparent size remains constant.</li>
 *     </ul>
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} outlineWidth - The width of the outline, in meters if <code>outlineWidthMode</code> is <code>"worldCoordinates"</code>,
 *     or a ratio of the screen height if <code>outlineWidthMode</code> is <code>"screenCoordinates"</code>.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {ColorFloat|RGBS|string} outline - The outline color. A {@link ColorFloat} value is treated as sRGB and must have
 *     component values in the range <code>0.0</code> &ndash; <code>1.0</code>. A {@link RGBS} value can be either RGB or sRGB.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {string} uvAnimationMaskMap - The URL of the UV animation mask texture image, or an entity ID.  An entity ID may be that of an
 *     Image or Web entity.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} uvAnimationScrollXSpeed - The speed of the UV scrolling animation in the X dimension, in UV units per second.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} uvAnimationScrollYSpeed - The speed of the UV scrolling animation in the Y dimension, in UV units per second.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 * @property {number|string} uvAnimationRotationSpeed - The speed of the UV scrolling rotation about (0.5, 0.5), in radians per second.
 *     Set to <code>"fallthrough"</code> to fall through to the material below. Supported models: <code>"vrm_mtoon"</code>.
 */
// Note: See MaterialEntityItem.h for default values used in practice.
std::pair<std::string, std::shared_ptr<NetworkMaterial>> NetworkMaterialResource::parseJSONMaterial(const QJsonValue& materialJSONValue, const QUrl& baseUrl) {
    std::string name = "";
    std::shared_ptr<NetworkMaterial> networkMaterial;

    if (materialJSONValue.isString()) {
        QString uuidString = materialJSONValue.toString();
        name = uuidString.toStdString();
        QUuid uuid = QUuid(uuidString);
        if (!uuid.isNull()) {
            networkMaterial = std::make_shared<ReferenceMaterial>(uuid);
        }
        return std::pair<std::string, std::shared_ptr<NetworkMaterial>>(name, networkMaterial);
    }

    QJsonObject materialJSON = materialJSONValue.toObject();

    std::string modelString = graphics::Material::HIFI_PBR;
    auto modelJSONIter = materialJSON.find("model");
    if (modelJSONIter != materialJSON.end() && modelJSONIter.value().isString()) {
        modelString = modelJSONIter.value().toString().toStdString();
    }

    std::array<glm::mat4, graphics::Material::NUM_TEXCOORD_TRANSFORMS> texcoordTransforms;

    const QString FALLTHROUGH("fallthrough");
    if (modelString == graphics::Material::HIFI_PBR || modelString == graphics::Material::VRM_MTOON) {
        std::shared_ptr<NetworkMaterial> material;
        if (modelString == graphics::Material::HIFI_PBR) {
            material = std::make_shared<NetworkMaterial>();
        } else {
            material = std::make_shared<NetworkMToonMaterial>();
        }
        for (auto& key : materialJSON.keys()) {
            if (key == "name") {
                auto nameJSON = materialJSON.value(key);
                if (nameJSON.isString()) {
                    name = nameJSON.toString().toStdString();
                    material->setName(name);
                }
            } else if (key == "emissive") {
                auto value = materialJSON.value(key);
                if (value.isString() && value.toString() == FALLTHROUGH) {
                    material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::EMISSIVE_VAL_BIT);
                } else {
                    glm::vec3 color;
                    bool isSRGB;
                    bool valid = parseJSONColor(value, color, isSRGB);
                    if (valid) {
                        material->setEmissive(color, isSRGB);
                    }
                }
            } else if (key == "opacity") {
                auto value = materialJSON.value(key);
                if (value.isString() && value.toString() == FALLTHROUGH) {
                    material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::OPACITY_VAL_BIT);
                } else if (value.isDouble()) {
                    material->setOpacity(value.toDouble());
                }
            } else if (key == "albedo") {
                auto value = materialJSON.value(key);
                if (value.isString() && value.toString() == FALLTHROUGH) {
                    material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::ALBEDO_VAL_BIT);
                } else {
                    glm::vec3 color;
                    bool isSRGB;
                    bool valid = parseJSONColor(value, color, isSRGB);
                    if (valid) {
                        material->setAlbedo(color, isSRGB);
                    }
                }
            } else if (key == "opacityMapMode") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::OPACITY_MAP_MODE_BIT);
                    } else {
                        graphics::MaterialKey::OpacityMapMode mode;
                        if (graphics::MaterialKey::getOpacityMapModeFromName(valueString.toStdString(), mode)) {
                            material->setOpacityMapMode(mode);
                        }
                    }
                }
           } else if (key == "opacityCutoff") {
                auto value = materialJSON.value(key);
                if (value.isString() && value.toString() == FALLTHROUGH) {
                    material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::OPACITY_CUTOFF_VAL_BIT);
                } else if (value.isDouble()) {
                    material->setOpacityCutoff(value.toDouble());
                }
            } else if (key == "cullFaceMode") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::Material::ExtraFlagBit::CULL_FACE_MODE);
                    } else {
                        graphics::MaterialKey::CullFaceMode mode;
                        if (graphics::MaterialKey::getCullFaceModeFromName(valueString.toStdString(), mode)) {
                            material->setCullFaceMode(mode);
                        }
                    }
                }
           } else if (key == "emissiveMap") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::EMISSIVE_MAP_BIT);
                    } else {
                        material->setEmissiveMap(baseUrl.resolved(valueString));
                    }
                }
            } else if (key == "albedoMap") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    QString valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::ALBEDO_MAP_BIT);
                    } else {
                        bool useAlphaChannel = false;
                        auto opacityMap = materialJSON.find("opacityMap");
                        if (opacityMap != materialJSON.end() && opacityMap->isString() && opacityMap->toString() == valueString) {
                            useAlphaChannel = true;
                        }
                        material->setAlbedoMap(baseUrl.resolved(valueString), useAlphaChannel);
                    }
                }
            } else if (key == "normalMap") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::NORMAL_MAP_BIT);
                    } else {
                        material->setNormalMap(baseUrl.resolved(valueString), false);
                    }
                }
            } else if (key == "bumpMap") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::NORMAL_MAP_BIT);
                    } else {
                        material->setNormalMap(baseUrl.resolved(valueString), true);
                    }
                }
            } else if (key == "texCoordTransform0") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::Material::ExtraFlagBit::TEXCOORDTRANSFORM0);
                    }
                } else if (value.isObject()) {
                    auto valueVariant = value.toVariant();
                    glm::mat4 transform = mat4FromVariant(valueVariant);
                    texcoordTransforms[0] = transform;
                }
            } else if (key == "texCoordTransform1") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::Material::ExtraFlagBit::TEXCOORDTRANSFORM1);
                    }
                } else if (value.isObject()) {
                    auto valueVariant = value.toVariant();
                    glm::mat4 transform = mat4FromVariant(valueVariant);
                    texcoordTransforms[1] = transform;
                }
            } else if (key == "materialParams") {
                auto value = materialJSON.value(key);
                if (value.isString()) {
                    auto valueString = value.toString();
                    if (valueString == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::Material::ExtraFlagBit::MATERIAL_PARAMS);
                    }
                }
                // TODO: implement materialParams and update JSDoc
            } else if (key == "defaultFallthrough") {
                auto value = materialJSON.value(key);
                if (value.isBool()) {
                    material->setDefaultFallthrough(value.toBool());
                }
            }

            if (modelString == graphics::Material::HIFI_PBR) {
                if (key == "unlit") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::UNLIT_VAL_BIT);
                    } else if (value.isBool()) {
                        material->setUnlit(value.toBool());
                    }
                } else if (key == "roughness") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::GLOSSY_VAL_BIT);
                    } else if (value.isDouble()) {
                        material->setRoughness(value.toDouble());
                    }
                } else if (key == "metallic") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::METALLIC_VAL_BIT);
                    } else if (value.isDouble()) {
                        material->setMetallic(value.toDouble());
                    }
                } else if (key == "scattering") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::SCATTERING_VAL_BIT);
                    } else if (value.isDouble()) {
                        material->setScattering(value.toDouble());
                    }
                } else if (key == "roughnessMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::ROUGHNESS_MAP_BIT);
                        } else {
                            material->setRoughnessMap(baseUrl.resolved(valueString), false);
                        }
                    }
                } else if (key == "glossMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::ROUGHNESS_MAP_BIT);
                        } else {
                            material->setRoughnessMap(baseUrl.resolved(valueString), true);
                        }
                    }
                } else if (key == "metallicMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::METALLIC_MAP_BIT);
                        } else {
                            material->setMetallicMap(baseUrl.resolved(valueString), false);
                        }
                    }
                } else if (key == "specularMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::METALLIC_MAP_BIT);
                        } else {
                            material->setMetallicMap(baseUrl.resolved(valueString), true);
                        }
                    }
                } else if (key == "occlusionMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::OCCLUSION_MAP_BIT);
                        } else {
                            material->setOcclusionMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "scatteringMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::SCATTERING_MAP_BIT);
                        } else {
                            material->setScatteringMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "lightMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::LIGHT_MAP_BIT);
                        } else {
                            material->setLightMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "lightmapParams") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(graphics::Material::ExtraFlagBit::LIGHTMAP_PARAMS);
                        }
                    }
                    // TODO: implement lightmapParams and update JSDoc
                }
            } else if (modelString == graphics::Material::VRM_MTOON) {
                auto toonMaterial = std::static_pointer_cast<NetworkMToonMaterial>(material);
                if (key == "shade") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT);
                    } else {
                        glm::vec3 color;
                        bool isSRGB;
                        bool valid = parseJSONColor(value, color, isSRGB);
                        if (valid) {
                            toonMaterial->setShade(color, isSRGB);
                        }
                    }
                } else if (key == "shadeMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT);
                        } else {
                            toonMaterial->setShadeMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "shadingShift") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setShadingShift(value.toDouble());
                    }
                } else if (key == "shadingShiftMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT);
                        } else {
                            toonMaterial->setShadingShiftMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "shadingToony") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setShadingToony(value.toDouble());
                    }
                } else if (key == "matcap") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT);
                    } else {
                        glm::vec3 color;
                        bool isSRGB;
                        bool valid = parseJSONColor(value, color, isSRGB);
                        if (valid) {
                            toonMaterial->setMatcap(color, isSRGB);
                        }
                    }
                } else if (key == "matcapMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT);
                        } else {
                            toonMaterial->setMatcapMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "parametricRim") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT);
                    } else {
                        glm::vec3 color;
                        bool isSRGB;
                        bool valid = parseJSONColor(value, color, isSRGB);
                        if (valid) {
                            toonMaterial->setParametricRim(color, isSRGB);
                        }
                    }
                } else if (key == "parametricRimFresnelPower") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setParametricRimFresnelPower(value.toDouble());
                    }
                } else if (key == "parametricRimLift") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setParametricRimLift(value.toDouble());
                    }
                } else if (key == "rimMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT);
                        } else {
                            toonMaterial->setRimMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "rimLightingMix") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setRimLightingMix(value.toDouble());
                    }
                } else if (key == "uvAnimationMaskMap") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT);
                        } else {
                            toonMaterial->setUVAnimationMaskMap(baseUrl.resolved(valueString));
                        }
                    }
                } else if (key == "uvAnimationScrollXSpeed") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setUVAnimationScrollXSpeed(value.toDouble());
                    }
                } else if (key == "uvAnimationScrollYSpeed") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setUVAnimationScrollYSpeed(value.toDouble());
                    }
                } else if (key == "uvAnimationRotationSpeed") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setUVAnimationRotationSpeed(value.toDouble());
                    }
                } else if (key == "outlineWidthMode") {
                    auto value = materialJSON.value(key);
                    if (value.isString()) {
                        auto valueString = value.toString();
                        if (valueString == FALLTHROUGH) {
                            material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::OUTLINE_WIDTH_MODE_VAL_BIT);
                        } else {
                            NetworkMToonMaterial::OutlineWidthMode mode;
                            if (NetworkMToonMaterial::getOutlineWidthModeFromName(valueString.toStdString(), mode)) {
                                // FIXME: Outlines are currently disabled because they're buggy
                                //toonMaterial->setOutlineWidthMode(mode);
                            }
                        }
                    }
                } else if (key == "outlineWidth") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::OUTLINE_WIDTH_VAL_BIT);
                    } else if (value.isDouble()) {
                        toonMaterial->setOutlineWidth(value.toDouble());
                    }
                } else if (key == "outline") {
                    auto value = materialJSON.value(key);
                    if (value.isString() && value.toString() == FALLTHROUGH) {
                        material->setPropertyDoesFallthrough(NetworkMToonMaterial::MToonFlagBit::OUTLINE_VAL_BIT);
                    } else {
                        glm::vec3 color;
                        bool isSRGB;
                        bool valid = parseJSONColor(value, color, isSRGB);
                        if (valid) {
                            toonMaterial->setOutline(color, isSRGB);
                        }
                    }
                }
                // TODO: support outlineWidthTexture and outlineLightingMix
            }
        }

        // Do this after the texture maps are defined, so it overrides the default transforms
        for (int i = 0; i < graphics::Material::NUM_TEXCOORD_TRANSFORMS; i++) {
            mat4 newTransform = texcoordTransforms[i];
            if (newTransform != mat4() || newTransform != material->getTexCoordTransform(i)) {
                material->setTexCoordTransform(i, newTransform);
            }
        }
        networkMaterial = material;
    } else if (modelString == graphics::Material::HIFI_SHADER_SIMPLE) {
        auto material = std::make_shared<graphics::ProceduralMaterial>();
        for (auto& key : materialJSON.keys()) {
            if (key == "name") {
                auto nameJSON = materialJSON.value(key);
                if (nameJSON.isString()) {
                    name = nameJSON.toString().toStdString();
                    material->setName(name);
                }
            } else if (key == "opacity") {
                auto value = materialJSON.value(key);
                if (value.isString() && value.toString() == FALLTHROUGH) {
                    material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::OPACITY_VAL_BIT);
                } else if (value.isDouble()) {
                    material->setOpacity(value.toDouble());
                }
            } else if (key == "albedo") {
                auto value = materialJSON.value(key);
                if (value.isString() && value.toString() == FALLTHROUGH) {
                    material->setPropertyDoesFallthrough(graphics::MaterialKey::FlagBit::ALBEDO_VAL_BIT);
                } else {
                    glm::vec3 color;
                    bool isSRGB;
                    bool valid = parseJSONColor(value, color, isSRGB);
                    if (valid) {
                        material->setAlbedo(color, isSRGB);
                    }
                }
            } else if (key == "defaultFallthrough") {
                auto value = materialJSON.value(key);
                if (value.isBool()) {
                    material->setDefaultFallthrough(value.toBool());
                }
            } else if (key == "procedural") {
                auto value = materialJSON.value(key);
                material->setProceduralData(QJsonDocument::fromVariant(value.toVariant()).toJson());
            }
        }
        networkMaterial = material;
    }

    if (networkMaterial) {
        networkMaterial->setModel(modelString);
    }

    return std::pair<std::string, std::shared_ptr<NetworkMaterial>>(name, networkMaterial);
}

NetworkMaterialResourcePointer MaterialCache::getMaterial(const QUrl& url) {
    return ResourceCache::getResource(url).staticCast<NetworkMaterialResource>();
}

QSharedPointer<Resource> MaterialCache::createResource(const QUrl& url) {
    return QSharedPointer<NetworkMaterialResource>(new NetworkMaterialResource(url), &Resource::deleter);
}

QSharedPointer<Resource> MaterialCache::createResourceCopy(const QSharedPointer<Resource>& resource) {
    return QSharedPointer<NetworkMaterialResource>(new NetworkMaterialResource(*resource.staticCast<NetworkMaterialResource>()), &Resource::deleter);
}

NetworkMaterial::NetworkMaterial(const NetworkMaterial& m) :
    Material(m),
    _textures(m._textures),
    _albedoTransform(m._albedoTransform),
    _lightmapTransform(m._lightmapTransform),
    _lightmapParams(m._lightmapParams),
    _isOriginal(m._isOriginal)
{}

const QString NetworkMaterial::NO_TEXTURE = QString();

const QString& NetworkMaterial::getTextureName(MapChannel channel) {
    if (_textures[channel].texture) {
        return _textures[channel].name;
    }
    return NO_TEXTURE;
}

QUrl NetworkMaterial::getTextureUrl(const QUrl& baseUrl, const HFMTexture& texture) {
    if (texture.content.isEmpty()) {
        // External file: search relative to the baseUrl, in case filename is relative
        return baseUrl.resolved(QUrl(texture.filename));
    } else {
        // Inlined file: cache under the fbx file to avoid namespace clashes
        // NOTE: We cannot resolve the path because filename may be an absolute path
        assert(texture.filename.size() > 0);
        auto baseUrlStripped = baseUrl.toDisplayString(QUrl::RemoveFragment | QUrl::RemoveQuery | QUrl::RemoveUserInfo);
        if (texture.filename.at(0) == '/') {
            return baseUrlStripped + texture.filename;
        } else {
            return baseUrlStripped + '/' + texture.filename;
        }
    }
}

graphics::TextureMapPointer NetworkMaterial::fetchTextureMap(const QUrl& baseUrl, const HFMTexture& hfmTexture,
                                                             image::TextureUsage::Type type, MapChannel channel) {

    if (baseUrl.isEmpty()) {
        return nullptr;
    }

    const auto url = getTextureUrl(baseUrl, hfmTexture);
    const auto texture = DependencyManager::get<TextureCache>()->getTexture(url, type, hfmTexture.content, hfmTexture.maxNumPixels, hfmTexture.sourceChannel);
    _textures[channel] = Texture { hfmTexture.name, texture };

    auto map = std::make_shared<graphics::TextureMap>();
    if (texture) {
        map->setTextureSource(texture->_textureSource);
    }
    map->setTextureTransform(hfmTexture.transform);

    return map;
}

graphics::TextureMapPointer NetworkMaterial::fetchTextureMap(const QUrl& url, image::TextureUsage::Type type, MapChannel channel) {
    auto textureCache = DependencyManager::get<TextureCache>();
    if (textureCache && !url.isEmpty()) {
        auto texture = textureCache->getTexture(url, type);
        _textures[channel].texture = texture;

        auto map = std::make_shared<graphics::TextureMap>();
        if (texture) {
            map->setTextureSource(texture->_textureSource);
        }

        return map;
    }
    return nullptr;
}

void NetworkMaterial::setAlbedoMap(const QUrl& url, bool useAlphaChannel) {
    auto map = fetchTextureMap(url, image::TextureUsage::ALBEDO_TEXTURE, MapChannel::ALBEDO_MAP);
    if (map) {
        map->setUseAlphaChannel(useAlphaChannel);
        setTextureMap(MapChannel::ALBEDO_MAP, map);
    }
}

void NetworkMaterial::setNormalMap(const QUrl& url, bool isBumpmap) {
    auto map = fetchTextureMap(url, isBumpmap ? image::TextureUsage::BUMP_TEXTURE : image::TextureUsage::NORMAL_TEXTURE, MapChannel::NORMAL_MAP);
    if (map) {
        setTextureMap(MapChannel::NORMAL_MAP, map);
    }
}

void NetworkMaterial::setRoughnessMap(const QUrl& url, bool isGloss) {
    auto map = fetchTextureMap(url, isGloss ? image::TextureUsage::GLOSS_TEXTURE : image::TextureUsage::ROUGHNESS_TEXTURE, MapChannel::ROUGHNESS_MAP);
    if (map) {
        setTextureMap(MapChannel::ROUGHNESS_MAP, map);
    }
}

void NetworkMaterial::setMetallicMap(const QUrl& url, bool isSpecular) {
    auto map = fetchTextureMap(url, isSpecular ? image::TextureUsage::SPECULAR_TEXTURE : image::TextureUsage::METALLIC_TEXTURE, MapChannel::METALLIC_MAP);
    if (map) {
        setTextureMap(MapChannel::METALLIC_MAP, map);
    }
}

void NetworkMaterial::setOcclusionMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::OCCLUSION_TEXTURE, MapChannel::OCCLUSION_MAP);
    if (map) {
        setTextureMap(MapChannel::OCCLUSION_MAP, map);
    }
}

void NetworkMaterial::setEmissiveMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::EMISSIVE_TEXTURE, MapChannel::EMISSIVE_MAP);
    if (map) {
        setTextureMap(MapChannel::EMISSIVE_MAP, map);
    }
}

void NetworkMaterial::setScatteringMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::SCATTERING_TEXTURE, MapChannel::SCATTERING_MAP);
    if (map) {
        setTextureMap(MapChannel::SCATTERING_MAP, map);
    }
}

void NetworkMaterial::setLightMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::LIGHTMAP_TEXTURE, MapChannel::LIGHT_MAP);
    if (map) {
        //map->setTextureTransform(_lightmapTransform);
        //map->setLightmapOffsetScale(_lightmapParams.x, _lightmapParams.y);
        setTextureMap(MapChannel::LIGHT_MAP, map);
    }
}

NetworkMaterial::NetworkMaterial(const HFMMaterial& material, const QUrl& textureBaseUrl) :
    graphics::Material(*material._material)
{
    _name = material.name.toStdString();
    if (!material.albedoTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.albedoTexture, image::TextureUsage::ALBEDO_TEXTURE, MapChannel::ALBEDO_MAP);
        if (map) {
            _albedoTransform = material.albedoTexture.transform;
            map->setTextureTransform(_albedoTransform);

            if (!material.opacityTexture.filename.isEmpty()) {
                if (material.albedoTexture.filename == material.opacityTexture.filename) {
                    // Best case scenario, just indicating that the albedo map contains transparency
                    // TODO: Different albedo/opacity maps are not currently supported
                    map->setUseAlphaChannel(true);
                }
            }
        }

        setTextureMap(MapChannel::ALBEDO_MAP, map);
    }


    if (!material.normalTexture.filename.isEmpty()) {
        auto type = (material.normalTexture.isBumpmap ? image::TextureUsage::BUMP_TEXTURE : image::TextureUsage::NORMAL_TEXTURE);
        auto map = fetchTextureMap(textureBaseUrl, material.normalTexture, type, MapChannel::NORMAL_MAP);
        setTextureMap(MapChannel::NORMAL_MAP, map);
    }

    if (!material.roughnessTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.roughnessTexture, image::TextureUsage::ROUGHNESS_TEXTURE, MapChannel::ROUGHNESS_MAP);
        setTextureMap(MapChannel::ROUGHNESS_MAP, map);
    } else if (!material.glossTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.glossTexture, image::TextureUsage::GLOSS_TEXTURE, MapChannel::ROUGHNESS_MAP);
        setTextureMap(MapChannel::ROUGHNESS_MAP, map);
    }

    if (!material.metallicTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.metallicTexture, image::TextureUsage::METALLIC_TEXTURE, MapChannel::METALLIC_MAP);
        setTextureMap(MapChannel::METALLIC_MAP, map);
    } else if (!material.specularTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.specularTexture, image::TextureUsage::SPECULAR_TEXTURE, MapChannel::METALLIC_MAP);
        setTextureMap(MapChannel::METALLIC_MAP, map);
    }

    if (!material.occlusionTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.occlusionTexture, image::TextureUsage::OCCLUSION_TEXTURE, MapChannel::OCCLUSION_MAP);
        if (map) {
            map->setTextureTransform(material.occlusionTexture.transform);
        }
        setTextureMap(MapChannel::OCCLUSION_MAP, map);
    }

    if (!material.emissiveTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.emissiveTexture, image::TextureUsage::EMISSIVE_TEXTURE, MapChannel::EMISSIVE_MAP);
        setTextureMap(MapChannel::EMISSIVE_MAP, map);
    }

    if (!material.scatteringTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.scatteringTexture, image::TextureUsage::SCATTERING_TEXTURE, MapChannel::SCATTERING_MAP);
        setTextureMap(MapChannel::SCATTERING_MAP, map);
    }

    if (!material.lightmapTexture.filename.isEmpty()) {
        auto map = fetchTextureMap(textureBaseUrl, material.lightmapTexture, image::TextureUsage::LIGHTMAP_TEXTURE, MapChannel::LIGHT_MAP);
        if (map) {
            _lightmapTransform = material.lightmapTexture.transform;
            _lightmapParams = material.lightmapParams;
            map->setTextureTransform(_lightmapTransform);
            map->setLightmapOffsetScale(_lightmapParams.x, _lightmapParams.y);
        }
        setTextureMap(MapChannel::LIGHT_MAP, map);
    }
}

void NetworkMaterial::setTextures(const QVariantMap& textureMap) {
    _isOriginal = false;

    const auto& albedoName = getTextureName(MapChannel::ALBEDO_MAP);
    const auto& normalName = getTextureName(MapChannel::NORMAL_MAP);
    const auto& roughnessName = getTextureName(MapChannel::ROUGHNESS_MAP);
    const auto& metallicName = getTextureName(MapChannel::METALLIC_MAP);
    const auto& occlusionName = getTextureName(MapChannel::OCCLUSION_MAP);
    const auto& emissiveName = getTextureName(MapChannel::EMISSIVE_MAP);
    const auto& lightmapName = getTextureName(MapChannel::LIGHT_MAP);
    const auto& scatteringName = getTextureName(MapChannel::SCATTERING_MAP);

    if (!albedoName.isEmpty()) {
        auto url = textureMap.contains(albedoName) ? textureMap[albedoName].toUrl() : QUrl();
        auto map = fetchTextureMap(url, image::TextureUsage::ALBEDO_TEXTURE, MapChannel::ALBEDO_MAP);
        if (map) {
            map->setTextureTransform(_albedoTransform);
            // when reassigning the albedo texture we also check for the alpha channel used as opacity
            map->setUseAlphaChannel(true);
        }
        setTextureMap(MapChannel::ALBEDO_MAP, map);
    }

    if (!normalName.isEmpty()) {
        auto url = textureMap.contains(normalName) ? textureMap[normalName].toUrl() : QUrl();
        auto map = fetchTextureMap(url, image::TextureUsage::NORMAL_TEXTURE, MapChannel::NORMAL_MAP);
        setTextureMap(MapChannel::NORMAL_MAP, map);
    }

    if (!roughnessName.isEmpty()) {
        auto url = textureMap.contains(roughnessName) ? textureMap[roughnessName].toUrl() : QUrl();
        // FIXME: If passing a gloss map instead of a roughmap how do we know?
        auto map = fetchTextureMap(url, image::TextureUsage::ROUGHNESS_TEXTURE, MapChannel::ROUGHNESS_MAP);
        setTextureMap(MapChannel::ROUGHNESS_MAP, map);
    }

    if (!metallicName.isEmpty()) {
        auto url = textureMap.contains(metallicName) ? textureMap[metallicName].toUrl() : QUrl();
        // FIXME: If passing a specular map instead of a metallic how do we know?
        auto map = fetchTextureMap(url, image::TextureUsage::METALLIC_TEXTURE, MapChannel::METALLIC_MAP);
        setTextureMap(MapChannel::METALLIC_MAP, map);
    }

    if (!occlusionName.isEmpty()) {
        auto url = textureMap.contains(occlusionName) ? textureMap[occlusionName].toUrl() : QUrl();
        // FIXME: we need to handle the occlusion map transform here
        auto map = fetchTextureMap(url, image::TextureUsage::OCCLUSION_TEXTURE, MapChannel::OCCLUSION_MAP);
        setTextureMap(MapChannel::OCCLUSION_MAP, map);
    }

    if (!emissiveName.isEmpty()) {
        auto url = textureMap.contains(emissiveName) ? textureMap[emissiveName].toUrl() : QUrl();
        auto map = fetchTextureMap(url, image::TextureUsage::EMISSIVE_TEXTURE, MapChannel::EMISSIVE_MAP);
        setTextureMap(MapChannel::EMISSIVE_MAP, map);
    }

    if (!scatteringName.isEmpty()) {
        auto url = textureMap.contains(scatteringName) ? textureMap[scatteringName].toUrl() : QUrl();
        auto map = fetchTextureMap(url, image::TextureUsage::SCATTERING_TEXTURE, MapChannel::SCATTERING_MAP);
        setTextureMap(MapChannel::SCATTERING_MAP, map);
    }

    if (!lightmapName.isEmpty()) {
        auto url = textureMap.contains(lightmapName) ? textureMap[lightmapName].toUrl() : QUrl();
        auto map = fetchTextureMap(url, image::TextureUsage::LIGHTMAP_TEXTURE, MapChannel::LIGHT_MAP);
        if (map) {
            map->setTextureTransform(_lightmapTransform);
            map->setLightmapOffsetScale(_lightmapParams.x, _lightmapParams.y);
        }
        setTextureMap(MapChannel::LIGHT_MAP, map);
    }
}

bool NetworkMaterial::isMissingTexture() {
    for (auto& networkTexture : _textures) {
        auto& texture = networkTexture.second.texture;
        if (!texture) {
            continue;
        }
        // Failed texture downloads need to be considered as 'loaded'
        // or the object will never fade in
        bool finished = texture->isFailed() || (texture->isLoaded() && texture->getGPUTexture() && texture->getGPUTexture()->isDefined());
        if (!finished) {
            return true;
        }
    }
    return false;
}

bool NetworkMaterial::checkResetOpacityMap() {
    // If material textures are loaded, check the material translucency
    // FIXME: This should not be done here.  The opacity map should already be reset in Material::setTextureMap.
    // However, currently that code can be called before the albedo map is defined, so resetOpacityMap will fail.
    // Geometry::areTexturesLoaded() is called repeatedly until it returns true, so we do the check here for now
    const auto& albedoTexture = _textures[NetworkMaterial::MapChannel::ALBEDO_MAP];
    if (albedoTexture.texture) {
        return resetOpacityMap();
    }
    return false;
}

NetworkMToonMaterial::NetworkMToonMaterial(const NetworkMToonMaterial& material) :
    NetworkMaterial(material),
    _shade(material._shade),
    _shadingShift(material._shadingShift),
    _shadingToony(material._shadingToony),
    _matcap(material._matcap),
    _parametricRim(material._parametricRim),
    _parametricRimFresnelPower(material._parametricRimFresnelPower),
    _parametricRimLift(material._parametricRimLift),
    _rimLightingMix(material._rimLightingMix),
    _uvAnimationScrollXSpeed(material._uvAnimationScrollXSpeed),
    _uvAnimationScrollYSpeed(material._uvAnimationScrollYSpeed),
    _uvAnimationRotationSpeed(material._uvAnimationRotationSpeed),
    _outlineWidthMode(material._outlineWidthMode),
    _outlineWidth(material._outlineWidth),
    _outline(material._outline)
{}

std::string NetworkMToonMaterial::getOutlineWidthModeName(OutlineWidthMode mode) {
    const std::string names[3] = { "none", "worldCoordinates", "screenCoordinates" };
    return names[mode];
}

bool NetworkMToonMaterial::getOutlineWidthModeFromName(const std::string& modeName, OutlineWidthMode& mode) {
    for (int i = OUTLINE_NONE; i < NUM_OUTLINE_MODES; i++) {
        mode = (OutlineWidthMode)i;
        if (modeName == getOutlineWidthModeName(mode)) {
            return true;
        }
    }
    return false;
}

void NetworkMToonMaterial::setShadeMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::ALBEDO_TEXTURE, (MapChannel) MToonMapChannel::SHADE_MAP);
    if (map) {
        setTextureMap((MapChannel) MToonMapChannel::SHADE_MAP, map);
    }
}

void NetworkMToonMaterial::setShadingShiftMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::ROUGHNESS_TEXTURE, (MapChannel) MToonMapChannel::SHADING_SHIFT_MAP);
    if (map) {
        setTextureMap((MapChannel) MToonMapChannel::SHADING_SHIFT_MAP, map);
    }
}

void NetworkMToonMaterial::setMatcapMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::EMISSIVE_TEXTURE, (MapChannel)MToonMapChannel::MATCAP_MAP);
    if (map) {
        setTextureMap((MapChannel) MToonMapChannel::MATCAP_MAP, map);
    }
}

void NetworkMToonMaterial::setRimMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::ALBEDO_TEXTURE, (MapChannel)MToonMapChannel::RIM_MAP);
    if (map) {
        setTextureMap((MapChannel) MToonMapChannel::RIM_MAP, map);
    }
}

void NetworkMToonMaterial::setUVAnimationMaskMap(const QUrl& url) {
    auto map = fetchTextureMap(url, image::TextureUsage::ROUGHNESS_TEXTURE, (MapChannel)MToonMapChannel::UV_ANIMATION_MASK_MAP);
    if (map) {
        setTextureMap((MapChannel) MToonMapChannel::UV_ANIMATION_MASK_MAP, map);
    }
}

void NetworkMToonMaterial::setShade(const glm::vec3& shade, bool isSRGB) {
    _key._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT, true);
    _shade = (isSRGB ? ColorUtils::sRGBToLinearVec3(shade) : shade);
}

void NetworkMToonMaterial::setShadingShift(float shadingShift) {
    _key._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT, true);
    _shadingShift = shadingShift;
}

void NetworkMToonMaterial::setShadingToony(float shadingToony) {
    _key._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT, true);
    _shadingToony = shadingToony;
}

void NetworkMToonMaterial::setMatcap(const glm::vec3& matcap, bool isSRGB) {
    _key._flags.set(MToonFlagBit::MATCAP_VAL_BIT, true);
    _matcap = (isSRGB ? ColorUtils::sRGBToLinearVec3(matcap) : matcap);
}

void NetworkMToonMaterial::setParametricRim(const glm::vec3& parametricRim, bool isSRGB) {
    _key._flags.set(MToonFlagBit::PARAMETRIC_RIM_VAL_BIT, true);
    _parametricRim = (isSRGB ? ColorUtils::sRGBToLinearVec3(parametricRim) : parametricRim);
}

void NetworkMToonMaterial::setParametricRimFresnelPower(float parametricRimFresnelPower) {
    _key._flags.set(MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT, true);
    _parametricRimFresnelPower = parametricRimFresnelPower;
}

void NetworkMToonMaterial::setParametricRimLift(float parametricRimLift) {
    _key._flags.set(MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT, true);
    _parametricRimLift = parametricRimLift;
}

void NetworkMToonMaterial::setRimLightingMix(float rimLightingMix) {
    _key._flags.set(MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT, true);
    _rimLightingMix = rimLightingMix;
}

void NetworkMToonMaterial::setUVAnimationScrollXSpeed(float uvAnimationScrollXSpeed) {
    _key._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT, true);
    _uvAnimationScrollXSpeed = uvAnimationScrollXSpeed;
}

void NetworkMToonMaterial::setUVAnimationScrollYSpeed(float uvAnimationScrollYSpeed) {
    _key._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT, true);
    _uvAnimationScrollYSpeed = uvAnimationScrollYSpeed;
}

void NetworkMToonMaterial::setUVAnimationRotationSpeed(float uvAnimationRotationSpeed) {
    _key._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT, true);
    _uvAnimationRotationSpeed = uvAnimationRotationSpeed;
}

void NetworkMToonMaterial::setOutlineWidthMode(OutlineWidthMode mode) {
    _outlineWidthMode = mode;
}

void NetworkMToonMaterial::setOutlineWidth(float width) {
    _outlineWidth = width;
}

void NetworkMToonMaterial::setOutline(const glm::vec3& outline, bool isSRGB) {
    _outline = (isSRGB ? ColorUtils::sRGBToLinearVec3(outline) : outline);
}
