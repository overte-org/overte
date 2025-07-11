//
//  Copyright 2013-2019 High Fidelity, Inc.
//  Copyright 2019-2021 Vircadia contributors
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QVariant>
#include <QtCore/QUuid>
#include <QPointer>
#include <memory>
#include <unordered_map>

#include <DependencyManager.h>
#include <SpatiallyNestable.h>

#include "graphics/Material.h"
#include "graphics/TextureMap.h"

namespace graphics {
    class Mesh;
}
class Model;
using ModelPointer = std::shared_ptr<Model>;
namespace gpu {
    class BufferView;
}

namespace scriptable {
    using Mesh = graphics::Mesh;
    using MeshPointer = std::shared_ptr<scriptable::Mesh>;
    using WeakMeshPointer = std::weak_ptr<scriptable::Mesh>;

    class ScriptableModelBase;
    using ScriptableModelBasePointer = QPointer<ScriptableModelBase>;

    class ModelProvider;
    using ModelProviderPointer = std::shared_ptr<scriptable::ModelProvider>;
    using WeakModelProviderPointer = std::weak_ptr<scriptable::ModelProvider>;

    class ScriptableMaterial {
    public:
        ScriptableMaterial() {}
        ScriptableMaterial(const graphics::MaterialPointer& material);
        ScriptableMaterial(const ScriptableMaterial& material) { *this = material; }
        ScriptableMaterial& operator=(const ScriptableMaterial& material);

        QString name { "" };
        QString model { "" };
        float opacity { 0.0f };
        float roughness { 0.0f };
        float metallic { 0.0f };
        float scattering { 0.0f };
        bool unlit { false };
        glm::vec3 emissive { 0.0f };
        glm::vec3 albedo { 0.0f };
        QString emissiveMap { "" };
        QString albedoMap { "" };
        QString opacityMap { "" };
        QString opacityMapMode { "" };
        float opacityCutoff { 0.0f };
        QString metallicMap { "" };
        QString specularMap { "" };
        QString roughnessMap { "" };
        QString glossMap { "" };
        QString normalMap { "" };
        QString bumpMap { "" };
        QString occlusionMap { "" };
        QString lightMap { "" };
        QString scatteringMap { "" };
        std::array<glm::mat4, graphics::Material::NUM_TEXCOORD_TRANSFORMS> texCoordTransforms;
        QString cullFaceMode { "" };
        uint8_t layers { 1 };
        QString splatMap { "" };
        bool defaultFallthrough { false };
        std::unordered_map<uint, bool> propertyFallthroughs; // not actually exposed to script

        QString procedural { "" };

        glm::vec3 shade { 0.0f };
        QString shadeMap { "" };
        float shadingShift { 0.0f };
        QString shadingShiftMap { "" };
        float shadingToony { 0.0f };
        glm::vec3 matcap { 0.0f };
        QString matcapMap { "" };
        glm::vec3 parametricRim { 0.0f };
        float parametricRimFresnelPower { 0.0f };
        float parametricRimLift { 0.0f };
        QString rimMap { "" };
        float rimLightingMix { 0.0f };
        QString outlineWidthMode { "" };
        float outlineWidth { 0.0f };
        glm::vec3 outline { 0.0f };
        QString uvAnimationMaskMap { "" };
        float uvAnimationScrollXSpeed { 0.0f };
        float uvAnimationScrollYSpeed { 0.0f };
        float uvAnimationRotationSpeed { 0.0f };

        graphics::MaterialKey key { 0 };
    };

    /*@jsdoc
     * A material layer.
     * @typedef {object} Graphics.MaterialLayer
     * @property {Entities.Material} material - The layer's material.
     * @property {number} priority - The priority of the layer. If multiple materials are applied to a mesh part, only the 
     *     layer with the highest priority is applied, with materials of the same priority randomly assigned.
     */
    class ScriptableMaterialLayer {
    public:
        ScriptableMaterialLayer() {}
        ScriptableMaterialLayer(const graphics::MaterialLayer& materialLayer) : material(materialLayer.material), priority(materialLayer.priority) {}
        ScriptableMaterialLayer(const ScriptableMaterialLayer& materialLayer) { *this = materialLayer; }
        ScriptableMaterialLayer& operator=(const ScriptableMaterialLayer& materialLayer);

        ScriptableMaterial material;
        quint16 priority;
    };
    typedef QHash<QString, QVector<scriptable::ScriptableMaterialLayer>> MultiMaterialMap;

    class ScriptableMeshBase : public QObject {
        Q_OBJECT
    public:
        WeakModelProviderPointer provider;
        ScriptableModelBasePointer model;
        WeakMeshPointer weakMesh;
        MeshPointer strongMesh;
        ScriptableMeshBase(WeakModelProviderPointer provider, ScriptableModelBasePointer model, WeakMeshPointer weakMesh, QObject* parent);
        ScriptableMeshBase(WeakMeshPointer weakMesh = WeakMeshPointer(), QObject* parent = nullptr);
        ScriptableMeshBase(const ScriptableMeshBase& other, QObject* parent = nullptr) : QObject(parent) { *this = other; }
        ScriptableMeshBase& operator=(const ScriptableMeshBase& view);
        virtual ~ScriptableMeshBase();

        /*@jsdoc
         * @function GraphicsMesh.getMeshPointer
         * @deprecated This method is deprecated and will be removed.
         * @returns {undefined}
         */
        // scriptable::MeshPointer is not registered as a JavaScript type.
        Q_INVOKABLE const scriptable::MeshPointer getMeshPointer() const { return weakMesh.lock(); }

        /*@jsdoc
         * @function GraphicsMesh.getModelProviderPointer
         * @deprecated This method is deprecated and will be removed.
         * @returns {undefined}
         */
        // scriptable::ModelProviderPointer is not registered as a JavaScript type.
        Q_INVOKABLE const scriptable::ModelProviderPointer getModelProviderPointer() const { return provider.lock(); }

        /*@jsdoc
         * @function GraphicsMesh.getModelBasePointer
         * @deprecated This method is deprecated and will be removed.
         * @returns {undefined}
         */
        // scriptable::ScriptableModelBasePointer is not registered as a JavaScript type.
        Q_INVOKABLE const scriptable::ScriptableModelBasePointer getModelBasePointer() const { return model; }
    };
    
    // abstract container for holding one or more references to mesh pointers
    class ScriptableModelBase : public QObject {
        Q_OBJECT
    public:
        WeakModelProviderPointer provider;
        QUuid objectID; // spatially nestable ID
        QVector<scriptable::ScriptableMeshBase> meshes;
        MultiMaterialMap materialLayers;
        QVector<QString> materialNames;

        ScriptableModelBase(QObject* parent = nullptr) : QObject(parent) {}
        ScriptableModelBase(const ScriptableModelBase& other) : QObject(other.parent()) { *this = other; }
        ScriptableModelBase& operator=(const ScriptableModelBase& other);
        virtual ~ScriptableModelBase();

        void append(const ScriptableMeshBase& mesh);
        void append(scriptable::WeakMeshPointer mesh);
        void appendMaterial(const graphics::MaterialLayer& materialLayer, int shapeID, std::string materialName);
        void appendMaterials(const std::unordered_map<std::string, graphics::MultiMaterial>& materialsToAppend);
        void appendMaterialNames(const std::vector<std::string>& names);
        // TODO: in future containers for these could go here
        // QVariantMap shapes;
        // QVariantMap armature;
    };

    // mixin class for Avatar + Entity Rendering that expose their in-memory graphics::Meshes
    class ModelProvider {
    public:
        NestableType modelProviderType;
        virtual scriptable::ScriptableModelBase getScriptableModel() = 0;
        virtual bool canReplaceModelMeshPart(int meshIndex, int partIndex) { return false; }
        virtual bool replaceScriptableModelMeshPart(scriptable::ScriptableModelBasePointer model, int meshIndex, int partIndex) { return false; }
    };

    // mixin class for resolving UUIDs into a corresponding ModelProvider
    class ModelProviderFactory : public QObject, public Dependency {
        Q_OBJECT
    public:
        virtual scriptable::ModelProviderPointer lookupModelProvider(const QUuid& uuid) = 0;
    signals:
        void modelAddedToScene(const QUuid& objectID, NestableType nestableType, const ModelPointer& sender);
        void modelRemovedFromScene(const QUuid& objectID, NestableType nestableType, const ModelPointer& sender);
    };

    class ScriptableModel;
    using ScriptableModelPointer = QPointer<ScriptableModel>;
    class ScriptableMesh;
    using ScriptableMeshPointer = QPointer<ScriptableMesh>;
    class ScriptableMeshPart;
    using ScriptableMeshPartPointer = QPointer<ScriptableMeshPart>;
}
