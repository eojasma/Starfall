#pragma once

#include "axmol.h"

/// <summary>
/// struct for holding visual entity data, kept in a pool to be reused and avoid instantiating
/// </summary>
struct VisualEntity
{
    ax::Vec2 vel{};
    ax::Vec2 pos{};
    float radius   = 0.0f;
    ax::Node* node = nullptr;
    bool pendingRemoval = false;
};
