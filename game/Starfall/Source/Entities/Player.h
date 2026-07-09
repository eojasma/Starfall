#pragma once
#include "axmol.h"
#include <unordered_set>

class Player : public ax::Node
{
public:
    static Player* create();

    bool init();
    void update(float dt);

    void onKeyDown(ax::EventKeyboard::KeyCode code);
    void onKeyUp(ax::EventKeyboard::KeyCode code);

private:
    ax ::Sprite* _sprite = nullptr;

    /// <summary>
    /// holds the currently held keys
    /// </summary>
    std::unordered_set<ax::EventKeyboard::KeyCode> _held;

    /// <summary>
    /// px/sec
    /// </summary>
    float _speed = 320.0f;
};
