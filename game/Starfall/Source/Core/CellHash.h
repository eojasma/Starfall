#pragma once

#include <cstddef>
#include "Cell.h"

struct CellHash
{
    std::size_t operator()(const Cell& cell) const
    {
        const std::size_t x = static_cast<std::size_t>(cell.x);
        const std::size_t y = static_cast<std::size_t>(cell.y);

        return x * 73856093u ^ y * 19349663u;
    }
};
