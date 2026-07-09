#pragma once
#include "axmol.h"
#include "Entities/Player.h"

class GameScene :public ax::Scene
{
public:
    /// <summary>
    /// factory
    /// </summary>
    /// <returns>auto released pointer to new instance of GameScene</returns>
    static GameScene* create();

    /// <summary>
    /// real constructor function
    /// </summary>
    /// <returns>bool isInitialized</returns>
    bool init() override;

    /// <summary>
    /// per-frame tick
    /// </summary>
    /// <param name="dt">delta time: seconds since last frame</param>
    void update(float dt) override;

    private:
        /// <summary>
        /// observing pointer - scene owns it via addChild
        /// </summary>
        ax :: Label* _dtLabel = nullptr;

        Player* _player = nullptr;

        double _elapsed = 0.0;
};
