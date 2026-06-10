//
//  Scene.h
//  render/src/render
//
//  Created by Sam Gateau on 1/11/15.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_Scene_h
#define hifi_render_Scene_h

#include "Item.h"
#include "SpatialTree.h"
#include "Stage.h"
#include "Selection.h"
#include "Transition.h"
#include "HighlightStyle.h"

namespace render {

class RenderEngine;
class Scene;

/**
 / @brief Transaction is the mechanism to make any change to the scene.
 * Whenever a new item need to be reset,
 * or when an item changes its position or its size,
 * or when an item's payload has to be updated with new states (coming from outside the scene knowledge),
 * or when an item is destroyed,
 * these changes must be expressed through the corresponding command from the Transaction.
 * The Transaction is then queued on the Scene so all the pending transactions can be consolidated and processed at the time
 * of updating the scene before it s rendered.
 */


class Transaction {
    friend class Scene;
public:

    typedef std::function<void(ItemID, const Transition*)> TransitionQueryFunc;
    typedef std::function<void()> TransitionFinishedFunc;
    typedef std::function<void(HighlightStyle const*)> SelectionHighlightQueryFunc;

    Transaction() {}
    ~Transaction() {}

    // Item transactions

    /**
     * @brief The transaction will replace item's payload with new one.
     * @param id Item ID.
     * @param payload Shared pointer to the new payload.
     */
    void resetItem(ItemID id, const PayloadPointer& payload);

    /**
     * @brief The transaction will remove given item.
     * @param id Item ID.
     */
    void removeItem(ItemID id);

    /**
     * @return `true` if a given transaction removes items.
     */
    bool hasRemovedItems() const { return !_removedItems.empty(); }

    /**
     * @brief The transaction will execute a function on the render thread to update the given render item.
     *
     * Spatial properties and key of an item are also updated.
     * @tparam T Type of the render item to update.
     * @param id Item ID.
     * @param func Function that will perform the update.
     */
    template <class T> void updateItem(ItemID id, std::function<void(T&)> func) {
        updateItem(id, std::make_shared<UpdateFunctor<T>>(func));
    }


    /**
     * @brief The transaction will update an item without running a function.
     *
     * Spatial properties and key of an item will be updated on render thread.
     * @param id Item ID.
     */
    void updateItem(ItemID id) { updateItem(id, nullptr); }

    // Transition (applied to an item) transactions

    /**
     * @brief The transaction will start a new fade effect on a given item.
     *
     * If given item already has a fade effect going, it will be replaced with the new one.
     * @param id Item ID.
     * @param transition Type of the transition that should be started.
     * @param boundId If it's specified, dimensions for the fade effect will be taken from this item.
     */
    void resetTransitionOnItem(ItemID id, TransitionType transition, ItemID boundId = render::Item::INVALID_ITEM_ID);

    /**
     * @brief The transaction will remove fade effect for a given item.
     *
     * If it was set, a transition finished operator will be called on the render thread when removal happens.
     * @param id Item ID.
     */
    void removeTransitionFromItem(ItemID id);

    /**
     * @brief The transaction will add a function to be called when fade effect finishes for a given item.
     *
     * Previous transition finished operators for the given item are not removed by this call, and all of them are called
     * on the render thread when fade effect finishes.
     * @param id Item ID.
     * @param func Function to be called.
     */
    void setTransitionFinishedOperator(ItemID id, TransitionFinishedFunc func);

    /**
     * @brief The transaction will add a function to be called on the render thread that will be provided with current state of the transition.
     *
     * @param id Item ID.
     * @param func Function to be called.
     */
    void queryTransitionOnItem(ItemID id, TransitionQueryFunc func);
   
    // Selection transactions

    /**
     * @brief The transaction will replace selection with a given name with the new one.
     *
     * @param selection Object containing selection name and list of item ids.
     */
    void resetSelection(const Selection& selection);

    /**
     * @brief The transaction will set a highlight style for a selection with a given name.
     *
     * @param selectionName Unique name of the selection.
     * @param style Object containing style properties.
     */
    void resetSelectionHighlight(const std::string& selectionName, const HighlightStyle& style = HighlightStyle());

    /**
     * @brief The transaction will remove highlight from a selection with specified name.
     *
     * @param selectionName Unique name of the selection.
     */
    void removeHighlightFromSelection(const std::string& selectionName);

    /**
     * @brief The transaction will add a function to be called on the render thread that will be provided with current
     *        highlight style for a given selection.
     *
     * @param selectionName Unique name of the selection.
     * @param func Function to be called.
     */
    void querySelectionHighlight(const std::string& selectionName, const SelectionHighlightQueryFunc& func);

    /**
     * @brief Reserves space needed to fit content of provided transactions.
     *
     * WARNING: this assumes that this transaction is empty at the time of `reserve()` call.
     * If the transaction is not empty, existing content will not be taken into account, so relying on this will
     * cause performance issues due to vector reallocation.
     * @param transactionContainer Vector containing transactions for which space needs to be reserved.
     */
    void reserve(const std::vector<Transaction>& transactionContainer);

    /**
     * @brief Adds content of given transactions to the current one.
     *
     * If the current transaction is empty, then proper amount of space for the content of new transactions will
     * be reserved. Otherwise, performance may suffer due to reallocation.
     * @param transactionContainer Transactions from which content will be added.
     */
    void mergeByCopying(const std::vector<Transaction>& transactionContainer);

    /**
     * @brief Moves content of given transactions to the current one and then clears input vector.
     *
     * If the current transaction is empty, then proper amount of space for the content of new transactions will
     * be reserved. Otherwise, performance may suffer due to reallocation.
     * @param transactionContainer Transactions from which content will be moved. This gets cleared after content is moved.
     */
    void mergeByMoving(std::vector<Transaction>&& transactionContainer);

    /**
     * @brief Adds content of given transactions to the current one.
     *
     * @param transaction Transaction from which contents will be copied.
     */
    void mergeByCopying(const Transaction& transaction);

    /**
     * @brief Moves content of given transactions to the current one and then clears input vector.
     *
     * @param transaction Transaction from which contents will be moved.
     */
    void mergeByMoving(Transaction&& transaction);

    /**
     * @brief Clears the content of this transaction.
     */
    void clear();

private:

    /**
     * @brief Adds a wrapped update function for an item with a given ID.
     *
     * Used internally by public `updateItem` functions.
     * @param id Item ID.
     * @param functor Wrapper over a function that will perform item update. Can be a `nullptr`.
     */
    void updateItem(ItemID id, const UpdateFunctorPointer& functor);

    using Reset = std::tuple<ItemID, PayloadPointer>;
    using Remove = ItemID;
    using Update = std::tuple<ItemID, UpdateFunctorPointer>;

    using TransitionReset = std::tuple<ItemID, TransitionType, ItemID>;
    using TransitionRemove = ItemID;
    using TransitionFinishedOperator = std::tuple<ItemID, TransitionFinishedFunc>;
    using TransitionQuery = std::tuple<ItemID, TransitionQueryFunc>;

    using SelectionReset = Selection;

    using HighlightReset = std::tuple<std::string, HighlightStyle>;
    using HighlightRemove = std::string;
    using HighlightQuery = std::tuple<std::string, SelectionHighlightQueryFunc>;

    using Resets = std::vector<Reset>;
    using Removes = std::vector<Remove>;
    using Updates = std::vector<Update>;

    using TransitionResets = std::vector<TransitionReset>;
    using TransitionRemoves = std::vector<TransitionRemove>;
    using TransitionFinishedOperators = std::vector<TransitionFinishedOperator>;
    using TransitionQueries = std::vector<TransitionQuery>;

    using SelectionResets = std::vector<SelectionReset>;

    using HighlightResets = std::vector<HighlightReset>;
    using HighlightRemoves = std::vector<HighlightRemove>;
    using HighlightQueries = std::vector<HighlightQuery>;

    Resets _resetItems;
    Removes _removedItems;
    Updates _updatedItems;

    TransitionResets _resetTransitions;
    TransitionRemoves _removeTransitions;
    TransitionFinishedOperators _transitionFinishedOperators;
    TransitionQueries _queriedTransitions;

    SelectionResets _resetSelections;

    HighlightResets _highlightResets;
    HighlightRemoves _highlightRemoves;
    HighlightQueries _highlightQueries;
};
typedef std::vector<Transaction> TransactionQueue;


/// @brief Scene is a container for Items.
/// Items are introduced, modified or erased in the scene through Transaction.
/// Once per Frame, the Transaction are all flushed.
/// During the flush the standard buckets are updated.
/// Items are notified accordingly on any update message happening.
class Scene {
public:
    /**
     * @param origin Coordinates of the beginning point of the octree. Minimum X, Y and Z position that an item may have.
     * @param size Size of the octree. All rendered entities must fit inside.
     */
    Scene(const glm::vec3 &origin, float size);
    ~Scene();

    /**
     * @brief Get a new, unique item ID.
     *
     * This call is thread safe, can be called from anywhere to allocate a new ID
     * @return New ID.
     */
    ItemID allocateID();

    /**
     * @brief Check that the ID is valid and allocated for this scene, this a threadsafe call
     * @param id ID to be checked.
     * @return `true` if ID was already assigned.
     */
    bool isAllocatedID(const ItemID& id) const;

    /**
     * @brief Get the total number of allocated items, this a threadsafe call
     *
     * @return Total number of allocated items.
     */
    size_t getNumItems() const { return _numAllocatedItems.load(); }

    /**
     * @brief Enqueue transaction to the scene.
     *
     * This is a thread-safe call.
     * @param transaction Transaction to enqueue.
     */
    void enqueueTransaction(const Transaction& transaction);

    /**
     * @brief Enqueue transaction to the scene by moving it.
     *
     * This is a thread-safe call.
     * @param transaction Transaction to enqueue.
     */
    void enqueueTransactionMove(Transaction&& transaction);

    /**
     * @brief Enqueue end of frame transactions boundary
     *
     * This is a thread-safe call.
     * @return New frame number.
     */
    uint32_t enqueueFrame();

    /**
     * @brief Process the pending transactions queued.
     *
     * Processes all queued frames.
     * Called on render thread on every frame and also on main thread during shutdown.
     */
    void processTransactionQueue();

    /**
     * @brief Access a particular selection (empty if doesn't exist)
     *
     * Thread safe.
     * @param name Unique name of the selection.
     * @return Object containing name and list of entities in the selection.
     */
    Selection getSelection(const Selection::Name& name) const;

    /**
     * @brief Check if a particular selection is empty (returns ).
     *
     * Thread safe.
     * @param name Unique name of the selection.
     * @return `true` if selection doesn't exist.
     */
    bool isSelectionEmpty(const Selection::Name& name) const;

    /**
     * @brief Add a single item to a selection by name
     *
     * Thread safe
     * @param selectionName Unique name of the selection.
     * @param itemID Item ID.
     */
    void addItemToSelection(const std::string& selectionName, ItemID itemID);

    /**
     * @brief Remove a selection by name
     *
     * Thread safe
     * @param selectionName Unique name of the selection.
     */
    void removeSelection(const std::string& selectionName);

    // Following call are NOT threadsafe, you have to call them from the correct thread to avoid any potential issues.

    /**
     * @brief Access a particular item from its ID.
     *
     * Not thread safe.
     * WARNING, There is no check on the validity of the ID, so this could return a bad Item.
     * @param id Item ID, must be valid.
     * @return Item with a given ID.
     */
    const Item& getItem(const ItemID& id) const { return _items[id]; }

    /**
     * @brief Same as getItem, checking if the id is valid
     *
     * Not thread safe.
     * @param id Item ID.
     * @return Item with a given ID.
     */
    const Item getItemSafe(const ItemID& id) const { if (isAllocatedID(id)) { return _items[id]; } else { return Item(); } }

    /**
     * @brief Access the spatialized items.
     *
     * Not thread safe.
     * @return Octree containing items.
     */
    const ItemSpatialTree& getSpatialTree() const { return _primarySpatialTree; }

    /**
     * @brief Access non-spatialized items (layered objects, backgrounds).
     *
     * Not thread safe.
     * @return Set of non-spatialized items.
     */
    const ItemIDSet& getNonspatialSet() const { return _primaryNonspatialSet; }

    /**
     * @brief Access a particular Stage (empty pointer if it doesn't exist).
     *
     * Thread safe.
     * @param name Stage name, as returned by getName() static function of a given stage type.
     * @return Shared pointer to the stage.
     */
    StagePointer getStage(const Stage::Name& name) const;

    /**
     * @brief Access stage of a given type.
     *
     * Thread safe.
     * @tparam T Type of the stage.
     * @param name Stage name, typically as returned by getName() static function of a given stage type.
     * @return Shared pointer to a requested stage.
     */
    template <class T>
    std::shared_ptr<T> getStage(const Stage::Name& name = T::getName()) const {
        auto stage = getStage(name);
        return (stage ? std::static_pointer_cast<T>(stage) : std::shared_ptr<T>());
    }

    /**
     * @brief Assign a given instance of a stage object to a given name.
     *
     * Used during setup when creating instances of the stage objects.
     * @param name Stage name, typically as returned by getName() static function of a given stage type.
     * @param stage Shared pointer to a stage object.
     */
    void resetStage(const Stage::Name& name, const StagePointer& stage);

    /**
     * @brief Perform renderer-side simulation of a given item.
     *
     * Not thread-safe.
     * Used for effects such as animating particle systems.
     * @param id Item ID.
     * @param args See Render::Args class documentation.
     */
    void simulate(ItemID id, RenderArgs* args) { _items[id].renderSimulate(args); }

    /**
     * @brief Returns outline style properties for a given item.
     *
     * Not thread-safe.
     * @param id Item ID.
     * @param viewFrustum Current view frustum from render args.
     * @param height Framebuffer height in pixels.
     * @return Object describing outline style.
     */
    HighlightStyle getOutlineStyle(ItemID id, const ViewFrustum& viewFrustum, uint16_t height) { return _items[id].getOutlineStyle(viewFrustum, height); }

private:
    /**
     * @brief Sets transition (fade effect) ID for a given item.
     *
     * Not thread-safe.
     * @param id Item ID.
     * @param transitionId Transition id, can be `render::TransitionStage::INVALID_INDEX`.
     */
    void setItemTransition(ItemID id, Index transitionId);

    /**
     * @brief Removes transition (fade effect) from an item.
     *
     * Not thread-safe.
     * @param id Item ID.
     */
    void removeItemTransition(ItemID id);


    // Thread safe elements that can be accessed from anywhere
    std::atomic<unsigned int> _IDAllocator{ 1 }; // first valid itemID will be One
    std::atomic<unsigned int> _numAllocatedItems{ 1 }; // num of allocated items, matching the _items.size()

    /// Must be locked before accessing transaction queue.
    std::mutex _transactionQueueMutex;

    /// Holds transactions that are currently gathered from other threads.
    /// Once frame is ready transactions are moved from here to `_transactionFrames`
    TransactionQueue _transactionQueue;

    /// Must be locked before accessing transaction frames.
    std::mutex _transactionFramesMutex;

    using TransactionFrames = std::vector<Transaction>;

    /// Contains consolidated transactions for individual frames.
    TransactionFrames _transactionFrames;

    /// Incremented every time a frame is enqueued.
    uint32_t _transactionFrameNumber{ 0 };

    /**
     * @brief Process one transaction frame.
     *
     * @param transaction for a given frame.
     */
    void processTransactionFrame(const Transaction& transaction);

    // The actual database

    /// Database of items is protected for editing by a mutex.
    std::mutex _itemsMutex;

    /// Stores all items.
    Item::Vector _items;

    /// Octree containing 3D items.
    ItemSpatialTree _primarySpatialTree;

    /// Set of 2D items, such as the background.
    ItemIDSet _primaryNonspatialSet;

    /**
     * @brief Applies transactions replacing payloads of given items with new ones.
     * @param transactions Item reset transactions to process.
     */
    void resetItems(const Transaction::Resets& transactions);

    /**
     * @brief Applies transactions that add a function to be called when fade effect finishes for a given item.
     * @param transactions  Transactions to process.
     */
    void resetTransitionFinishedOperator(const Transaction::TransitionFinishedOperators& transactions);

    /**
     * @brief Applies transactions removing items.
     * @param transactions Item removal transactions to process.
     */
    void removeItems(const Transaction::Removes& transactions);

    /**
     * @brief Applies transactions updating items.
     *
     * Updates spatial properties, key and executes custom update function if it was provided.
     * @param transactions Item update transactions to process.
     */
    void updateItems(const Transaction::Updates& transactions);

    /**
     * @brief Applies transactions starting new fade effects on items.
     * @param transactions Transactions resetting transitions on items.
     */
    void resetTransitionItems(const Transaction::TransitionResets& transactions);

    /**
     * @brief Applies transactions removing fade effect form items.
     * @param transactions Transactions removing transitions on items.
     */
    void removeTransitionItems(const Transaction::TransitionRemoves& transactions);

    /**
     * @brief Applies transactions querying current state of fade effects.
     * @param transactions Transactions providing functions to query transition states items.
     */
    void queryTransitionItems(const Transaction::TransitionQueries& transactions);

    /**
     * @brief Applies transactions that set a highlight style for a selection with a given name.
     * @param transactions Transactions resetting highlights to process.
     */
    void resetHighlights(const Transaction::HighlightResets& transactions);

    /**
     * @brief Applies transactions that remove highlight from a selection with specified name.
     * @param transactions Transactions removing highlights to process.
     */
    void removeHighlights(const Transaction::HighlightRemoves& transactions);

    /**
     * @brief Runs functions querying highlights.
     * @param transactions Transactions to query highlights.
     */
    void queryHighlights(const Transaction::HighlightQueries& transactions);

    /**
     * @brief Recursively adds all subitems of a given item to a given vector.
     * @param parentId ID of the item which subitems will be collected.
     * @param subItems Vector to which subitems will be added.
     */
    void collectSubItems(ItemID parentId, ItemIDs& subItems) const;

    // The Selection map
    mutable std::mutex _selectionsMutex; // mutable so it can be used in the thread safe getSelection const method
    SelectionMap _selections;

    /// Functions called when fade effect finishes.
    std::unordered_map<int32_t, std::vector<Transaction::TransitionFinishedFunc>> _transitionFinishedOperatorMap;

    /**
     * @brief Applies transactions that replace selection with a given name with the new one.
     * @param transactions Selection reset transactions to apply.
     */
    void resetSelections(const Transaction::SelectionResets& transactions);
    // More actions coming to selections soon:
    //  void removeFromSelection(const Selection& selection);
    //  void appendToSelection(const Selection& selection);
    //  void mergeWithSelection(const Selection& selection);

    // The Stage map
    /// Mutex needs to be locked before accessing stages.
    /// Mutable so it can be used in the thread safe getStage const method.
    mutable std::mutex _stagesMutex;

    /// Stages are stored here.
    StageMap _stages;


    friend class RenderEngine;
};

typedef std::shared_ptr<Scene> ScenePointer;
typedef std::vector<ScenePointer> Scenes;

}

#endif // hifi_render_Scene_h
