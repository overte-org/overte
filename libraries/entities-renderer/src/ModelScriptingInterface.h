//
//  ModelScriptingInterface.h
//  libraries/entities-renderer/src
//
//  Created by Seth Alves on 2017-1-27.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ModelScriptingInterface_h
#define hifi_ModelScriptingInterface_h

#include <memory>

#include <QtCore/QObject>

#include <RegisteredMetaTypes.h>
#include <ScriptValue.h>

class ScriptEngine;
using ScriptEnginePointer = std::shared_ptr<ScriptEngine>;

/*@jsdoc
 * The <code>Model</code> API provides the ability to manipulate meshes. You can get the meshes for an entity using 
 * {@link Entities.getMeshes}, or create a new mesh using {@link Model.newMesh}.
 * <p>See also, the {@link Graphics} API.</p>
 *
 * @namespace Model
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 * @hifi-server-entity
 * @hifi-assignment-client
 *
 * @deprecated This API is deprecated. Use the {@link Graphics} API instead.
 */
/// Provides the <code><a href="https://apidocs.overte.org/Model.html">Model</a></code> scripting interface
class ModelScriptingInterface : public QObject {
    Q_OBJECT

public:
    ModelScriptingInterface(QObject* parent);

    /*@jsdoc
     * Exports meshes to an OBJ format model.
     * @function Model.meshToOBJ
     * @param {MeshProxy[]} meshes - The meshes to export.
     * @returns {string} The OBJ format representation of the meshes.
     */
    Q_INVOKABLE QString meshToOBJ(MeshProxyList in);

    /*@jsdoc
     * Combines multiple meshes into one.
     * @function Model.appendMeshes
     * @param {MeshProxy[]} meshes - The meshes to combine.
     * @returns {MeshProxy} The combined mesh.
     */
    Q_INVOKABLE ScriptValue appendMeshes(MeshProxyList in);

    /*@jsdoc
     * Transforms the vertices in a mesh.
     * @function Model.transformMesh
     * @param {Mat4} transform - The transform to apply.
     * @param {MeshProxy} mesh - The mesh to apply the transform to.
     * @returns {MeshProxy|boolean} The transformed mesh, if valid. <code>false</code> if an error.
     */
    Q_INVOKABLE ScriptValue transformMesh(glm::mat4 transform, MeshProxy* meshProxy);

    /*@jsdoc
     * Creates a new mesh.
     * @function Model.newMesh
     * @param {Vec3[]} vertices - The vertices in the mesh.
     * @param {Vec3[]} normals - The normals in the mesh.
     * @param {MeshFace[]} faces - The faces in the mesh.
     * @returns {MeshProxy} A new mesh.
     */
    Q_INVOKABLE ScriptValue newMesh(const QVector<glm::vec3>& vertices,
                                     const QVector<glm::vec3>& normals,
                                     const QVector<MeshFace>& faces);

    /*@jsdoc
     * Gets the number of vertices in a mesh.
     * @function Model.getVertexCount
     * @param {MeshProxy} mesh - The mesh to count the vertices in.
     * @returns {number|boolean} The number of vertices in the mesh, if valid. <code>false</code> if an error.
     */
    Q_INVOKABLE ScriptValue getVertexCount(MeshProxy* meshProxy);

    /*@jsdoc
     * Gets the position of a vertex in a mesh.
     * @function Model.getVertex
     * @param {MeshProxy} mesh - The mesh.
     * @param {number} index - The index of the vertex to get.
     * @returns {Vec3|boolean} The local position of the vertex relative to the mesh, if valid. <code>false</code> if an error.
     */
    Q_INVOKABLE ScriptValue getVertex(MeshProxy* meshProxy, int vertexIndex);

private:
    ScriptEnginePointer _modelScriptEngine;
};

#endif // hifi_ModelScriptingInterface_h

/// @}
