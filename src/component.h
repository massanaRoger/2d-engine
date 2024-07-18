#pragma once

extern int s_componentCounter;

template <class T>
int GetId() {
    static int s_componentId = s_componentCounter++;
    return s_componentId;
}