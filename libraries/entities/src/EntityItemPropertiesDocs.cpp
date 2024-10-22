//
//  EntityItemPropertiesDocs.cpp
//  libraries/entities/src
//
//  Created by HifiExperiments on 7/24/24.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/*@jsdoc
 * Different entity types have different properties: some common to all entities (listed in the table) and some specific to
 * each {@link Entities.EntityType|EntityType} (linked to below).
 *
 * @typedef {object} Entities.EntityProperties
 * @property {Uuid} id - The ID of the entity. <em>Read-only.</em>
 * @property {string} name="" - A name for the entity. Need not be unique.
 * @property {Entities.EntityType} type - The entity's type. You cannot change the type of an entity after it's created.
 *     However, its value may switch among <code>"Box"</code>, <code>"Shape"</code>, and <code>"Sphere"</code> depending on
 *     changes to the <code>shape</code> property set for entities of these types. <em>Read-only.</em>
 *
 * @property {Entities.EntityHostType} entityHostType="domain" - How the entity is hosted and sent to others for display.
 *     The value can only be set at entity creation by one of the {@link Entities.addEntity} methods. <em>Read-only.</em>
 * @property {boolean} avatarEntity=false - <code>true</code> if the entity is an {@link Entities.EntityHostType|avatar entity},
 *     <code>false</code> if it isn't. The value is per the <code>entityHostType</code> property value, set at entity creation
 *     by one of the {@link Entities.addEntity} methods. <em>Read-only.</em>
 * @property {boolean} clientOnly=false - A synonym for <code>avatarEntity</code>. <em>Read-only.</em>
 * @property {boolean} localEntity=false - <code>true</code> if the entity is a {@link Entities.EntityHostType|local entity},
 *     <code>false</code> if it isn't. The value is per the <code>entityHostType</code> property value, set at entity creation
 *     by one of the {@link Entities.addEntity} methods. <em>Read-only.</em>
 *
 * @property {Uuid} owningAvatarID=Uuid.NONE - The session ID of the owning avatar if <code>avatarEntity</code> is
 *     <code>true</code>, otherwise {@link Uuid(0)|Uuid.NONE}. <em>Read-only.</em>
 *
 * @property {number} created - When the entity was created, expressed as the number of microseconds since
 *     1970-01-01T00:00:00 UTC. <em>Read-only.</em>
 * @property {number} age - The age of the entity in seconds since it was created. <em>Read-only.</em>
 * @property {string} ageAsText - The age of the entity since it was created, formatted as <code>h hours m minutes s
 *     seconds</code>.
 * @property {number} lifetime=-1 - How long an entity lives for, in seconds, before being automatically deleted. A value of
 *     <code>-1</code> means that the entity lives for ever.
 * @property {number} lastEdited - When the entity was last edited, expressed as the number of microseconds since
 *     1970-01-01T00:00:00 UTC. <em>Read-only.</em>
 * @property {Uuid} lastEditedBy - The session ID of the avatar or agent that most recently created or edited the entity.
 *     <em>Read-only.</em>
 *
 * @property {boolean} locked=false - <code>true</code> if properties other than <code>locked</code> cannot be changed and the
 *     entity cannot be deleted, <code>false</code> if all properties can be changed and the entity can be deleted.
 * @property {boolean} visible=true - <code>true</code> if the entity is rendered, <code>false</code> if it isn't.
 * @property {boolean} canCastShadow=true - <code>true</code> if the entity can cast a shadow, <code>false</code> if it can't.
 *     Currently applicable only to {@link Entities.EntityProperties-Model|Model} and
 *     {@link Entities.EntityProperties-Shape|Shape} entities. Shadows are cast if inside a
 *     {@link Entities.EntityProperties-Zone|Zone} entity with <code>castShadows</code> enabled in its <code>keyLight</code>
 *     property.
 * @property {boolean} isVisibleInSecondaryCamera=true - <code>true</code> if the entity is rendered in the secondary camera,
 *     <code>false</code> if it isn't.
 * @property {Entities.RenderLayer} renderLayer="world" - The layer that the entity renders in.
 * @property {Entities.PrimitiveMode} primitiveMode="solid" - How the entity's geometry is rendered.
 * @property {boolean} ignorePickIntersection=false - <code>true</code> if {@link Picks} and {@link RayPick} ignore the entity,
 *     <code>false</code> if they don't.
 *
 * @property {Vec3} position=0,0,0 - The position of the entity in world coordinates.
 * @property {Quat} rotation=0,0,0,1 - The orientation of the entity in world coordinates.
 * @property {Vec3} registrationPoint=0.5,0.5,0.5 - The point in the entity that is set to the entity's position and is rotated
 *      about, range {@link Vec3(0)|Vec3.ZERO} &ndash; {@link Vec3(0)|Vec3.ONE}. A value of {@link Vec3(0)|Vec3.ZERO} is the
 *      entity's minimum x, y, z corner; a value of {@link Vec3(0)|Vec3.ONE} is the entity's maximum x, y, z corner.
 *
 * @property {Vec3} naturalPosition=0,0,0 - The center of the entity's unscaled mesh model if it has one, otherwise
 *     {@link Vec3(0)|Vec3.ZERO}. <em>Read-only.</em>
 * @property {Vec3} naturalDimensions - The dimensions of the entity's unscaled mesh model or image if it has one, otherwise
 *     {@link Vec3(0)|Vec3.ONE}. <em>Read-only.</em>
 *
 * @property {Vec3} velocity=0,0,0 - The linear velocity of the entity in m/s with respect to world coordinates.
 * @property {number} damping=0.39347 - How much the linear velocity of an entity slows down over time, range
 *     <code>0.0</code> &ndash; <code>1.0</code>. A higher damping value slows down the entity more quickly. The default value
 *     is for an exponential decay timescale of 2.0s, where it takes 2.0s for the movement to slow to <code>1/e = 0.368</code>
 *     of its initial value.
 * @property {Vec3} angularVelocity=0,0,0 - The angular velocity of the entity in rad/s with respect to its axes, about its
 *     registration point.
 * @property {number} angularDamping=0.39347 - How much the angular velocity of an entity slows down over time, range
 *     <code>0.0</code> &ndash; <code>1.0</code>. A higher damping value slows down the entity more quickly. The default value
 *     is for an exponential decay timescale of 2.0s, where it takes 2.0s for the movement to slow to <code>1/e = 0.368</code>
 *     of its initial value.
 *
 * @property {Vec3} gravity=0,0,0 - The acceleration due to gravity in m/s<sup>2</sup> that the entity should move with, in
 *     world coordinates. Use a value of <code>{ x: 0, y: -9.8, z: 0 }</code> to simulate Earth's gravity. Gravity is applied
 *     to an entity's motion only if its <code>dynamic</code> property is <code>true</code>.
 *     <p>If changing an entity's <code>gravity</code> from {@link Vec3(0)|Vec3.ZERO}, you need to give it a small
 *     <code>velocity</code> in order to kick off physics simulation.</p>
 * @property {Vec3} acceleration - The current, measured acceleration of the entity, in m/s<sup>2</sup>.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {number} restitution=0.5 - The "bounciness" of an entity when it collides, range <code>0.0</code> &ndash;
 *     <code>0.99</code>. The higher the value, the more bouncy.
 * @property {number} friction=0.5 - How much an entity slows down when it's moving against another, range <code>0.0</code>
 *     &ndash; <code>10.0</code>. The higher the value, the more quickly it slows down. Examples: <code>0.1</code> for ice,
 *     <code>0.9</code> for sandpaper.
 * @property {number} density=1000 - The density of the entity in kg/m<sup>3</sup>, range <code>100</code> &ndash;
 *     <code>10000</code>. Examples: <code>100</code> for balsa wood, <code>10000</code> for silver. The density is used in
 *     conjunction with the entity's bounding box volume to work out its mass in the application of physics.
 *
 * @property {boolean} collisionless=false - <code>true</code> if the entity shouldn't collide, <code>false</code> if it
 *     collides with items per its <code>collisionMask</code> property.
 * @property {boolean} ignoreForCollisions - Synonym for <code>collisionless</code>.
 * @property {CollisionMask} collisionMask=31 - What types of items the entity should collide with.
 * @property {string} collidesWith="static,dynamic,kinematic,myAvatar,otherAvatar," - Synonym for <code>collisionMask</code>,
 *     in text format.
 * @property {string} collisionSoundURL="" - The sound that's played when the entity experiences a collision. Valid file
 *     formats are per {@link SoundObject}.
 * @property {boolean} dynamic=false - <code>true</code> if the entity's movement is affected by collisions, <code>false</code>
 *     if it isn't.
 * @property {boolean} collisionsWillMove - A synonym for <code>dynamic</code>.
 *
 * @property {string} href="" - A "hifi://" directory services address that a user is teleported to when they click on the entity.
 * @property {string} description="" - A description of the <code>href</code> property value.
 *
 * @property {string} userData="" - Used to store extra data about the entity in JSON format.
 *     <p><strong>Warning:</strong> Other apps may also use this property, so make sure you handle data stored by other apps:
 *     edit only your bit and leave the rest of the data intact. You can use <code>JSON.parse()</code> to parse the string into
 *     a JavaScript object which you can manipulate the properties of, and use <code>JSON.stringify()</code> to convert the
 *     object into a string to put back in the property.</p>
 *
 * @property {string} privateUserData="" - Like <code>userData</code>, but only accessible by server entity scripts, assignment
 *     client scripts, and users who have "Can Get and Set Private User Data" permissions in the domain.
 *
 * @property {string} script="" - The URL of the client entity script, if any, that is attached to the entity.
 * @property {number} scriptTimestamp=0 - Used to indicate when the client entity script was loaded. Should be
 *     an integer number of milliseconds since midnight GMT on January 1, 1970 (e.g., as supplied by <code>Date.now()</code>.
 *     If you update the property's value, the <code>script</code> is re-downloaded and reloaded. This is how the "reload"
 *     button beside the "script URL" field in properties tab of the Create app works.
 * @property {string} serverScripts="" - The URL of the server entity script, if any, that is attached to the entity.
 *
 * @property {Uuid} parentID=Uuid.NONE - The ID of the entity or avatar that the entity is parented to. A value of
 *     {@link Uuid(0)|Uuid.NONE} is used if the entity is not parented.
 * @property {number} parentJointIndex=65535 - The joint of the entity or avatar that the entity is parented to. Use
 *     <code>65535</code> or <code>-1</code> to parent to the entity or avatar's position and orientation rather than a joint.
 * @property {Vec3} localPosition=0,0,0 - The position of the entity relative to its parent if the entity is parented,
 *     otherwise the same value as <code>position</code>. If the entity is parented to an avatar and is an avatar entity
 *     so that it scales with the avatar, this value remains the original local position value while the avatar scale changes.
 * @property {Quat} localRotation=0,0,0,1 - The rotation of the entity relative to its parent if the entity is parented,
 *     otherwise the same value as <code>rotation</code>.
 * @property {Vec3} localVelocity=0,0,0 - The velocity of the entity relative to its parent if the entity is parented,
 *     otherwise the same value as <code>velocity</code>.
 * @property {Vec3} localAngularVelocity=0,0,0 - The angular velocity of the entity relative to its parent if the entity is
 *     parented, otherwise the same value as <code>angularVelocity</code>.
 * @property {Vec3} localDimensions - The dimensions of the entity. If the entity is parented to an avatar and is an
 *     avatar entity so that it scales with the avatar, this value remains the original dimensions value while the
 *     avatar scale changes.
 *
 * @property {Entities.BoundingBox} boundingBox - The axis-aligned bounding box that tightly encloses the entity.
 *     <em>Read-only.</em>
 * @property {AACube} queryAACube - The axis-aligned cube that determines where the entity lives in the entity server's octree.
 *     The cube may be considerably larger than the entity in some situations, e.g., when the entity is grabbed by an avatar:
 *     the position of the entity is determined through avatar mixer updates and so the AA cube is expanded in order to reduce
 *     unnecessary entity server updates. Scripts should not change this property's value.
 *
 * @property {string} actionData="" - Base-64 encoded compressed dump of the actions associated with the entity. This property
 *     is typically not used in scripts directly; rather, functions that manipulate an entity's actions update it, e.g.,
 *     {@link Entities.addAction}. The size of this property increases with the number of actions. Because this property value
 *     has to fit within a Overte datagram packet, there is a limit to the number of actions that an entity can have;
 *     edits which would result in overflow are rejected. <em>Read-only.</em>
 * @property {Entities.RenderInfo} renderInfo - Information on the cost of rendering the entity. Currently information is only
 *     provided for <code>Model</code> entities. <em>Read-only.</em>
 *
 * @property {boolean} cloneable=false - <code>true</code> if the domain or avatar entity can be cloned via
 *     {@link Entities.cloneEntity}, <code>false</code> if it can't be.
 * @property {number} cloneLifetime=300 - The entity lifetime for clones created from this entity.
 * @property {number} cloneLimit=0 - The total number of clones of this entity that can exist in the domain at any given time.
 * @property {boolean} cloneDynamic=false - <code>true</code> if clones created from this entity will have their
 *     <code>dynamic</code> property set to <code>true</code>, <code>false</code> if they won't.
 * @property {boolean} cloneAvatarEntity=false - <code>true</code> if clones created from this entity will be created as
 *     avatar entities, <code>false</code> if they won't be.
 * @property {Uuid} cloneOriginID - The ID of the entity that this entity was cloned from.
 *
 * @property {Uuid[]} renderWithZones=[] - A list of entity IDs representing with which zones this entity should render.
 *     If it is empty, this entity will render normally.  Otherwise, this entity will only render if your avatar is within
 *     one of the zones in this list.
 * @property {BillboardMode} billboardMode="none" - Whether the entity is billboarded to face the camera.  Use the rotation
 *     property to control which axis is facing you.
 * @property {string[]} tags=[] - A set of tags describing this entity.
 *
 * @property {Entities.Grab} grab - The entity's grab-related properties.
 *
 * @property {MirrorMode} mirrorMode="none" - If this entity should render as a mirror (reflecting the view of the camera),
 *     a portal (reflecting the view through its <code>portalExitID</code>), or normally.
 * @property {Uuid} portalExitID=Uuid.NONE - The ID of the entity that should act as the portal exit if the <code>mirrorMode</code>
 *     is set to <code>portal</code>.
 *
 * @comment The different entity types have additional properties as follows:
 * @see {@link Entities.EntityProperties-Box|EntityProperties-Box}
 * @see {@link Entities.EntityProperties-Gizmo|EntityProperties-Gizmo}
 * @see {@link Entities.EntityProperties-Grid|EntityProperties-Grid}
 * @see {@link Entities.EntityProperties-Image|EntityProperties-Image}
 * @see {@link Entities.EntityProperties-Light|EntityProperties-Light}
 * @see {@link Entities.EntityProperties-Line|EntityProperties-Line}
 * @see {@link Entities.EntityProperties-Material|EntityProperties-Material}
 * @see {@link Entities.EntityProperties-Model|EntityProperties-Model}
 * @see {@link Entities.EntityProperties-ParticleEffect|EntityProperties-ParticleEffect}
 * @see {@link Entities.EntityProperties-PolyLine|EntityProperties-PolyLine}
 * @see {@link Entities.EntityProperties-PolyVox|EntityProperties-PolyVox}
 * @see {@link Entities.EntityProperties-ProceduralParticleEffect|EntityProperties-ProceduralParticleEffect}
 * @see {@link Entities.EntityProperties-Shape|EntityProperties-Shape}
 * @see {@link Entities.EntityProperties-Sound|EntityProperties-Sound}
 * @see {@link Entities.EntityProperties-Sphere|EntityProperties-Sphere}
 * @see {@link Entities.EntityProperties-Text|EntityProperties-Text}
 * @see {@link Entities.EntityProperties-Web|EntityProperties-Web}
 * @see {@link Entities.EntityProperties-Zone|EntityProperties-Zone}
 */

/*@jsdoc
 * The <code>"Box"</code> {@link Entities.EntityType|EntityType} is the same as the <code>"Shape"</code>
 * {@link Entities.EntityType|EntityType} except that its <code>shape</code> value is always set to <code>"Cube"</code>
 * when the entity is created. If its <code>shape</code> property value is subsequently changed then the entity's
 * <code>type</code> will be reported as <code>"Sphere"</code> if the <code>shape</code> is set to <code>"Sphere"</code>,
 * otherwise it will be reported as <code>"Shape"</code>.
 *
 * @typedef {object} Entities.EntityProperties-Box
 * @see {@link Entities.EntityProperties-Shape|EntityProperties-Shape}
 */

/*@jsdoc
 * The <code>"Light"</code> {@link Entities.EntityType|EntityType} adds local lighting effects. It has properties in addition
 * to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Light
 * @property {Vec3} dimensions=0.1,0.1,0.1 - The dimensions of the entity. Surfaces outside these dimensions are not lit
 *     by the light.
 * @property {Color} color=255,255,255 - The color of the light emitted.
 * @property {number} intensity=1 - The brightness of the light.
 * @property {number} falloffRadius=0.1 - The distance from the light's center at which intensity is reduced by 25%.
 * @property {boolean} isSpotlight=false - <code>true</code> if the light is directional, emitting along the entity's
 *     local negative z-axis; <code>false</code> if the light is a point light which emanates in all directions.
 * @property {number} exponent=0 - Affects the softness of the spotlight beam: the higher the value the softer the beam.
 * @property {number} cutoff=1.57 - Affects the size of the spotlight beam: the higher the value the larger the beam.
 * @example <caption>Create a spotlight pointing at the ground.</caption>
 * Entities.addEntity({
 *     type: "Light",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.5, z: -4 })),
 *     rotation: Quat.fromPitchYawRollDegrees(-75, 0, 0),
 *     dimensions: { x: 5, y: 5, z: 5 },
 *     intensity: 100,
 *     falloffRadius: 0.3,
 *     isSpotlight: true,
 *     exponent: 20,
 *     cutoff: 30,
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Line"</code> {@link Entities.EntityType|EntityType} draws thin, straight lines between a sequence of two or more
 * points. It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 * <p class=important>Deprecated: Use {@link Entities.EntityProperties-PolyLine|PolyLine} entities instead.</p>
 *
 * @typedef {object} Entities.EntityProperties-Line
 * @property {Vec3} dimensions=0.1,0.1,0.1 - The dimensions of the entity. Must be sufficient to contain all the
 *     <code>linePoints</code>.
 * @property {Vec3[]} linePoints=[]] - The sequence of points to draw lines between. The values are relative to the entity's
 *     position. A maximum of 70 points can be specified. The property's value is set only if all the <code>linePoints</code>
 *     lie within the entity's <code>dimensions</code>.
 * @property {Color} color=255,255,255 - The color of the line.
 * @example <caption>Draw lines in a "V".</caption>
 * var entity = Entities.addEntity({
 *     type: "Line",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.75, z: -5 })),
 *     rotation: MyAvatar.orientation,
 *     dimensions: { x: 2, y: 2, z: 1 },
 *     linePoints: [
 *         { x: -1, y: 1, z: 0 },
 *         { x: 0, y: -1, z: 0 },
 *         { x: 1, y: 1, z: 0 },
 *     ],
 *     color: { red: 255, green: 0, blue: 0 },
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Material"</code> {@link Entities.EntityType|EntityType} modifies existing materials on entities and avatars. It
 * has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 * <p>To apply a material to an entity, set the material entity's <code>parentID</code> property to the entity ID.
 * To apply a material to an avatar, set the material entity's <code>parentID</code> property to the avatar's session UUID.
 * To apply a material to your avatar such that it persists across domains and log-ins, create the material as an avatar entity
 * by setting the <code>entityHostType</code> parameter in {@link Entities.addEntity} to <code>"avatar"</code> and set the
 * entity's <code>parentID</code> property to <code>MyAvatar.SELF_ID</code>.
 * Material entities render as non-scalable spheres if they don't have their parent set.</p>
 *
 * @typedef {object} Entities.EntityProperties-Material
 * @property {Vec3} dimensions=0.1,0.1,0.1 - Used when <code>materialMappingMode == "projected"</code>.
 * @property {string} materialURL="" - URL to a {@link Entities.MaterialResource|MaterialResource}. Alternatively, set the
 *     property value to <code>"materialData"</code> to use the <code>materialData</code> property for the
 *     {@link Entities.MaterialResource|MaterialResource} values. If you append <code>"#name"</code> to the URL, the material
 *     with that name will be applied to the entity. You can also use the ID of another Material entity as the URL, in which
 *     case this material will act as a copy of that material, with its own unique material transform, priority, etc.
 * @property {string} materialData="" - Used to store {@link Entities.MaterialResource|MaterialResource} data as a JSON string.
 *     You can use <code>JSON.parse()</code> to parse the string into a JavaScript object which you can manipulate the
 *     properties of, and use <code>JSON.stringify()</code> to convert the object into a string to put in the property.
 * @property {number} priority=0 - The priority for applying the material to its parent. Only the highest priority material is
 *     applied, with materials of the same priority randomly assigned. Materials that come with the model have a priority of
 *     <code>0</code>.
 * @property {string} parentMaterialName="0" - Selects the mesh part or parts within the parent to which to apply the material.
 *     If in the format <code>"mat::string"</code>, all mesh parts with material name <code>"string"</code> are replaced.
 *     If <code>"all"</code>, then all mesh parts are replaced.
 *     Otherwise the property value is parsed as an unsigned integer, specifying the mesh part index to modify.
 *     <p>If the string represents an array (starts with <code>"["</code> and ends with <code>"]"</code>), the string is split
 *     at each <code>","</code> and each element parsed as either a number or a string if it starts with <code>"mat::"</code>.
 *     For example, <code>"[0,1,mat::string,mat::string2]"</code> will replace mesh parts 0 and 1, and any mesh parts with
 *     material <code>"string"</code> or <code>"string2"</code>. Do not put spaces around the commas. Invalid values are parsed
 *     to <code>0</code>.</p>
 * @property {string} materialMappingMode="uv" - How the material is mapped to the entity. Either <code>"uv"</code> or
 *     <code>"projected"</code>. In <code>"uv"</code> mode, the material is evaluated within the UV space of the mesh it is
 *     applied to. In <code>"projected"</code> mode, the 3D transform (position, rotation, and dimensions) of the Material
 *     entity is used to evaluate the texture coordinates for the material.
 * @property {Vec2} materialMappingPos=0,0 - Offset position in UV-space of the top left of the material, range
 *     <code>{ x: 0, y: 0 }</code> &ndash; <code>{ x: 1, y: 1 }</code>.
 * @property {Vec2} materialMappingScale=1,1 - How much to scale the material within the parent's UV-space.
 * @property {number} materialMappingRot=0 - How much to rotate the material within the parent's UV-space, in degrees.
 * @property {boolean} materialRepeat=true - <code>true</code> if the material repeats, <code>false</code> if it doesn't. If
 *     <code>false</code>, fragments outside of texCoord 0 &ndash; 1 will be discarded. Works in both <code>"uv"</code> and
 *     <code>"projected"</code> modes.
 * @example <caption>Color a sphere using a Material entity.</caption>
 * var entityID = Entities.addEntity({
 *     type: "Sphere",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -5 })),
 *     dimensions: { x: 1, y: 1, z: 1 },
 *     color: { red: 128, green: 128, blue: 128 },
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 *
 * var materialID = Entities.addEntity({
 *     type: "Material",
 *     parentID: entityID,
 *     materialURL: "materialData",
 *     priority: 1,
 *     materialData: JSON.stringify({
 *         materialVersion: 1,
 *         materials: {
 *             // Value overrides entity's "color" property.
 *             albedo: [1.0, 1.0, 0]  // Yellow
 *         }
 *     })
 * });
 */

/*@jsdoc
 * The <code>"Model"</code> {@link Entities.EntityType|EntityType} displays a glTF, FBX, or OBJ model. When adding an entity,
 * if no <code>dimensions</code> value is specified then the model is automatically sized to its
 * <code>{@link Entities.EntityProperties|naturalDimensions}</code>. It has properties in addition to the common
 * {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Model
 * @property {Vec3} dimensions=0.1,0.1,0.1 - The dimensions of the entity. When adding an entity, if no <code>dimensions</code>
 *     value is specified then the model is automatically sized to its
 *     <code>{@link Entities.EntityProperties|naturalDimensions}</code>.
 * @property {string} modelURL="" - The URL of the glTF, FBX, or OBJ model. glTF models may be in JSON or binary format
 *     (".gltf" or ".glb" URLs respectively). Baked models' URLs have ".baked" before the file type. Model files may also be
 *     compressed in GZ format, in which case the URL ends in ".gz".
 * @property {Vec3} modelScale - The scale factor applied to the model's dimensions.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {string} blendshapeCoefficients - A JSON string of a map of blendshape names to values.  Only stores set values.
 *     When editing this property, only coefficients that you are editing will change; it will not explicitly reset other
 *     coefficients.
 * @property {boolean} useOriginalPivot=false - If <code>false</code>, the model will be centered based on its content,
 *     ignoring any offset in the model itself. If <code>true</code>, the model will respect its original offset.  Currently,
 *     only pivots relative to <code>{x: 0, y: 0, z: 0}</code> are supported.
 * @property {number} loadPriority=0.0 - If <code>0</code>, the model download will be prioritized based on distance, size, and
 *     other factors, and assigned a priority automatically between <code>0</code> and <code>PI / 2</code>.  Otherwise, the
 *     download will be ordered based on the set <code>loadPriority</code>.
 * @property {string} textures="" - A JSON string of texture name, URL pairs used when rendering the model in place of the
 *     model's original textures. Use a texture name from the <code>originalTextures</code> property to override that texture.
 *     Only the texture names and URLs to be overridden need be specified; original textures are used where there are no
 *     overrides. You can use <code>JSON.stringify()</code> to convert a JavaScript object of name, URL pairs into a JSON
 *     string.
 * @property {string} originalTextures="{}" - A JSON string of texture name, URL pairs used in the model. The property value is
 *     filled in after the entity has finished rezzing (i.e., textures have loaded). You can use <code>JSON.parse()</code> to
 *     parse the JSON string into a JavaScript object of name, URL pairs. <em>Read-only.</em>
 * @property {Color} color=255,255,255 - <em>Currently not used.</em>
 *
 * @property {ShapeType} shapeType="none" - The shape of the collision hull used if collisions are enabled.
 * @property {string} compoundShapeURL="" - The model file to use for the compound shape if <code>shapeType</code> is
 *     <code>"compound"</code>.
 *
 * @property {Entities.AnimationProperties} animation - An animation to play on the model.
 *
 * @property {Quat[]} jointRotations=[]] - Joint rotations applied to the model; <code>[]</code> if none are applied or the
 *     model hasn't loaded. The array indexes are per {@link Entities.getJointIndex|getJointIndex}. Rotations are relative to
 *     each joint's parent.
 *     <p>Joint rotations can be set by {@link Entities.setLocalJointRotation|setLocalJointRotation} and similar functions, or
 *     by setting the value of this property. If you set a joint rotation using this property, you also need to set the
 *     corresponding <code>jointRotationsSet</code> value to <code>true</code>.</p>
 * @property {boolean[]} jointRotationsSet=[]] - <code>true</code> values for joints that have had rotations applied,
 *     <code>false</code> otherwise; <code>[]</code> if none are applied or the model hasn't loaded. The array indexes are per
 *     {@link Entities.getJointIndex|getJointIndex}.
 * @property {Vec3[]} jointTranslations=[]] - Joint translations applied to the model; <code>[]</code> if none are applied or
 *     the model hasn't loaded. The array indexes are per {@link Entities.getJointIndex|getJointIndex}. Translations are
 *     relative to each joint's parent.
 *     <p>Joint translations can be set by {@link Entities.setLocalJointTranslation|setLocalJointTranslation} and similar
 *     functions, or by setting the value of this property. If you set a joint translation using this property you also need to
 *     set the corresponding <code>jointTranslationsSet</code> value to <code>true</code>.</p>
 * @property {boolean[]} jointTranslationsSet=[]] - <code>true</code> values for joints that have had translations applied,
 *     <code>false</code> otherwise; <code>[]</code> if none are applied or the model hasn't loaded. The array indexes are per
 *     {@link Entities.getJointIndex|getJointIndex}.
 * @property {boolean} relayParentJoints=false - <code>true</code> if when the entity is parented to an avatar, the avatar's
 *     joint rotations are applied to the entity's joints; <code>false</code> if a parent avatar's joint rotations are not
 *     applied to the entity's joints.
 * @property {boolean} groupCulled=false - <code>true</code> if the mesh parts of the model are LOD culled as a group,
 *     <code>false</code> if separate mesh parts are LOD culled individually.
 *
 * @example <caption>Rez a cowboy hat.</caption>
 * var entity = Entities.addEntity({
 *     type: "Model",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.75, z: -2 })),
 *     rotation: MyAvatar.orientation,
 *     modelURL: "https://apidocs.overte.org/examples/cowboy-hat.fbx",
 *     dimensions: { x: 0.8569, y: 0.3960, z: 1.0744 },
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"ParticleEffect"</code> {@link Entities.EntityType|EntityType} displays a particle system that can be used to
 * simulate things such as fire, smoke, snow, magic spells, etc. The particles emanate from an ellipsoid or part thereof.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-ParticleEffect
 * @property {boolean} isEmitting=true - <code>true</code> if particles are being emitted, <code>false</code> if they aren't.
 * @property {number} maxParticles=1000 - The maximum number of particles to render at one time. Older particles are deleted if
 *     necessary when new ones are created.
 * @property {number} lifespan=3s - How long, in seconds, each particle lives.
 * @property {number} emitRate=15 - The number of particles per second to emit.
 * @property {number} emitSpeed=5 - The speed, in m/s, that each particle is emitted at.
 * @property {number} speedSpread=1 - The spread in speeds at which particles are emitted at. For example, if
 *     <code>emitSpeed == 5</code> and <code>speedSpread == 1</code>, particles will be emitted with speeds in the range
 *     <code>4</code> &ndash; <code>6</code>m/s.
 * @property {Vec3} emitAcceleration=0,-9.8,0 - The acceleration that is applied to each particle during its lifetime. The
 *     default is Earth's gravity value.
 * @property {Vec3} accelerationSpread=0,0,0 - The spread in accelerations that each particle is given. For example, if
 *     <code>emitAccelerations == {x: 0, y: -9.8, z: 0}</code> and <code>accelerationSpread ==
 *     {x: 0, y: 1, z: 0}</code>, each particle will have an acceleration in the range <code>{x: 0, y: -10.8, z: 0}</code>
 *     &ndash; <code>{x: 0, y: -8.8, z: 0}</code>.
 * @property {Vec3} dimensions - The dimensions of the particle effect, i.e., a bounding box containing all the particles
 *     during their lifetimes, assuming that <code>emitterShouldTrail == false</code>. <em>Read-only.</em>
 * @property {boolean} emitterShouldTrail=false - <code>true</code> if particles are "left behind" as the emitter moves,
 *     <code>false</code> if they stay within the entity's dimensions.
 *
 * @property {Quat} emitOrientation=-0.707,0,0,0.707 - The orientation of particle emission relative to the entity's axes. By
 *     default, particles emit along the entity's local z-axis, and <code>azimuthStart</code> and <code>azimuthFinish</code>
 *     are relative to the entity's local x-axis. The default value is a rotation of -90 degrees about the local x-axis, i.e.,
 *     the particles emit vertically.
 *
 * @property {ShapeType} shapeType="ellipsoid" - The shape from which particles are emitted.
 * @property {string} compoundShapeURL="" - The model file to use for the compound shape if <code>shapeType ==
 *     "compound"</code>.
 * @property {Vec3} emitDimensions=0,0,0 - The dimensions of the shape from which particles are emitted.
 * @property {number} emitRadiusStart=1 - The starting radius within the shape at which particles start being emitted;
 *     range <code>0.0</code> &ndash; <code>1.0</code> for the center to the surface, respectively.
 *     Particles are emitted from the portion of the shape that lies between <code>emitRadiusStart</code> and the
 *     shape's surface.
 * @property {number} polarStart=0 - The angle in radians from the entity's local z-axis at which particles start being emitted
 *     within the shape; range <code>0</code> &ndash; <code>Math.PI</code>. Particles are emitted from the portion of the
 *     shape that lies between <code>polarStart</code> and <code>polarFinish</code>. Only used if <code>shapeType</code> is
 *     <code>"ellipsoid"</code> or <code>"sphere"</code>.
 * @property {number} polarFinish=0 - The angle in radians from the entity's local z-axis at which particles stop being emitted
 *     within the shape; range <code>0</code> &ndash; <code>Math.PI</code>. Particles are emitted from the portion of the
 *     shape that lies between <code>polarStart</code> and <code>polarFinish</code>. Only used if <code>shapeType</code> is
 *     <code>"ellipsoid"</code> or <code>"sphere"</code>.
 * @property {number} azimuthStart=-Math.PI - The angle in radians from the entity's local x-axis about the entity's local
 *     z-axis at which particles start being emitted; range <code>-Math.PI</code> &ndash; <code>Math.PI</code>. Particles are
 *     emitted from the portion of the shape that lies between <code>azimuthStart</code> and <code>azimuthFinish</code>.
 *     Only used if <code>shapeType</code> is <code>"ellipsoid"</code>, <code>"sphere"</code>, or <code>"circle"</code>.
 * @property {number} azimuthFinish=Math.PI - The angle in radians from the entity's local x-axis about the entity's local
 *     z-axis at which particles stop being emitted; range <code>-Math.PI</code> &ndash; <code>Math.PI</code>. Particles are
 *     emitted from the portion of the shape that lies between <code>azimuthStart</code> and <code>azimuthFinish</code>.
 *     Only used if <code>shapeType</code> is <code>"ellipsoid"</code>, <code>"sphere"</code>, or <code>"circle"</code>.
 *
 * @property {string} textures="" - The URL of a JPG or PNG image file to display for each particle. If you want transparency,
 *     use PNG format.
 * @property {number} particleRadius=0.025 - The radius of each particle at the middle of its life.
 * @property {number} radiusStart=null - The radius of each particle at the start of its life. If <code>null</code>, the
 *     <code>particleRadius</code> value is used.
 * @property {number} radiusFinish=null - The radius of each particle at the end of its life. If <code>null</code>, the
 *     <code>particleRadius</code> value is used.
 * @property {number} radiusSpread=0 - The spread in radius that each particle is given. For example, if
 *     <code>particleRadius == 0.5</code> and <code>radiusSpread == 0.25</code>, each particle will have a radius in the range
 *     <code>0.25</code> &ndash; <code>0.75</code>.
 * @property {Color} color=255,255,255 - The color of each particle at the middle of its life.
 * @property {ColorFloat} colorStart=null,null,null - The color of each particle at the start of its life. If any of the
 *     component values are undefined, the <code>color</code> value is used.
 * @property {ColorFloat} colorFinish=null,null,null - The color of each particle at the end of its life. If any of the
 *     component values are undefined, the <code>color</code> value is used.
 * @property {Color} colorSpread=0,0,0 - The spread in color that each particle is given. For example, if
 *     <code>color == {red: 100, green: 100, blue: 100}</code> and <code>colorSpread ==
 *     {red: 10, green: 25, blue: 50}</code>, each particle will have a color in the range
 *     <code>{red: 90, green: 75, blue: 50}</code> &ndash; <code>{red: 110, green: 125, blue: 150}</code>.
 * @property {number} alpha=1 - The opacity of each particle at the middle of its life.
 * @property {number} alphaStart=null - The opacity of each particle at the start of its life. If <code>null</code>, the
 *     <code>alpha</code> value is used.
 * @property {number} alphaFinish=null - The opacity of each particle at the end of its life. If <code>null</code>, the
 *     <code>alpha</code> value is used.
 * @property {number} alphaSpread=0 - The spread in alpha that each particle is given. For example, if
 *     <code>alpha == 0.5</code> and <code>alphaSpread == 0.25</code>, each particle will have an alpha in the range
 *     <code>0.25</code> &ndash; <code>0.75</code>.
 * @property {Entities.Pulse} pulse - Color and alpha pulse.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {number} particleSpin=0 - The rotation of each particle at the middle of its life, range <code>-2 * Math.PI</code>
 *     &ndash; <code>2 * Math.PI</code> radians.
 * @property {number} spinStart=null - The rotation of each particle at the start of its life, range <code>-2 * Math.PI</code>
 *     &ndash; <code>2 * Math.PI</code> radians. If <code>null</code>, the <code>particleSpin</code> value is used.
 * @property {number} spinFinish=null - The rotation of each particle at the end of its life, range <code>-2 * Math.PI</code>
 *     &ndash; <code>2 * Math.PI</code> radians. If <code>null</code>, the <code>particleSpin</code> value is used.
 * @property {number} spinSpread=0 - The spread in spin that each particle is given, range <code>0</code> &ndash;
 *     <code>2 * Math.PI</code> radians. For example, if <code>particleSpin == Math.PI</code> and
 *     <code>spinSpread == Math.PI / 2</code>, each particle will have a rotation in the range <code>Math.PI / 2</code> &ndash;
 *     <code>3 * Math.PI / 2</code>.
 * @property {boolean} rotateWithEntity=false - <code>true</code> if the particles' rotations are relative to the entity's
 *     instantaneous rotation, <code>false</code> if they're relative to world coordinates. If <code>true</code> with
 *     <code>particleSpin == 0</code>, the particles keep oriented per the entity's orientation.
 *
 * @example <caption>Create a ball of green smoke.</caption>
 * particles = Entities.addEntity({
 *     type: "ParticleEffect",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.5, z: -4 })),
 *     lifespan: 5,
 *     emitRate: 10,
 *     emitSpeed: 0.02,
 *     speedSpread: 0.01,
 *     emitAcceleration: { x: 0, y: 0.02, z: 0 },
 *     polarFinish: Math.PI,
 *     textures: "https://content.overte.org/Bazaar/Assets/Textures/Defaults/Interface/default_particle.png",
 *     particleRadius: 0.1,
 *     color: { red: 0, green: 255, blue: 0 },
 *     alphaFinish: 0,
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"PolyLine"</code> {@link Entities.EntityType|EntityType} draws textured, straight lines between a sequence of
 * points. It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-PolyLine
 * @property {Vec3} dimensions=0.1,0.1,0.1 - The dimensions of the entity, i.e., the size of the bounding box that contains the
 *     lines drawn. <em>Read-only.</em>
 * @property {Vec3[]} linePoints=[]] - The sequence of points to draw lines between. The values are relative to the entity's
 *     position. A maximum of 70 points can be specified.
 * @property {Vec3[]} normals=[]] - The normal vectors for the line's surface at the <code>linePoints</code>. The values are
 *     relative to the entity's orientation. Must be specified in order for the entity to render.
 * @property {number[]} strokeWidths=[]] - The widths, in m, of the line at the <code>linePoints</code>. Must be specified in
 *     order for the entity to render.
 * @property {Vec3[]} strokeColors=[]] - The base colors of each point, with values in the range <code>0.0,0.0,0.0</code>
 *     &ndash; <code>1.0,1.0,1.0</code>. These colors are multiplied with the color of the texture. If there are more line
 *     points than stroke colors, the <code>color</code> property value is used for the remaining points.
 *     <p><strong>Warning:</strong> The ordinate values are in the range <code>0.0</code> &ndash; <code>1.0</code>.</p>
 * @property {Color} color=255,255,255 - Used as the color for each point if <code>strokeColors</code> doesn't have a value for
 *     the point.
 * @property {string} textures="" - The URL of a JPG or PNG texture to use for the lines. If you want transparency, use PNG
 *     format.
 * @property {boolean} isUVModeStretch=true - <code>true</code> if the texture is stretched to fill the whole line,
 *     <code>false</code> if the texture repeats along the line.
 * @property {boolean} glow=false - <code>true</code> if the opacity of the strokes drops off away from the line center,
 *     <code>false</code> if it doesn't.
 * @property {boolean} faceCamera=false - <code>true</code> if each line segment rotates to face the camera, <code>false</code>
 *     if they don't.
 * @example <caption>Draw a textured "V".</caption>
 * var entity = Entities.addEntity({
 *     type: "PolyLine",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.75, z: -5 })),
 *     rotation: MyAvatar.orientation,
 *     linePoints: [
 *         { x: -1, y: 0.5, z: 0 },
 *         { x: 0, y: 0, z: 0 },
 *         { x: 1, y: 0.5, z: 0 }
 *     ],
 *     normals: [
 *         { x: 0, y: 0, z: 1 },
 *         { x: 0, y: 0, z: 1 },
 *         { x: 0, y: 0, z: 1 }
 *     ],
 *     strokeWidths: [ 0.1, 0.1, 0.1 ],
 *     color: { red: 255, green: 0, blue: 0 },  // Use just the red channel from the image.
 *     textures: "https://hifi-content/DomainContent/Toybox/flowArts/trails.png",
 *     isUVModeStretch: true,
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"PolyVox"</code> {@link Entities.EntityType|EntityType} displays a set of textured voxels.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 * If you have two or more neighboring PolyVox entities of the same size abutting each other, you can display them as joined by
 * configuring their <code>voxelSurfaceStyle</code> and various neighbor ID properties.
 * <p>PolyVox entities uses a library from <a href="http://www.volumesoffun.com/">Volumes of Fun</a>. Their
 * <a href="http://www.volumesoffun.com/polyvox-documentation/">library documentation</a> may be useful to read.</p>
 *
 * @typedef {object} Entities.EntityProperties-PolyVox
 * @property {Vec3} dimensions=0.1,0.1,0.1 - The dimensions of the entity.
 * @property {Vec3} voxelVolumeSize=32,32,32 - Integer number of voxels along each axis of the entity, in the range
 *     <code>1,1,1</code> to <code>128,128,128</code>. The dimensions of each voxel is
 *     <code>dimensions / voxelVolumesize</code>.
 * @property {string} voxelData="ABAAEAAQAAAAHgAAEAB42u3BAQ0AAADCoPdPbQ8HFAAAAPBuEAAAAQ==" - Base-64 encoded compressed dump of
 *     the PolyVox data. This property is typically not used in scripts directly; rather, functions that manipulate a PolyVox
 *     entity update it.
 *     <p>The size of this property increases with the size and complexity of the PolyVox entity, with the size depending on how
 *     the particular entity's voxels compress. Because this property value has to fit within a Overte datagram packet,
 *     there is a limit to the size and complexity of a PolyVox entity; edits which would result in an overflow are rejected.</p>
 * @property {Entities.PolyVoxSurfaceStyle} voxelSurfaceStyle=2 - The style of rendering the voxels' surface and how
 *     neighboring PolyVox entities are joined.
 * @property {string} xTextureURL="" - The URL of the texture to map to surfaces perpendicular to the entity's local x-axis.
 *     JPG or PNG format. If no texture is specified the surfaces display white.
 * @property {string} yTextureURL="" - The URL of the texture to map to surfaces perpendicular to the entity's local y-axis.
 *     JPG or PNG format. If no texture is specified the surfaces display white.
 * @property {string} zTextureURL="" - The URL of the texture to map to surfaces perpendicular to the entity's local z-axis.
 *     JPG or PNG format. If no texture is specified the surfaces display white.
 * @property {Uuid} xNNeighborID=Uuid.NONE - The ID of the neighboring PolyVox entity in the entity's -ve local x-axis
 *     direction, if you want them joined. Set to {@link Uuid(0)|Uuid.NONE} if there is none or you don't want to join them.
 * @property {Uuid} yNNeighborID=Uuid.NONE - The ID of the neighboring PolyVox entity in the entity's -ve local y-axis
 *     direction, if you want them joined. Set to {@link Uuid(0)|Uuid.NONE} if there is none or you don't want to join them.
 * @property {Uuid} zNNeighborID=Uuid.NONE - The ID of the neighboring PolyVox entity in the entity's -ve local z-axis
 *     direction, if you want them joined. Set to {@link Uuid(0)|Uuid.NONE} if there is none or you don't want to join them.
 * @property {Uuid} xPNeighborID=Uuid.NONE - The ID of the neighboring PolyVox entity in the entity's +ve local x-axis
 *     direction, if you want them joined. Set to {@link Uuid(0)|Uuid.NONE} if there is none or you don't want to join them.
 * @property {Uuid} yPNeighborID=Uuid.NONE - The ID of the neighboring PolyVox entity in the entity's +ve local y-axis
 *     direction, if you want them joined. Set to {@link Uuid(0)|Uuid.NONE} if there is none or you don't want to join them.
 * @property {Uuid} zPNeighborID=Uuid.NONE - The ID of the neighboring PolyVox entity in the entity's +ve local z-axis
 *     direction, if you want them joined. Set to {@link Uuid(0)|Uuid.NONE} if there is none or you don't want to join them.
 * @example <caption>Create a textured PolyVox sphere.</caption>
 * var position = Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.5, z: -8 }));
 * var texture = "http://public.highfidelity.com/cozza13/tuscany/Concrete2.jpg";
 * var polyVox = Entities.addEntity({
 *     type: "PolyVox",
 *     position: position,
 *     dimensions: { x: 2, y: 2, z: 2 },
 *     voxelVolumeSize: { x: 16, y: 16, z: 16 },
 *     voxelSurfaceStyle: 2,
 *     xTextureURL: texture,
 *     yTextureURL: texture,
 *     zTextureURL: texture,
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 * Entities.setVoxelSphere(polyVox, position, 0.8, 255);
 */

/*@jsdoc
 * The <code>"ProceduralParticleEffect"</code> {@link Entities.EntityType|EntityType} displays a particle system that can be
 * used to simulate things such as fire, smoke, snow, magic spells, etc. The particles are fully controlled by the provided
 * update and rendering shaders on the GPU.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-ProceduralParticleEffect
 * @property {number} numParticles=10000 - The number of particles to render.
 * @property {number} numTrianglesPerParticle=1 - The number of triangles to render per particle.  By default, these triangles
 *     still need to be positioned in the <code>particleRenderData</code> vertex shader.
 * @property {number} numUpdateProps=0 - The number of persistent Vec4 values stored per particle and updated once per frame.
 *     These can be modified in the <code>particleUpdateData</code> fragment shader and read in the
 *     <code>particleRenderData</code> vertex/fragment shaders.
 * @property {boolean} particleTransparent=false - Whether the particles should render as transparent (with additive blending)
 *     or opaque.
 * @property {ProceduralData} particleUpdateData="" - Used to store {@link ProceduralData} data as a JSON string to control
 *     per-particle updates if <code>numUpdateProps > 0</code>.  You can use <code>JSON.parse()</code> to parse the string
 *     into a JavaScript object which you can manipulate the properties of, and use <code>JSON.stringify()</code> to convert
 *     the object into a string to put in the property.
 * @property {ProceduralData} particleRenderData="" - Used to store {@link ProceduralData} data as a JSON string to control
 *     per-particle rendering.  You can use <code>JSON.parse()</code> to parse the string into a JavaScript object which you
 *     can manipulate the properties of, and use <code>JSON.stringify()</code> to convert the object into a string to put in
 *     the property.
 *
 * @example <caption>A cube of oscillating, unlit, billboarded triangles, with the oscillation in the update (computed once per particle instead of once per vertex).</caption>
 * particles = Entities.addEntity({
 *     type: "ProceduralParticleEffect",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.5, z: -4 })),
 *     dimensions: 3,
 *     numParticles: 10000,
 *     numTrianglesPerParticle: 1,
 *     numUpdateProps: 1,
 *     particleUpdateData: JSON.stringify({
 *         version: 1.0,
 *         fragmentShaderURL: "https://gist.githubusercontent.com/HifiExperiments/9049fb4a8dcd2c1401ff4321103dce16/raw/4f9474ed82c66c1f94c1055d2724af808cd7aace/proceduralParticleUpdate.fs",
 *     }),
 *     particleRenderData: JSON.stringify({
 *         version: 1.0,
 *         vertexShaderURL: "https://gist.github.com/HifiExperiments/5dda24e28e7de1719e3a594d81306343/raw/92e0c5b82a9fa87685064cdbab92ed0c16f49f94/proceduralParticle2.vs",
 *         fragmentShaderURL: "https://gist.github.com/HifiExperiments/7def54504362c7bc79b5c85cd515b98b/raw/93b3828c2ec66b12b789a625dd141f533c595ede/proceduralParticle.fs",
 *         uniforms: {
 *             radius: 0.03
 *         }
 *     }),
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Shape"</code> {@link Entities.EntityType|EntityType} displays an entity of a specified <code>shape</code>.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Shape
 * @property {Entities.Shape} shape="Sphere" - The shape of the entity.
 * @property {Vec3} dimensions=0.1,0.1,0.1 - The dimensions of the entity.
 * @property {Color} color=255,255,255 - The color of the entity.
 * @property {number} alpha=1 - The opacity of the entity, range <code>0.0</code> &ndash; <code>1.0</code>.
 * @property {boolean} unlit=false - <code>true</code> if the entity is unaffected by lighting, <code>false</code> if it is lit
 *     by the key light and local lights.
 * @property {Entities.Pulse} pulse - Color and alpha pulse.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @example <caption>Create a cylinder.</caption>
 * var shape = Entities.addEntity({
 *     type: "Shape",
 *     shape: "Cylinder",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -5 })),
 *     dimensions: { x: 0.4, y: 0.6, z: 0.4 },
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Sound"</code> {@link Entities.EntityType|EntityType} plays a sound from a URL. It has properties in addition to
 * the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Sound
 * @property {string} soundURL="" - The URL of the sound to play, as a wav, mp3, or raw file.  Supports stereo and ambisonic.
 *     Note: ambisonic sounds can only play as <code>localOnly</code>.
 * @property {boolean} playing=true - Whether or not the sound should play.
 * @property {number} volume=1.0 - The volume of the sound, from <code>0</code> to <code>1</code>.
 * @property {number} pitch=1.0 - The relative sample rate at which to resample the sound, within +/- 2 octaves.
 * @property {number} timeOffset=0.0 - The time (in seconds) at which to start playback within the sound file.  If looping,
 *     this only affects the first loop.
 * @property {boolean} loop=true - Whether or not to loop the sound.
 * @property {boolean} positional=true - Whether or not the volume of the sound should decay with distance.
 * @property {boolean} localOnly=false - Whether or not the sound should play locally for everyone (unsynced), or synchronously
 *     for everyone via the Entity Mixer.
 * @example <caption>Create a Sound entity.</caption>
 * var entity = Entities.addEntity({
 *     type: "Sound",
 *     soundURL: "https://themushroomkingdom.net/sounds/wav/lm/lm_gold_mouse.wav",
 *     positional: true,
 *     volume: 0.75,
 *     localOnly: true,
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.75, z: -4 })),
 *     rotation: MyAvatar.orientation,
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Sphere"</code> {@link Entities.EntityType|EntityType} is the same as the <code>"Shape"</code>
 * {@link Entities.EntityType|EntityType} except that its <code>shape</code> value is always set to <code>"Sphere"</code>
 * when the entity is created. If its <code>shape</code> property value is subsequently changed then the entity's
 * <code>type</code> will be reported as <code>"Box"</code> if the <code>shape</code> is set to <code>"Cube"</code>,
 * otherwise it will be reported as <code>"Shape"</code>.
 *
 * @typedef {object} Entities.EntityProperties-Sphere
 * @see {@link Entities.EntityProperties-Shape|EntityProperties-Shape}
 */

/*@jsdoc
 * The <code>"Text"</code> {@link Entities.EntityType|EntityType} displays a 2D rectangle of text in the domain.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Text
 * @property {Vec3} dimensions=0.1,0.1,0.01 - The dimensions of the entity.
 * @property {string} text="" - The text to display on the face of the entity. Text wraps if necessary to fit. New lines can be
 *     created using <code>\n</code>. Overflowing lines are not displayed.
 * @property {number} lineHeight=0.1 - The height of each line of text (thus determining the font size).
 * @property {Color} textColor=255,255,255 - The color of the text.
 * @property {number} textAlpha=1.0 - The opacity of the text.
 * @property {Color} backgroundColor=0,0,0 - The color of the background rectangle.
 * @property {number} backgroundAlpha=1.0 - The opacity of the background.
 * @property {Entities.Pulse} pulse - Color and alpha pulse.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {number} leftMargin=0.0 - The left margin, in meters.
 * @property {number} rightMargin=0.0 - The right margin, in meters.
 * @property {number} topMargin=0.0 - The top margin, in meters.
 * @property {number} bottomMargin=0.0 - The bottom margin, in meters.
 * @property {boolean} unlit=false - <code>true</code> if the entity is unaffected by lighting, <code>false</code> if it is lit
 *     by the key light and local lights.
 * @property {string} font="" - The font to render the text with. It can be one of the following: <code>"Courier"</code>,
 *     <code>"Inconsolata"</code>, <code>"Roboto"</code>, <code>"Timeless"</code>, or a path to a PNG MTSDF .arfont file generated
 *     by the msdf-atlas-gen tool (https://github.com/Chlumsky/msdf-atlas-gen).
 * @property {Entities.TextEffect} textEffect="none" - The effect that is applied to the text.
 * @property {Color} textEffectColor=255,255,255 - The color of the effect.
 * @property {number} textEffectThickness=0.2 - The magnitude of the text effect, range <code>0.0</code> &ndash; <code>0.5</code>.
 * @property {Entities.TextAlignment} alignment="left" - How the text is horizontally aligned against its background.
 * @property {Entities.TextVerticalAlignment} verticalAlignment="top" - How the text is vertically aligned against its background.
 * @property {boolean} faceCamera - <code>true</code> if <code>billboardMode</code> is <code>"yaw"</code>, <code>false</code>
 *     if it isn't. Setting this property to <code>false</code> sets the <code>billboardMode</code> to <code>"none"</code>.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {boolean} isFacingAvatar - <code>true</code> if <code>billboardMode</code> is <code>"full"</code>,
 *     <code>false</code> if it isn't. Setting this property to <code>false</code> sets the <code>billboardMode</code> to
 *     <code>"none"</code>.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @example <caption>Create a text entity.</caption>
 * var text = Entities.addEntity({
 *     type: "Text",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -5 })),
 *     dimensions: { x: 0.6, y: 0.3, z: 0.01 },
 *     lineHeight: 0.12,
 *     text: "Hello\nthere!",
 *     billboardMode: "yaw",
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Web"</code> {@link Entities.EntityType|EntityType} displays a browsable web page. Each user views their own copy
 * of the web page: if one user navigates to another page on the entity, other users do not see the change; if a video is being
 * played, users don't see it in sync. Internally, a Web entity is rendered as a non-repeating, upside down texture, so additional
 * transformations may be necessary if you reference a Web entity texture by UUID. It has properties in addition to the common
 * {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Web
 * @property {Vec3} dimensions=0.1,0.1,0.01 - The dimensions of the entity.
 * @property {string} sourceUrl="" - The URL of the web page to display. This value does not change as you or others navigate
 *     on the Web entity.
 * @property {Color} color=255,255,255 - The color of the web surface. This color tints the web page displayed: the pixel
 *     colors on the web page are multiplied by the property color. For example, a value of
 *     <code>{ red: 255, green: 0, blue: 0 }</code> lets only the red channel of pixels' colors through.
 * @property {number} alpha=1 - The opacity of the web surface.
 * @property {Entities.Pulse} pulse - Color and alpha pulse.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {boolean} faceCamera - <code>true</code> if <code>billboardMode</code> is <code>"yaw"</code>, <code>false</code>
 *     if it isn't. Setting this property to <code>false</code> sets the <code>billboardMode</code> to <code>"none"</code>.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {boolean} isFacingAvatar - <code>true</code> if <code>billboardMode</code> is <code>"full"</code>,
 *     <code>false</code> if it isn't. Setting this property to <code>false</code> sets the <code>billboardMode</code> to
 *     <code>"none"</code>.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {number} dpi=30 - The resolution to display the page at, in dots per inch. If you convert this to dots per meter
 *     (multiply by 1 / 0.0254 = 39.3701) then multiply <code>dimensions.x</code> and <code>dimensions.y</code> by that value
 *     you get the resolution in pixels.
 * @property {string} scriptURL="" - The URL of a JavaScript file to inject into the web page.
 * @property {number} maxFPS=10 - The maximum update rate for the web content, in frames/second.
 * @property {WebInputMode} inputMode="touch" - The user input mode to use.
 * @property {boolean} wantsKeyboardFocus=true - <code>true</code> if the entity should capture keyboard focus, <code>false</code> if it
 *     shouldn't.
 * @property {boolean} showKeyboardFocusHighlight=true - <code>true</code> if the entity is highlighted when it has keyboard
 *     focus, <code>false</code> if it isn't.
 * @property {boolean} useBackground=true - <code>true</code> if the web entity should have a background,
 *     <code>false</code> if the web entity's background should be transparent. The webpage must have CSS properties for transparency set
 *     on the <code>background-color</code> for this property to have an effect.
 * @property {string} userAgent - The user agent for the web entity to use when visiting web pages.
 *     Default value: <code>Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko)
 *     Chrome/69.0.3497.113 Mobile Safari/537.36</code>
 * @example <caption>Create a Web entity displaying at 1920 x 1080 resolution.</caption>
 * var METERS_TO_INCHES = 39.3701;
 * var entity = Entities.addEntity({
 *     type: "Web",
 *     sourceUrl: "https://overte.org/",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0.75, z: -4 })),
 *     rotation: MyAvatar.orientation,
 *     dimensions: {
 *         x: 3,
 *         y: 3 * 1080 / 1920,
 *         z: 0.01
 *     },
 *     dpi: 1920 / (3 * METERS_TO_INCHES),
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Zone"</code> {@link Entities.EntityType|EntityType} is a volume of lighting effects and avatar permissions.
 * Avatar interaction events such as {@link Entities.enterEntity} are also often used with a Zone entity. It has properties in
 * addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Zone
 * @property {Vec3} dimensions=0.1,0.1,0.1 - The dimensions of the volume in which the zone's lighting effects and avatar
 *     permissions have effect.
 *
 * @property {ShapeType} shapeType="box" - The shape of the volume in which the zone's lighting effects and avatar
 *     permissions have effect. Reverts to the default value if set to <code>"none"</code>, or set to <code>"compound"</code>
 *     and <code>compoundShapeURL</code> is <code>""</code>.
  * @property {string} compoundShapeURL="" - The model file to use for the compound shape if <code>shapeType</code> is
 *     <code>"compound"</code>.
 *
 * @property {Entities.ComponentMode} keyLightMode="inherit" - Configures the key light in the zone.
 * @property {Entities.KeyLight} keyLight - The key light properties of the zone.
 *
 * @property {Entities.ComponentMode} ambientLightMode="inherit" - Configures the ambient light in the zone.
 * @property {Entities.AmbientLight} ambientLight - The ambient light properties of the zone.
 *
 * @property {Entities.ComponentMode} skyboxMode="inherit" - Configures the skybox displayed in the zone.
 * @property {Entities.Skybox} skybox - The skybox properties of the zone.
 *
 * @property {Entities.ComponentMode} hazeMode="inherit" - Configures the haze in the zone.
 * @property {Entities.Haze} haze - The haze properties of the zone.
 *
 * @property {Entities.ComponentMode} bloomMode="inherit" - Configures the bloom in the zone.
 * @property {Entities.Bloom} bloom - The bloom properties of the zone.
 *
 * @property {Entities.ZoneAudio} audio - The audio properties of the zone.
 *
 * @property {Entities.ComponentMode} tonemappingMode="inherit" - Configures the tonemapping in the zone.
 * @property {Entities.Tonemapping} tonemapping - The tonemapping properties of the zone.
 *
 * @property {Entities.ComponentMode} ambientOcclusionMode="inherit" - Configures the ambient occlusion in the zone.
 * @property {Entities.AmbientOcclusion} ambientOcclusion - The ambient occlusion properties of the zone.
 *
 * @property {boolean} flyingAllowed=true - <code>true</code> if visitors can fly in the zone; <code>false</code> if they
 *     cannot. Only works for domain entities.
 * @property {boolean} ghostingAllowed=true - <code>true</code> if visitors with avatar collisions turned off will not
 *     collide with content in the zone; <code>false</code> if visitors will always collide with content in the zone. Only
 *     works for domain entities.
 *
 * @property {string} filterURL="" - The URL of a JavaScript file that filters changes to properties of entities within the
 *     zone. It is periodically executed for each entity in the zone. It can, for example, be used to not allow changes to
 *     certain properties:
 * <pre>
 * function filter(properties) {
 *     // Check and edit properties object values,
 *     // e.g., properties.modelURL, as required.
 *     return properties;
 * }
 * </pre>
 *
 * @property {Entities.AvatarPriorityMode} avatarPriority="inherit" - Configures the priority of updates from avatars in the
 *     zone to other clients.
 *
 * @example <caption>Create a zone that casts a red key light along the x-axis.</caption>
 * var zone = Entities.addEntity({
 *     type: "Zone",
 *     position: MyAvatar.position,
 *     dimensions: { x: 100, y: 100, z: 100 },
 *     keyLightMode: "enabled",
 *     keyLight: {
 *         "color": { "red": 255, "green": 0, "blue": 0 },
 *         "direction": { "x": 1, "y": 0, "z": 0 }
 *     },
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Image"</code> {@link Entities.EntityType|EntityType} displays an image on a 2D rectangle in the domain.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Image
 * @property {Vec3} dimensions=0.1,0.1,0.01 - The dimensions of the entity.
 * @property {string} imageURL="" - The URL of the image to use.
 * @property {boolean} emissive=false - <code>true</code> if the image should be emissive (unlit), <code>false</code> if it
 *     shouldn't.
 * @property {boolean} keepAspectRatio=true - <code>true</code> if the image should maintain its aspect ratio,
 *     <code>false</code> if it shouldn't.
 * @property {Rect} subImage=0,0,0,0 - The portion of the image to display. If width or height are <code>0</code>, it defaults
 *     to the full image in that dimension.
 * @property {Color} color=255,255,255 - The color of the image.
 * @property {number} alpha=1 - The opacity of the image.
 * @property {Entities.Pulse} pulse - Color and alpha pulse.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {boolean} faceCamera - <code>true</code> if <code>billboardMode</code> is <code>"yaw"</code>, <code>false</code>
 *     if it isn't. Setting this property to <code>false</code> sets the <code>billboardMode</code> to <code>"none"</code>.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {boolean} isFacingAvatar - <code>true</code> if <code>billboardMode</code> is <code>"full"</code>,
 *     <code>false</code> if it isn't. Setting this property to <code>false</code> sets the <code>billboardMode</code> to
 *     <code>"none"</code>.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @example <caption>Create an image entity.</caption>
 * var image = Entities.addEntity({
 *     type: "Image",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -5 })),
 *     dimensions: { x: 0.6, y: 0.3, z: 0.01 },
 *     imageURL: "https://images.pexels.com/photos/1020315/pexels-photo-1020315.jpeg",
 *     billboardMode: "yaw",
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Grid"</code> {@link Entities.EntityType|EntityType} displays a grid on a 2D plane.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Grid
 * @property {Vec3} dimensions - 0.1,0.1,0.01 - The dimensions of the entity.
 * @property {Color} color=255,255,255 - The color of the grid.
 * @property {number} alpha=1 - The opacity of the grid.
 * @property {Entities.Pulse} pulse - Color and alpha pulse.
 *     <p class="important">Deprecated: This property is deprecated and will be removed.</p>
 * @property {boolean} followCamera=true - <code>true</code> if the grid is always visible even as the camera moves to another
 *     position, <code>false</code> if it doesn't follow the camrmea.
 * @property {number} majorGridEvery=5 - Integer number of <code>minorGridEvery</code> intervals at which to draw a thick grid
 *     line. Minimum value = <code>1</code>.
 * @property {number} minorGridEvery=1 - Real number of meters at which to draw thin grid lines. Minimum value =
 *     <code>0.001</code>.
 * @example <caption>Create a grid entity.</caption>
 * var grid = Entities.addEntity({
 *     type: "Grid",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -5 })),
 *     dimensions: { x: 100.0, y: 100.0, z: 0.01 },
 *     followCamera: false,
 *     majorGridEvery: 4,
 *     minorGridEvery: 0.5,
 *     lifetime: 300  // Delete after 5 minutes.
 * });
 */

/*@jsdoc
 * The <code>"Gizmo"</code> {@link Entities.EntityType|EntityType} displays an entity that could be used as UI.
 * It has properties in addition to the common {@link Entities.EntityProperties|EntityProperties}.
 *
 * @typedef {object} Entities.EntityProperties-Gizmo
 * @property {Vec3} dimensions=0.1,0.001,0.1 - The dimensions of the entity.
 * @property {Entities.GizmoType} gizmoType="ring" - The gizmo type of the entity.
 * @property {Entities.RingGizmo} ring - The ring gizmo properties.
 */
