#pragma once
#include <cstddef>
#include <unordered_map>

#include "Cell.h"
#include "CellHash.h"
#include "axmol.h"

#include "Entities/Enemy.h"

class SpatialGrid
{
public:
    explicit SpatialGrid(float cellSize);

    void Clear();

    void Insert(Enemy* enemy);

    const std::vector<Enemy*>* GetCell(int x, int y) const;

    Cell ToCell(ax::Vec2 position) const;

    float GetCellSize() const;

private:
    float cellSize_;

    std::unordered_map<Cell, std::vector<Enemy*>, CellHash> cells_;
};
