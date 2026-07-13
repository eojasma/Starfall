#pragma once

struct Cell
{
    int x;
    int y;

    bool operator==(const Cell& rhs) const { return x == rhs.x && y == rhs.y; }
};
