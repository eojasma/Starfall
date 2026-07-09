#include "GameScene.h"

USING_NS_AX; //pulls axmol ax namespace

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
    _dtLabel->setPosition(origin.x + visibleSize.width / 2,
                          origin.y + visibleSize.height / 2);

    this->addChild(_dtLabel);

    this->scheduleUpdate();

    return true;
}

void GameScene :: update(float dt)
{
    _elapsed += dt;

    _dtLabel->setString(StringUtils::format("dt: %.4f s | elapsed %.1f s",
                                             dt, _elapsed));
}


