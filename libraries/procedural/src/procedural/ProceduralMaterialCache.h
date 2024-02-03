//
//  Created by Sam Gondelman on 2/9/2018
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_MaterialCache_h
#define hifi_MaterialCache_h

#include <QtCore/QSharedPointer>

#include "glm/glm.hpp"

#include <ResourceCache.h>
#include <graphics/Material.h>
#include <hfm/HFM.h>

#include <material-networking/TextureCache.h>

class NetworkMaterial : public graphics::Material {
public:
    using MapChannel = graphics::Material::MapChannel;

    NetworkMaterial() : _textures(MapChannel::NUM_MAP_CHANNELS) {}
    NetworkMaterial(const HFMMaterial& material, const QUrl& textureBaseUrl);
    NetworkMaterial(const NetworkMaterial& material);

    void setAlbedoMap(const QUrl& url, bool useAlphaChannel);
    void setNormalMap(const QUrl& url, bool isBumpmap);
    void setRoughnessMap(const QUrl& url, bool isGloss);
    void setMetallicMap(const QUrl& url, bool isSpecular);
    void setOcclusionMap(const QUrl& url);
    void setEmissiveMap(const QUrl& url);
    void setScatteringMap(const QUrl& url);
    void setLightMap(const QUrl& url);

    virtual bool isMissingTexture();
    virtual bool checkResetOpacityMap();

    class Texture {
    public:
        QString name;
        NetworkTexturePointer texture;
    };
    struct MapChannelHash {
        std::size_t operator()(MapChannel mapChannel) const {
            return static_cast<std::size_t>(mapChannel);
        }
    };
    using Textures = std::unordered_map<MapChannel, Texture, MapChannelHash>;
    Textures getTextures() { return _textures; }

protected:
    friend class Geometry;

    Textures _textures;

    static const QString NO_TEXTURE;
    const QString& getTextureName(MapChannel channel);

    void setTextures(const QVariantMap& textureMap);

    const bool& isOriginal() const { return _isOriginal; }

    graphics::TextureMapPointer fetchTextureMap(const QUrl& baseUrl, const HFMTexture& hfmTexture,
                                                image::TextureUsage::Type type, MapChannel channel);
    graphics::TextureMapPointer fetchTextureMap(const QUrl& url, image::TextureUsage::Type type, MapChannel channel);

private:
    // Helpers for the ctors
    QUrl getTextureUrl(const QUrl& baseUrl, const HFMTexture& hfmTexture);

    Transform _albedoTransform;
    Transform _lightmapTransform;
    vec2 _lightmapParams;

    bool _isOriginal { true };
};

class NetworkMToonMaterial : public NetworkMaterial {
public:
    NetworkMToonMaterial() : NetworkMaterial() {}
    NetworkMToonMaterial(const NetworkMToonMaterial& material);

    enum MToonMapChannel {
        // Keep aligned with graphics/ShaderConstants.h and graphics-scripting/ScriptableModel.cpp
        SHADE_MAP = MapChannel::ROUGHNESS_MAP,
        SHADING_SHIFT_MAP = MapChannel::METALLIC_MAP,
        MATCAP_MAP = MapChannel::OCCLUSION_MAP,
        RIM_MAP = MapChannel::SCATTERING_MAP,
        UV_ANIMATION_MASK_MAP = MapChannel::LIGHT_MAP,
    };

    enum MToonFlagBit {
        SHADE_MAP_BIT = graphics::MaterialKey::FlagBit::ROUGHNESS_MAP_BIT,
        SHADING_SHIFT_MAP_BIT = graphics::MaterialKey::FlagBit::METALLIC_MAP_BIT,
        MATCAP_MAP_BIT = graphics::MaterialKey::FlagBit::OCCLUSION_MAP_BIT,
        RIM_MAP_BIT = graphics::MaterialKey::FlagBit::SCATTERING_MAP_BIT,
        UV_ANIMATION_MASK_MAP_BIT = graphics::MaterialKey::FlagBit::LIGHT_MAP_BIT,

        SHADE_VAL_BIT = graphics::MaterialKey::FlagBit::UNLIT_VAL_BIT,
        SHADING_SHIFT_VAL_BIT = graphics::MaterialKey::FlagBit::METALLIC_VAL_BIT,
        SHADING_TOONY_VAL_BIT = graphics::MaterialKey::FlagBit::GLOSSY_VAL_BIT,
        UV_ANIMATION_SCROLL_VAL_BIT = graphics::MaterialKey::FlagBit::SCATTERING_VAL_BIT,
        MATCAP_VAL_BIT = graphics::MaterialKey::FlagBit::EXTRA_1_BIT,
        PARAMETRIC_RIM_VAL_BIT = graphics::MaterialKey::FlagBit::EXTRA_2_BIT,
        PARAMETRIC_RIM_POWER_VAL_BIT = graphics::MaterialKey::FlagBit::EXTRA_3_BIT,
        PARAMETRIC_RIM_LIFT_VAL_BIT = graphics::MaterialKey::FlagBit::EXTRA_4_BIT,
        RIM_LIGHTING_MIX_VAL_BIT = graphics::MaterialKey::FlagBit::EXTRA_5_BIT,

        OUTLINE_WIDTH_MODE_VAL_BIT = graphics::Material::ExtraFlagBit::EXTRA_1_BIT,
        OUTLINE_WIDTH_VAL_BIT = graphics::Material::ExtraFlagBit::EXTRA_2_BIT,
        OUTLINE_VAL_BIT = graphics::Material::ExtraFlagBit::EXTRA_3_BIT,
    };

    enum OutlineWidthMode {
        OUTLINE_NONE = 0,
        OUTLINE_WORLD,
        OUTLINE_SCREEN,

        NUM_OUTLINE_MODES
    };
    static std::string getOutlineWidthModeName(OutlineWidthMode mode);
    // find the enum value from a string, return true if match found
    static bool getOutlineWidthModeFromName(const std::string& modeName, OutlineWidthMode& mode);

    bool isMToon() const override { return true; }

    void setShadeMap(const QUrl& url);
    void setShadingShiftMap(const QUrl& url);
    void setMatcapMap(const QUrl& url);
    void setRimMap(const QUrl& url);
    void setUVAnimationMaskMap(const QUrl& url);

    void setShade(const glm::vec3& shade, bool isSRGB = true);
    glm::vec3 getShade(bool SRGB = true) const override { return (SRGB ? ColorUtils::tosRGBVec3(_shade) : _shade); }

    void setShadingShift(float shadeShift);
    float getShadingShift() const override { return _shadingShift; }

    void setShadingToony(float shadingToony);
    float getShadingToony() const override { return _shadingToony; }

    void setMatcap(const glm::vec3& matcap, bool isSRGB = true);
    glm::vec3 getMatcap(bool SRGB = true) const override { return (SRGB ? ColorUtils::tosRGBVec3(_matcap) : _matcap); }

    void setParametricRim(const glm::vec3& parametricRim, bool isSRGB = true);
    glm::vec3 getParametricRim(bool SRGB = true) const override { return (SRGB ? ColorUtils::tosRGBVec3(_parametricRim) : _parametricRim); }

    void setParametricRimFresnelPower(float parametricRimFresnelPower);
    float getParametricRimFresnelPower() const override { return _parametricRimFresnelPower; }

    void setParametricRimLift(float parametricRimLift);
    float getParametricRimLift() const override { return _parametricRimLift; }

    void setRimLightingMix(float rimLightingMix);
    float getRimLightingMix() const override { return _rimLightingMix; }

    void setUVAnimationScrollXSpeed(float uvAnimationScrollXSpeed);
    float getUVAnimationScrollXSpeed() const override { return _uvAnimationScrollXSpeed; }
    void setUVAnimationScrollYSpeed(float uvAnimationScrollYSpeed);
    float getUVAnimationScrollYSpeed() const override { return _uvAnimationScrollYSpeed; }
    void setUVAnimationRotationSpeed(float uvAnimationRotationSpeed);
    float getUVAnimationRotationSpeed() const override { return _uvAnimationRotationSpeed; }

    void setOutlineWidthMode(OutlineWidthMode mode);
    uint8_t getOutlineWidthMode() override { return _outlineWidthMode; }
    void setOutlineWidth(float width);
    float getOutlineWidth() override { return _outlineWidth; }
    void setOutline(const glm::vec3& outline, bool isSRGB = true);
    glm::vec3 getOutline(bool SRGB = true) const override { return (SRGB ? ColorUtils::tosRGBVec3(_outline) : _outline); }

private:
    glm::vec3 _shade { DEFAULT_SHADE };
    float _shadingShift { DEFAULT_SHADING_SHIFT };
    float _shadingToony { DEFAULT_SHADING_TOONY };

    glm::vec3 _matcap { DEFAULT_MATCAP };
    glm::vec3 _parametricRim { DEFAULT_PARAMETRIC_RIM };
    float _parametricRimFresnelPower { DEFAULT_PARAMETRIC_RIM_FRESNEL_POWER };
    float _parametricRimLift { DEFAULT_PARAMETRIC_RIM_LIFT };
    float _rimLightingMix { DEFAULT_RIM_LIGHTING_MIX };

    float _uvAnimationScrollXSpeed { DEFAULT_UV_ANIMATION_SCROLL_SPEED };
    float _uvAnimationScrollYSpeed { DEFAULT_UV_ANIMATION_SCROLL_SPEED };
    float _uvAnimationRotationSpeed { DEFAULT_UV_ANIMATION_SCROLL_SPEED };

    OutlineWidthMode _outlineWidthMode { OutlineWidthMode::OUTLINE_NONE };
    float _outlineWidth { 0.0f };
    glm::vec3 _outline { DEFAULT_OUTLINE };
};

class NetworkMaterialResource : public Resource {
public:
    NetworkMaterialResource() : Resource() {}
    NetworkMaterialResource(const QUrl& url);

    QString getType() const override { return "NetworkMaterial"; }

    virtual void downloadFinished(const QByteArray& data) override;

    typedef struct ParsedMaterials {
        uint version { 1 };
        std::vector<std::string> names;
        std::unordered_map<std::string, std::shared_ptr<NetworkMaterial>> networkMaterials;

        void reset() {
            version = 1;
            names.clear();
            networkMaterials.clear();
        }

    } ParsedMaterials;

    ParsedMaterials parsedMaterials;

    static ParsedMaterials parseJSONMaterials(const QJsonDocument& materialJSON, const QUrl& baseUrl);
    static ParsedMaterials parseMaterialForUUID(const QJsonValue& entityIDJSON);
    static std::pair<std::string, std::shared_ptr<NetworkMaterial>> parseJSONMaterial(const QJsonValue& materialJSONValue, const QUrl& baseUrl = QUrl());

private:
    static bool parseJSONColor(const QJsonValue& array, glm::vec3& color, bool& isSRGB);
};

using NetworkMaterialResourcePointer = QSharedPointer<NetworkMaterialResource>;
using MaterialMapping = std::vector<std::pair<std::string, NetworkMaterialResourcePointer>>;
Q_DECLARE_METATYPE(MaterialMapping)

class MaterialCache : public ResourceCache, public Dependency {
    Q_OBJECT
    SINGLETON_DEPENDENCY

public:
    NetworkMaterialResourcePointer getMaterial(const QUrl& url);

protected:
    virtual QSharedPointer<Resource> createResource(const QUrl& url) override;
    QSharedPointer<Resource> createResourceCopy(const QSharedPointer<Resource>& resource) override;
};

#endif
