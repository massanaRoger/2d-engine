#pragma once

#include <vector>
#include <bitset>

#include "Components.h"
#include "ComponentPool.h"

typedef unsigned long long EntityID;
static constexpr int MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

typedef unsigned int EntityIndex;
typedef unsigned int EntityVersion;
typedef unsigned long long EntityID;

inline EntityID CreateEntityId(EntityIndex index, EntityVersion version) {
    // Index on top, version on bottom
    return ((EntityID) index << 32) | ((EntityID) version);
}

inline EntityIndex GetEntityIndex(EntityID id) {
    // Shift down 32 to get our index
    return id >> 32;
}

inline EntityVersion GetEntityVersion(EntityID id) {
    // Cast to lose index and get only the version
    return (EntityVersion)id;
}

inline bool isEntityValid(EntityID id) {
    // Check if index is on the valid range
    return (id >> 32) != EntityIndex(-1);
}

struct Scene {
    struct EntityDesc {
        EntityID id;
        ComponentMask mask;
    };
    std::vector<EntityDesc> entities;
    std::vector<ComponentPool> componentPools;

    EntityID NewEntity() {
        if (!freeEntities.empty()) {
            EntityIndex newIndex = freeEntities.back();
            freeEntities.pop_back();
            EntityID newID = CreateEntityId(newIndex, GetEntityVersion(entities[newIndex].id));
            entities[newIndex].id = newID;
        }
        entities.push_back({ CreateEntityId(EntityIndex(entities.size()), 0), ComponentMask() });
        return entities.back().id;
    }

    template<typename T>
    void Assign(EntityID id) {
        int componentId = GetId<T>();
        entities[GetEntityIndex(id)].mask.set(componentId);
    }

    template<typename T>
    T *Assign(EntityID id) {
        int componentId = GetId<T>();

        if (componentPools.size() <= componentId) {
            // Not enough component pool 
            componentPools.resize(componentId + 1, nullptr);
        }
        
        if (componentPools[componentId] == nullptr) {
            // New component, create new pool
            componentPools[componentId] = new ComponentPool(sizeof(T));
        }

        // Looks up the component in the pool, and initializes it with placement new
        T* pComponent = new (componentPools[componentId].get(GetEntityIndex(id))) T();

        // Set the bit for the component to true
        entities[GetEntityIndex(id)].mask.set(componentId);
        return pComponent;
    }

    template<typename T>
    T* Get(EntityID id) {
        int componentId = GetId<T>();
        if (!entities[GetEntityIndex(id)].mask.test(componentId)) {
            return nullptr;
        }

        T* pComponent = static_cast<T*>(componentPools[componentId].get(GetEntityIndex(id)));
        return pComponent;
    }

    template<typename T>
    void RemoveComponentFromEntity(EntityID id) {
        // Ensure we are not accessing removed entity
        if (entities[GetEntityIndex(id)].id != id) {
            return;
        }

        int componentId = GetId<T>();
        entities[GetEntityIndex(id)].mask.reset(componentId);
    }

    void DestroyEntity(EntityID id) {
        EntityID emptyID = CreateEntityId(EntityIndex(-1), GetEntityVersion(id) + 1);
        entities[GetEntityIndex(id)].id = emptyID;
        entities[GetEntityIndex(id)].mask.reset();
        freeEntities.push_back(GetEntityIndex(id));
    }
};



#define INVALID_ENTITY CreateEntityId(EntityIndex(-1), 0)
