#include "GameScene.h"

USING_NS_AX; //pulls axmol ax namespace

namespace
{
static constexpr float kFixedStepTime = 1.0f / 120.0f;
static constexpr float kMaxFrame       = 0.25f;  // never simulate more than 0.25s of catch-up
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

    _world = Node::create();
    _hud   = Node::create();

    this->addChild(_world);
    this->addChild(_hud);

    _dtLabel = Label::createWithSystemFont("dt: --", "Arial", 24);
    _dtLabel->setPosition(origin.x + visibleSize.width / 2, origin.y);

    _hud->addChild(_dtLabel);

        
    _gpMgr = std ::make_unique<GameplayManager>();
    _plyr = std ::make_unique<Player>();
    

    _gameplayMgr = _gpMgr.get();
    _player = _plyr.get();


    _player->init();

    _world->addChild(_player->getRenderNode());

    _gameplayMgr->setPlayer(_player);

    _gameplayMgr->init(_world, _hud, visibleSize);
    
    auto keys           = EventListenerKeyboard::create();
    keys->onKeyPressed  = [this](EventKeyboard::KeyCode code, Event*) { _gameplayMgr->onKeyDown(code); };
    keys->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) { _gameplayMgr->onKeyUp(code); };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(keys, this);

    this->scheduleUpdate();

    return true;
}

void GameScene::fixedUpdate(float dt) 
{
    _gameplayMgr->update(dt);
}

void GameScene :: update(float dt)
{
    _elapsed += dt;
    //_accumulatedTime += dt;
    _accumulatedTime += std::min(kMaxFrame, dt);

    _dtLabel->setString(StringUtils::format("dt: %.4f s | elapsed %.1f s",
                                             dt, _elapsed));

    while (_accumulatedTime > kFixedStepTime)
    {
        fixedUpdate(static_cast<float>(kFixedStepTime));

        _accumulatedTime -= kFixedStepTime;
    }

    const double alpha = _accumulatedTime / kFixedStepTime;

    renderInterpolated(alpha);
}

void GameScene::renderInterpolated(double alpha) {}  // stub; real lerp is the stretch goal



GameScene::~GameScene()
{
   
}
