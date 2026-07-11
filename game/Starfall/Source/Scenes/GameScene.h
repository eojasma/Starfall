#pragma once
#include "axmol.h"
#include "GameplayManager.h"
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

    void renderInterpolated(double alpha);

    private:

        double _elapsed         = 0.0;
        double _accumulatedTime = 0.0f;

        /// <summary>
        /// observing pointer - scene owns it via addChild
        /// </summary>
        ax :: Label* _dtLabel = nullptr;

        GameplayManager* _gameplayMgr = nullptr;

        Player* _player = nullptr;

};
