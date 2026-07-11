#include "GameScene.h"

USING_NS_AX; //pulls axmol ax namespace

namespace
{
static constexpr float kFixedStepTime = 1.0f / 120.0f;

}

GameScene* GameScene::create()
{
    GameScene* p = new (std::nothrow) GameScene();

    if (p && p->init())
    {
        p->autorelease();
        return p;
    }

    AX_SAFE_DELETE(p);
    return nullptr;
}

bool GameScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto* director         = Director::getInstance();
    const auto visibleSize = director->getVisibleSize();
    const Vec2 origin      = director->getVisibleOrigin();

    _dtLabel = Label::createWithSystemFont("dt: --", "Arial", 24);
    _dtLabel->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);

    this->addChild(_dtLabel);

    _gameplayMgr = new GameplayManager();

    _player = Player::create();
    this->addChild(_player);

    _gameplayMgr->setPlayer(_player);

    _gameplayMgr->init(this, visibleSize);
    
    auto keys           = EventListenerKeyboard::create();
    keys->onKeyPressed  = [this](EventKeyboard::KeyCode code, Event*) { _gameplayMgr->onKeyDown(code); };
    keys->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) { _gameplayMgr->onKeyUp(code); };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(keys, this);

    this->scheduleUpdate();

    return true;
}

void GameScene :: update(float dt)
{
    _elapsed += dt;
    _accumulatedTime += dt;

    _dtLabel->setString(StringUtils::format("dt: %.4f s | elapsed %.1f s",
                                             dt, _elapsed));

    while (_accumulatedTime > kFixedStepTime)
    {
        fixedUpdate(static_cast<float>(kFixedStepTime));

        _gameplayMgr->update(kFixedStepTime);

        _accumulatedTime -= kFixedStepTime;
    }

    const double alpha = _accumulatedTime / kFixedStepTime;

    renderInterpolated(alpha);
}

void GameScene::renderInterpolated(double alpha) {}  // stub; real lerp is the stretch goal


