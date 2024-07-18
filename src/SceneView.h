#pragma once

#include "Scene.h"

template<typename... ComponentTypes>
struct SceneView {
    Scene *pScene { nullptr };
    ComponentMask componentMask;
    bool all { false };

    SceneView() {
        if (sizeof...(ComponentTypes) == 0) {
            all = true;
        } else {
            // Unpack the template parameters into an initializer list
            int componentIds[] = { 0, GetId<ComponentTypes>()... };
            for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++) {
                componentMask.set(componentIds[i]);
            }
        }
    }

    explicit SceneView(Scene *scene) : pScene(scene) {
        if (sizeof...(ComponentTypes) == 0) {
            all = true;
        } else {
            // Unpack the template parameters into an initializer list
            int componentIds[] = { 0, GetId<ComponentTypes>()... };
            for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++) {
                componentMask.set(componentIds[i]);
            }
        }
    }

    struct Iterator {
        EntityIndex index;
        Scene* pScene;
        ComponentMask mask;
        bool all { false };

        Iterator(Scene* scene, EntityIndex index, ComponentMask mask, bool all) : pScene(scene), index(index), mask(mask), all(all) {}

        EntityID operator*() const {
            // give back the entityID we're currently at
            return pScene->entities[index].id;
        }

        bool operator==(const Iterator& other) const {
            // Compare two iterators
            return index == other.index || index == pScene->entities.size();
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index && index != pScene->entities.size();
        }

        Iterator& operator++() {
            // Move the iterator forward
            do {
                index++;
            } while (index < pScene->entities.size() && !ValidIndex()); 
            return *this;
        }

        bool ValidIndex() {
            return isEntityValid(pScene->entities[index].id) && (all || (mask & pScene->entities[index].mask).any());
        }
    };

    Iterator begin() const {
        // Give an iterator to the beginning of this view
        int firstIndex = 0;
        while (firstIndex < pScene->entities.size() && (componentMask != (componentMask & pScene->entities[firstIndex].mask) || !isEntityValid(pScene->entities[firstIndex].id))) {
            firstIndex++;
        }
        return Iterator(pScene, firstIndex, componentMask, all);
    }

    Iterator end() const
    {
        return Iterator(pScene, EntityIndex(pScene->entities.size()), componentMask, all);
    }
};
