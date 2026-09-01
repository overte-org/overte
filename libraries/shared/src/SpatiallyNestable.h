//
//  SpatiallyNestable.h
//  libraries/shared/src/
//
//  Created by Seth Alves on 2015-10-18
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SpatiallyNestable_h
#define hifi_SpatiallyNestable_h

#include <QUuid>

#include "Transform.h"
#include "AACube.h"
#include "SpatialParentFinder.h"
#include "shared/ReadWriteLockable.h"
#include "Grab.h"

class SpatiallyNestable;
using SpatiallyNestableWeakPointer = std::weak_ptr<SpatiallyNestable>;
using SpatiallyNestableWeakConstPointer = std::weak_ptr<const SpatiallyNestable>;
using SpatiallyNestablePointer = std::shared_ptr<SpatiallyNestable>;
using SpatiallyNestableConstPointer = std::shared_ptr<const SpatiallyNestable>;

static const uint16_t INVALID_JOINT_INDEX = -1;

enum class NestableType {
    Entity,
    Avatar
};

// FIXME: SpatiallyNestable is very inconsistent and
// has an **enormous** amount of code duplication.
// Most of its member functions could be replaced
// with ones that take/return `Transform &`.
/**
 * The base "entity" class, with a transform and parent-child hierarchy.
 * The common ancestor of EntityItem and AvatarData.
 */
class SpatiallyNestable : public std::enable_shared_from_this<SpatiallyNestable> {
public:
    SpatiallyNestable(NestableType nestableType, QUuid id);
    virtual ~SpatiallyNestable();

    // FIXME: Is there a good reason these are virtual?
    // It doesn't look like anything actually overrides them
    virtual const QUuid getID() const;
    virtual void setID(const QUuid& id);

    // FIXME: SpatiallyNestables are always either an EntityItem
    // or an AvatarData, so why isn't this a pure virtual function?
    /**
     * @return The entity's name property for EntityItem, and "Avatar: displayName" for avatars.
     */
    virtual QString getName() const { return "SpatiallyNestable"; }

    /** @return The UUID of this SpatiallyNestable's parent. A null UUID if there is no parent. */
    virtual const QUuid getParentID() const;
    /** @param parentID The new parent UUID. Can be a null UUID for no parent. */
    virtual void setParentID(const QUuid& parentID);

    virtual bool isMyAvatar() const { return false; }

    /** @return This SpatiallyNestable's parent joint index, or -1 if it has none set. */
    virtual quint16 getParentJointIndex() const { return _parentJointIndex; }
    /** @param parentJointIndex A parent joint index, or -1 for none. */
    virtual void setParentJointIndex(quint16 parentJointIndex);

    /**
     * @param position Global position vector
     * @param parentID SpatiallyNestable that position will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that position will be transformed relative to
     * @param scalesWithParent Whether the global-to-local transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 worldToLocal(const glm::vec3& position, const QUuid& parentID, int parentJointIndex,
                                  bool scalesWithParent, bool& success);
    // FIXME: What does "inheriting scale" mean on a rotation?
    /**
     * @param orientation Global orientation quaternion
     * @param parentID SpatiallyNestable that orientation will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that orientation will be transformed relative to
     * @param scalesWithParent Whether the global-to-local transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::quat worldToLocal(const glm::quat& orientation, const QUuid& parentID, int parentJointIndex,
                                  bool scalesWithParent, bool& success);
    /**
     * @param velocity Global linear velocity vector
     * @param parentID SpatiallyNestable that velocity will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that velocity will be transformed relative to
     * @param scalesWithParent Whether the global-to-local transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 worldToLocalVelocity(const glm::vec3& velocity, const QUuid& parentID,
                                          int parentJointIndex, bool scalesWithParent, bool& success);
    /**
     * @param angularVelocity Global angular velocity vector
     * @param parentID SpatiallyNestable that angularVelocity will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that angularVelocity will be transformed relative to
     * @param scalesWithParent Whether the global-to-local transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 worldToLocalAngularVelocity(const glm::vec3& angularVelocity, const QUuid& parentID,
                                                 int parentJointIndex, bool scalesWithParent, bool& success);
    /**
     * @param dimensions Global dimensions
     * @param parentID SpatiallyNestable that dimensions will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that dimensions will be transformed relative to
     * @param scalesWithParent Whether the global-to-local transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 worldToLocalDimensions(const glm::vec3& dimensions, const QUuid& parentID,
                                            int parentJointIndex, bool scalesWithParent, bool& success);

    /**
     * @param position Position vector local to parentID
     * @param parentID SpatiallyNestable that position will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that position will be transformed relative to
     * @param scalesWithParent Whether the local-to-global transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 localToWorld(const glm::vec3& position, const QUuid& parentID, int parentJointIndex,
                                  bool scalesWithParent, bool& success);
    /**
     * @param orientation Orientation quaternion local to parentID
     * @param parentID SpatiallyNestable that orientation will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that orientation will be transformed relative to
     * @param scalesWithParent Whether the local-to-global transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::quat localToWorld(const glm::quat& orientation, const QUuid& parentID, int parentJointIndex,
                                  bool scalesWithParent, bool& success);
    /**
     * @param velocity Linear velocity vector local to parentID
     * @param parentID SpatiallyNestable that velocity will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that velocity will be transformed relative to
     * @param scalesWithParent Whether the local-to-global transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 localToWorldVelocity(const glm::vec3& velocity,
                                          const QUuid& parentID, int parentJointIndex, bool scalesWithParent, bool& success);
    /**
     * @param angularVelocity Angular velocity vector local to parentID
     * @param parentID SpatiallyNestable that angularVelocity will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that angularVelocity will be transformed relative to
     * @param scalesWithParent Whether the local-to-global transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 localToWorldAngularVelocity(const glm::vec3& angularVelocity,
                                                 const QUuid& parentID, int parentJointIndex,
                                                 bool scalesWithParent, bool& success);
    /**
     * @param dimensions Dimensions vector local to parentID
     * @param parentID SpatiallyNestable that dimensions will be transformed relative to
     * @param parentJointIndex The joint of the parent SpatiallyNestable that dimensions will be transformed relative to
     * @param scalesWithParent Whether the local-to-global transformation takes scaling into account
     * @param success false if parentID or parentJointIndex is invalid
     */
    static glm::vec3 localToWorldDimensions(const glm::vec3& dimensions, const QUuid& parentID,
                                            int parentJointIndex, bool scalesWithParent, bool& success);

    // FIXME: There is no valid variant of NestableType that would produce "unknown"
    /**
     * @param nestableType
     * @return A string matching nestableType: "entity", "avatar", or "unknown".
     */
    static QString nestableTypeToString(NestableType nestableType);

    /**
     * @param depth How many parents to check before assuming it's looping, Defaults to 0, which will mean this function only ever returns true if the SpatiallyNestable has no parent.
     * @return true if this SpatiallyNestable's parent tree is complete and non-recursive
     */
    virtual bool isParentPathComplete(int depth = 0) const;


    /**
     * @param success true if this SpatiallyNestable has a complete, non-recursive parent tree.
     * @param depth How many parent transforms in the parent tree to account for.
     * @return A copy of this SpatiallyNestable's <em>global</em> transform if depth is > 0, otherwise its <em>local</em> transform.
     */
    virtual const Transform getTransform(bool& success, int depth = 0) const;
    // FIXME: This looks like a hack. It's only used by Model entities when billboarding,
    // and ModelEntityItem just overrides this anyway.
    /**
     * @param success true if this SpatiallyNestable has a complete, non-recursive parent tree.
     * @param depth How many parent transforms in the parent tree to account for.
     * @return A copy of this SpatiallyNestable's <em>global</em> transform, but with a <em>local</em> rotation.
     */
    virtual const Transform getTransformWithOnlyLocalRotation(bool& success, int depth = 0) const;
    /**
     * @return A copy of this SpatiallyNestable's <em>local</em> transform.
     */
    virtual const Transform getTransform() const;
    /**
     * @param transform A new <em>global</em> transform for this SpatiallyNestable.
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     */
    virtual void setTransform(const Transform& transform, bool& success);
    /**
     * @param transform A new <em>global</em> transform for this SpatiallyNestable.
     */
    virtual bool setTransform(const Transform& transform);

    /**
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @param depth How many parent transforms in the parent tree to take into account.
     * @returns A copy of this SpatiallyNestable's global transform, excluding its own local transform.
     */
    virtual Transform getParentTransform(bool& success, int depth = 0) const;

    /**
     * @param position Global position
     * @param orientation Global rotation
     */
    void setWorldTransform(const glm::vec3& position, const glm::quat& orientation);
    /**
     * @param success true if this SpatiallyNestable has a complete, non-recursive parent tree.
     * @return This SpatiallyNestable's global position
     */
    virtual glm::vec3 getWorldPosition(bool& success) const;
    /**
     * @return This SpatiallyNestable's global position
     */
    virtual glm::vec3 getWorldPosition() const;
    /**
     * @param position Global position vector
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @param tellPhysics
     */
    virtual void setWorldPosition(const glm::vec3& position, bool& success, bool tellPhysics = true);
    /** @param position Global position vector */
    virtual void setWorldPosition(const glm::vec3& position);

    /**
     * @param success true if this SpatiallyNestable has a complete, non-recursive parent tree.
     * @return This SpatiallyNestable's global rotation
     */
    virtual glm::quat getWorldOrientation(bool& success) const;
    /** @return This SpatiallyNestable's global rotation */
    virtual glm::quat getWorldOrientation() const;
    // FIXME: This should be called getJointWorldOrientation
    /**
     * @param jointIndex The joint index to get the global rotation of
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return The global rotation of the joint
     */
    virtual glm::quat getWorldOrientation(int jointIndex, bool& success) const;
    /**
     * @param orientation Global rotation quaternion
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @param tellPhysics
     */
    virtual void setWorldOrientation(const glm::quat& orientation, bool& success, bool tellPhysics = true);
    /** @param orientation Global rotation quaternion */
    virtual void setWorldOrientation(const glm::quat& orientation);

    /**
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return The global linear velocity vector
     */
    virtual glm::vec3 getWorldVelocity(bool& success) const;
    /** @return The global linear velocity vector */
    virtual glm::vec3 getWorldVelocity() const;
    /**
     * @param velocity Global linear velocity vector
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     */
    virtual void setWorldVelocity(const glm::vec3& velocity, bool& success);
    /** @param velocity Global linear velocity vector */
    virtual void setWorldVelocity(const glm::vec3& velocity);
    /**
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return The global linear velocity of this SpatiallyNestable's parent tree, excluding its own local velocity.
     */
    virtual glm::vec3 getParentVelocity(bool& success) const;

    /**
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return The global angular velocity vector
     */
    virtual glm::vec3 getWorldAngularVelocity(bool& success) const;
    /** @return The global angular velocity vector */
    virtual glm::vec3 getWorldAngularVelocity() const;
    /**
     * @param angularVelocity Global angular velocity vector
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     */
    virtual void setWorldAngularVelocity(const glm::vec3& angularVelocity, bool& success);
    /** @param angularVelocity Global angular velocity vector */
    virtual void setWorldAngularVelocity(const glm::vec3& angularVelocity);
    /**
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return The global angular velocity of this SpatiallyNestable's parent tree, excluding its own local velocity.
     */
    virtual glm::vec3 getParentAngularVelocity(bool& success) const;

    // TODO: Document these, I don't understand what they're doing
    virtual AACube getMaximumAACube(bool& success) const;
    virtual AACube calculateInitialQueryAACube(bool& success);

    virtual void setQueryAACube(const AACube& queryAACube);
    virtual bool queryAACubeNeedsUpdate() const;
    virtual bool queryAACubeNeedsUpdateWithDescendantAACube(const AACube& descendantAACube) const;
    virtual bool shouldPuffQueryAACube() const { return false; }
    bool updateQueryAACube(bool updateParent = true);
    bool updateQueryAACubeWithDescendantAACube(const AACube& descendentAACube, bool updateParent = true);
    void forceQueryAACubeUpdate() { _queryAACubeSet = false; }
    virtual AACube getQueryAACube(bool& success) const;
    virtual AACube getQueryAACube() const;

    /** @return This SpatiallyNestable's global scale. */
    virtual glm::vec3 getSNScale() const;
    /**
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return This SpatiallyNestable's global scale.
     */
    virtual glm::vec3 getSNScale(bool& success) const;
    /** @param scale The new global global scale. */
    virtual void setSNScale(const glm::vec3& scale);
    /**
     * @param scale The new global global scale.
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     */
    virtual void setSNScale(const glm::vec3& scale, bool& success);

    /**
     * @param jointIndex The joint index to get the global transform of
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @param depth How many parent transforms in the parent tree to take into account.
     * @return The global transform of the specified joint, or identity if the joint is invalid
     */
    virtual const Transform getJointTransform(int jointIndex, bool& success, int depth = 0) const;
    /**
     * @param jointIndex The joint index to get the global position of
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return The global position of the specified joint, or zero if the joint is invalid
     */
    virtual glm::vec3 getJointWorldPosition(int jointIndex, bool& success) const;
    /**
     * @param jointIndex The joint index to get the global scale of
     * @param success true if this SpatiallyNestable has a valid parent transform or no parent.
     * @return The global position of the specified joint, or zero if the joint is invalid
     */
    virtual glm::vec3 getJointSNScale(int jointIndex, bool& success) const;

    // object's parent's frame
    virtual Transform getLocalTransform() const;
    virtual void setLocalTransform(const Transform& transform);

    virtual glm::vec3 getLocalPosition() const;
    virtual void setLocalPosition(const glm::vec3& position, bool tellPhysics = true);

    virtual glm::quat getLocalOrientation() const;
    virtual void setLocalOrientation(const glm::quat& orientation);

    virtual glm::vec3 getLocalVelocity() const;
    virtual void setLocalVelocity(const glm::vec3& velocity);

    virtual glm::vec3 getLocalAngularVelocity() const;
    virtual void setLocalAngularVelocity(const glm::vec3& angularVelocity);

    virtual glm::vec3 getLocalSNScale() const;
    virtual void setLocalSNScale(const glm::vec3& scale);

    /** @return true if this SpatiallyNestable should inherit its parents' scales */
    virtual bool getScalesWithParent() const { return false; }
    // FIXME: This looks like a bad hack. Children should just inherit the SNScale
    // of their parent joint, but Avatar and MyAvatar override this in different ways.
    // https://github.com/overte-org/overte/issues/2071
    /** @return Scale vector for child nodes to scale themselves with, only used by avatars for scaling attached avatar entities */
    virtual glm::vec3 scaleForChildren() const { return glm::vec3(1.0f); }

    QList<SpatiallyNestablePointer> getChildren() const;
    bool hasChildren() const;

    NestableType getNestableType() const { return _nestableType; }

    // this object's frame
    virtual const Transform getAbsoluteJointTransformInObjectFrame(int jointIndex) const;
    virtual glm::vec3 getAbsoluteJointScaleInObjectFrame(int index) const { return glm::vec3(1.0f); }
    virtual glm::quat getAbsoluteJointRotationInObjectFrame(int index) const { return glm::quat(); }
    virtual glm::vec3 getAbsoluteJointTranslationInObjectFrame(int index) const { return glm::vec3(); }
    /**
     * @param index The joint to get the parent of
     * @return The parent joint index of index, or -1 if it is invalid or the root joint.
     */
    virtual int getJointParent(int index) const { return -1; }

    virtual bool setAbsoluteJointRotationInObjectFrame(int index, const glm::quat& rotation) { return false; }
    virtual bool setAbsoluteJointTranslationInObjectFrame(int index, const glm::vec3& translation) {return false; }

    /**
     * Not implemented on SpatiallyNestable.
     * @param index Joint index
     * @return Local joint rotation quaternion, relative to the joint's parent
     */
    virtual glm::quat getLocalJointRotation(int index) const {return glm::quat(); }
    /**
     * Not implemented on SpatiallyNestable.
     * @param index Joint index
     * @return Local joint translation vector, relative to the joint's parent
     */
    virtual glm::vec3 getLocalJointTranslation(int index) const {return glm::vec3(); }
    /**
     * Not implemented on SpatiallyNestable.
     * @param index Joint index
     * @param rotation Local rotation quaternion
     * @return true if the joint index is valid, relative to the joint's parent
     */
    virtual bool setLocalJointRotation(int index, const glm::quat& rotation) { return false; }
    /**
     * Not implemented on SpatiallyNestable.
     * @param index Joint index
     * @param translation Local translation vector
     * @return true if the joint index is valid, relative to the joint's parent
     */
    virtual bool setLocalJointTranslation(int index, const glm::vec3& translation) { return false; }

    /** @return shared_from_this() const-casted back to a mutable pointer */
    SpatiallyNestablePointer getThisPointer() const;

    using ChildLambda = std::function<void(const SpatiallyNestablePointer&)>;
    using ChildLambdaTest = std::function<bool(const SpatiallyNestablePointer&)>;

    /** @param actor A function that will run on each of this SpatiallyNestable's children */
    void forEachChild(const ChildLambda& actor) const;
    /**
     * @param actor A function that will run on each of this SpatiallyNestable's children, and their children
     */
    void forEachDescendant(const ChildLambda& actor) const;
    /**
     * @param actor A function that will run on each of this SpatiallyNestable's children, but will stop iterating if the function returns false
     */
    void forEachChildTest(const ChildLambdaTest&  actor) const;
    /**
     * @param actor A function that will run on each of this SpatiallyNestable's children, and their children, but will stop iterating if the function returns false
     */
    void forEachDescendantTest(const ChildLambdaTest& actor) const;

    /** Marks this SpatiallyNestable as safe to remove. */
    void die() { _isDead = true; }
    /** @return true if this SpatiallyNestable is safe to remove. */
    bool isDead() const { return _isDead; }

    /** @return true if this SpatiallyNestable's direct parent is valid and exists, or if it has no parent.  */
    bool isParentIDValid() const { bool success = false; getParentPointer(success); return success; }

    /**
     * Only implemented in EntityItem.
     * @return nullptr
     */
    virtual SpatialParentTree* getParentTree() const { return nullptr; }

    bool hasAncestorOfType(NestableType nestableType, int depth = 0) const;
    const QUuid findAncestorOfType(NestableType nestableType, int depth = 0) const;
    /**
     * @param success true if this SpatiallyNestable has a valid parent tree, or if it has no parent.
     * @returns The direct parent, or nullptr if there is no parent.
     */
    SpatiallyNestablePointer getParentPointer(bool& success) const;
    static SpatiallyNestablePointer findByID(QUuid id, bool& success);

    void getLocalTransformAndVelocities(Transform& localTransform,
                                        glm::vec3& localVelocity,
                                        glm::vec3& localAngularVelocity) const;

    void setLocalTransformAndVelocities(
            const Transform& localTransform,
            const glm::vec3& localVelocity,
            const glm::vec3& localAngularVelocity);

    /**
     * @param time Microseconds since the engine started
     * @return true if this SpatiallyNestable's scale has changed since time
     */
    bool scaleChangedSince(quint64 time) const { return _scaleChanged > time; }
    /**
     * @param time Microseconds since the engine started
     * @return true if this SpatiallyNestable's position has changed since time
     */
    bool tranlationChangedSince(quint64 time) const { return _translationChanged > time; }
    /**
     * @param time Microseconds since the engine started
     * @return true if this SpatiallyNestable's rotation has changed since time
     */
    bool rotationChangedSince(quint64 time) const { return _rotationChanged > time; }

    /**
     * Dumps this SpatiallyNestable's state to the log.
     * @param prefix Prefix string before the dump log lines
     */
    void dump(const QString& prefix = "") const;

    /** Called when this SpatiallyNestable's position has changed. */
    virtual void locationChanged(bool tellPhysics = true, bool tellChildren = true);
    // FIXME: This should probably be moved to EntityItem.
    // SpatiallyNestable doesn't have dimensions.
    /** Called when this SpatiallyNestable's dimensions have changed. */
    virtual void dimensionsChanged() { _queryAACubeSet = false; }
    /** Called when this SpatiallyNestable's parent has been deleted. */
    virtual void parentDeleted() { }

    virtual void addGrab(GrabPointer grab);
    virtual void removeGrab(GrabPointer grab);
    virtual void disableGrab(GrabPointer grab) {};
    bool hasGrabs();
    virtual QUuid getEditSenderID();

protected:
    QUuid _id;
    mutable SpatiallyNestableWeakPointer _parent;

    virtual void beParentOfChild(SpatiallyNestablePointer newChild) const;
    virtual void forgetChild(SpatiallyNestablePointer newChild) const;
    // TODO: "Cauterization" is a weirdly violent term for this.
    // It should instead be called something like joint hiding or culling.
    virtual void recalculateChildCauterization() const { }

    // TODO: Is it necessary to reference both the parent *and* child nodes?
    mutable ReadWriteLockable _childrenLock;
    mutable QHash<QUuid, SpatiallyNestableWeakPointer> _children;

    // _queryAACube is used to decide where something lives in the octree
    mutable AACube _queryAACube;
    mutable bool _queryAACubeSet { false };

    /** Microsecond timestamp of when this SpatiallyNestable's SNScale was changed */
    quint64 _scaleChanged { 0 };
    /** Microsecond timestamp of when this SpatiallyNestable's position was changed */
    quint64 _translationChanged { 0 };
    /** Microsecond timestamp of when this SpatiallyNestable's rotation was changed */
    quint64 _rotationChanged { 0 };

    mutable ReadWriteLockable _grabsLock;
    QSet<GrabPointer> _grabs; // upon this thing

private:
    SpatiallyNestable() = delete;
    const NestableType _nestableType; // EntityItem or an AvatarData
    QUuid _parentID; // what is this thing's transform relative to?
    quint16 _parentJointIndex { INVALID_JOINT_INDEX }; // which joint of the parent is this relative to?

    mutable ReadWriteLockable _transformLock;
    mutable ReadWriteLockable _idLock;
    mutable ReadWriteLockable _velocityLock;
    mutable ReadWriteLockable _angularVelocityLock;
    Transform _transform; /**< This SpatiallyNestable's local transform */
    glm::vec3 _velocity; /**< This SpatiallyNestable's local linear velocity */
    glm::vec3 _angularVelocity; /**< This SpatiallyNestable's local angular velocity */
    mutable bool _parentKnowsMe { false };
    bool _isDead { false };
    bool _queryAACubeIsPuffed { false };

    void breakParentingLoop() const;
};


#endif // hifi_SpatiallyNestable_h
