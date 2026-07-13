#include "SpatialGrid.h"


SpatialGrid::SpatialGrid(float cellSize) : cellSize_(cellSize) {}

void SpatialGrid::Clear()
 {
    cells_.clear();
}

void SpatialGrid::Insert(Enemy* enemy)
{
    const ax::Vec2 minimum = {enemy->pos.x - enemy->radius, enemy->pos.y - enemy->radius};

    const ax::Vec2 maximum = {enemy->pos.x + enemy->radius, enemy->pos.y + enemy->radius};

    const Cell minCell = ToCell(minimum);
    const Cell maxCell = ToCell(maximum);

    for (int y = minCell.y; y <= maxCell.y; ++y)
    {
        for (int x = minCell.x; x <= maxCell.x; ++x)
        {
            cells_[{x, y}].push_back(enemy);
        }
    }
}

const std::vector<Enemy*>* SpatialGrid::GetCell(int x, int y) const
{
    const auto it = cells_.find({x, y});

    if (it == cells_.end())
        return nullptr;

    return &it->second;
}

Cell SpatialGrid::ToCell(ax::Vec2 position) const
{
    return {static_cast<int>(std::floor(position.x / cellSize_)), static_cast<int>(std::floor(position.y / cellSize_))};
}

float SpatialGrid::GetCellSize() const
{
    return cellSize_;
}
