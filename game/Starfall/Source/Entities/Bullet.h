#pragma once
#include "axmol.h"

/// <summary>
/// struct for holding bullet data, kept in a pool to be reused and avoid instantiating
/// </summary>
struct Bullet
{
    ax::Vec2 vel{};
    ax::Vec2 pos{};
    ax::Node* node = nullptr;
};
