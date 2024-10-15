//
//  Stage.h
//  render/src/render
//
//  Created by Sam Gateau on 6/14/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_Stage_h
#define hifi_render_Stage_h

#include <map>
#include <memory>
#include <unordered_map>
#include <string>

#include "IndexedContainer.h"

namespace render {

    using ElementIndices = std::vector<indexed_container::Index>;

    class Stage {
    public:
        Stage() {}
        virtual ~Stage() {}

        using Name = std::string;
        using Index = indexed_container::Index;
        static const Index INVALID_INDEX;
        using IDList = indexed_container::Indices;

        static bool isIndexInvalid(Index index) { return index == INVALID_INDEX; }
    };

    using StagePointer = std::shared_ptr<Stage>;
    using StageMap = std::map<const Stage::Name, StagePointer>;

    template<typename T>
    class TypedStage : public Stage {
    public:
        TypedStage() {}
        virtual ~TypedStage() {}

        static const Name& getName() { return _name; }

        bool checkId(Index index) const { return _elements.checkIndex(index); }

        const T& getElement(Index id) const { return _elements.get(id); }
        T& editElement(Index id) { return _elements.edit(id); }
        Index addElement(const T& element) {
            Index id = _elements.newElement(element);
            _activeElementIDs.push_back(id);
            return id;
        }

        void removeElement(Index index) {
            IDList::iterator idIterator = std::find(_activeElementIDs.begin(), _activeElementIDs.end(), index);
            if (idIterator != _activeElementIDs.end()) {
                _activeElementIDs.erase(idIterator);
            }
            if (!_elements.isElementFreed(index)) {
                _elements.freeElement(index);
            }
        }

        IDList::iterator begin() { return _activeElementIDs.begin(); }
        IDList::iterator end() { return _activeElementIDs.end(); }
    protected:
        static Name _name;

        using Elements = indexed_container::IndexedVector<T>;

        Elements _elements;
        IDList _activeElementIDs;
    };

    class Frame {
    public:
        Frame() {}

        using Index = indexed_container::Index;

        void clear() { _elements.clear(); }
        void pushElement(Index index) { _elements.emplace_back(index); }

        ElementIndices _elements;
    };

    template<typename T, typename P, typename F = Frame>
    class PointerStage : public Stage {
    public:
        PointerStage() {}
        virtual ~PointerStage() {}

        static const Name& getName() { return _name; }

        bool checkId(Index index) const { return _elements.checkIndex(index); }

        Index getNumElements() const { return _elements.getNumElements(); }
        Index getNumFreeElements() const { return _elements.getNumFreeIndices(); }
        Index getNumAllocatedElements() const { return _elements.getNumAllocatedIndices(); }

        P getElement(Index id) const { return _elements.get(id); }

        Index findElement(const P& element) const {
            auto found = _elementMap.find(haze);
            if (found != _elementMap.end()) {
                return INVALID_INDEX;
            } else {
                return (*found).second;
            }
        }

        virtual Index addElement(const P& element) {
            auto found = _elementMap.find(element);
            if (found == _elementMap.end()) {
                auto id = _elements.newElement(element);
                // Avoid failing to allocate an element, just pass
                if (id != INVALID_INDEX) {
                    // Insert the element and its index in the reverse map
                    _elementMap.insert(ElementMap::value_type(element, id));
                }
                return id;
            } else {
                return (*found).second;
            }
        }

        virtual P removeElement(Index index) {
            P removed = _elements.freeElement(index);

            if (removed) {
                _elementMap.erase(removed);
            }
            return removed;
        }

        using Frame = F;
        using FramePointer = std::shared_ptr<F>;
        F _currentFrame;

    protected:
        static Name _name;

        using Elements = indexed_container::IndexedPointerVector<T>;
        using ElementMap = std::unordered_map<P, Index>;

        Elements _elements;
        ElementMap _elementMap;
    };
}

#endif // hifi_render_Stage_h
