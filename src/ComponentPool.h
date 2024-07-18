#pragma once
#include <cstddef>
static constexpr int MAX_ENTITIES = 1000;

struct ComponentPool {
    char *pData{ nullptr };
    size_t elementSize = 0;

    explicit ComponentPool(size_t elementSize): elementSize(elementSize) {
        pData = new char[elementSize * MAX_ENTITIES];
    }

    ~ComponentPool() {
        delete[] pData;
    }

    void* get(size_t index) {
        return pData + index * elementSize;
    }
};
