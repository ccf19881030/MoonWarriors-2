#include "GameLayer.h"
#include "SimpleAudioEngine.h"
#include "Bullet.h"
#include "Resource.h"
#include "Config.h"
#include "Enemy.h"
#include "Effect.h"
#include "GameOver.h"
#include "PauseLayer.h"


using namespace cocos2d;
using namespace CocosDenshion;


bool isPaused = false;

GameLayer::GameLayer():m_state(statePlaying),m_time(0),m_ship(NULL),m_backSky(NULL),m_backSkyHeight(0),m_backSkyRe(NULL),m_backTileMap(NULL),m_backTileMapHeight(0),m_backTileMapRe(NULL),m_isBackSkyReload(false),m_isBackTileReload(false),m_lbScore(NULL),m_lifeCount(NULL),
    m_tempScore(0)
{
    
}
GameLayer::~GameLayer()
{
    if (m_levelManager) {
        delete m_levelManager;
    }
    
    play_bullet->release();
    enemy_bullet->release();
    enemy_items->release();
}

bool GameLayer::init()
{
    if (!CCLayer::init()) {
        return false;
    }
    // ��������
    this->setTouchEnabled(true);
    
    // �������飬��Ҫretainһ��
    play_bullet = CCArray::create();
    play_bullet->retain();
    
    enemy_bullet = CCArray::create();
    enemy_bullet->retain();
    
    enemy_items = CCArray::create();
    enemy_items->retain();
    
    m_state = statePlaying;//statePlaying=0
    
    Enemy::sharedEnemy();
    Effect::sharedExplosion();
    
    Config::sharedConfig()->resetConfig();
    
    winSize = CCDirector::sharedDirector()->getWinSize();
    m_levelManager = new LevelManager(this);
    
    //��ʼ������
    initBackground();

    m_screenRec = CCRectMake(0, 0,  winSize.width, winSize.height + 10);
    
    // score
    m_lbScore = CCLabelBMFont::create("Score:0", s_arial14_fnt);
    m_lbScore->setAnchorPoint(ccp(1, 0));
    m_lbScore->setAlignment(kCCTextAlignmentRight);
    addChild(m_lbScore, 1000);
    m_lbScore->setPosition(winSize.width - 5, winSize.height - 30);
    
    // ship life
    CCTexture2D *shipTexture = CCTextureCache::sharedTextureCache()->addImage(s_ship01);
    CCSprite *life = CCSprite::createWithTexture(shipTexture, CCRectMake(0, 0, 60, 38));
    life->setScale(0.6);
    life->setPosition(ccp(30,winSize.height-23));
    addChild(life, 1, 5);
    
    // ship life count
    char lifecount[2];
    sprintf(lifecount, "%d",Config::sharedConfig()->getLifeCount());
    m_lifeCount = CCLabelTTF::create(lifecount, "Arial", 20);
    m_lifeCount->setPosition(ccp(60, winSize.height-20));
    m_lifeCount->setColor(ccc3(255,0, 0));
    addChild(m_lifeCount, 1000);
    
    // ship
    m_ship = Ship::create();
    addChild(m_ship, m_ship->getZoder(), 1001);
    
    CCMenuItemImage *pause = CCMenuItemImage::create("pause.png", "pause.png", this, menu_selector(GameLayer::doPause));
    pause->setAnchorPoint(ccp(1, 0));
    pause->setPosition(ccp(winSize.width, 0));
    CCMenu *menu = CCMenu::create(pause, NULL);
    menu->setAnchorPoint(ccp(0, 0));
    addChild(menu, 1, 10);
    menu->setPosition(CCPointZero);
    
    // �� update����
    scheduleUpdate();
    
    // ÿ���һ�� scoreCounter����
    schedule(schedule_selector(GameLayer::scoreCounter), 1);
    
    if (Config::sharedConfig()->getAudioState()) {
        SimpleAudioEngine::sharedEngine()->playBackgroundMusic(s_bgMusic, true);
    }
    
    return true;
}



void GameLayer::update(float dt)
{
    if (m_state == statePlaying) {
        checkIsCollide();
        removeInactiveUnit(dt);
        checkIsReborn();
        updateUI();
    }
    
}

void GameLayer::scoreCounter(float dt)
{
    if (m_state == statePlaying) {
        m_time++;
        m_levelManager->loadLevelResource(m_time);
    }
}

void GameLayer::checkIsCollide()
{
    CCObject *units;
    CCObject *bullets;
    CCObject *enemybs;
	//�����������ÿ������
    CCARRAY_FOREACH(enemy_items, units)
    {
        UnitSprite *enemy = dynamic_cast<UnitSprite*>(units);
		//��������������ǵ��ӵ�
        CCARRAY_FOREACH(play_bullet, bullets)
        {
            UnitSprite *bullet = dynamic_cast<UnitSprite*>(bullets);
            if (this->collide(enemy, bullet)) {//�жϵ��˺��ӵ�
                enemy->hurt();
                bullet->hurt();
            }
			//Խ��ɾ��
            if (!(m_screenRec.intersectsRect(bullet->boundingBox()))) {
                bullet->destroy();
            }
        }
        if (collide(enemy, m_ship)) {//�жϵ��˺�����
            if (m_ship->isActive()) {
                enemy->hurt();
                m_ship->hurt();
            }
            
        }
        if (!(m_screenRec.intersectsRect(enemy->boundingBox()))) {
            enemy->destroy();
        }
    }
    //�����ÿ�����˵��ӵ�
    CCARRAY_FOREACH(enemy_bullet, enemybs)
    {
        UnitSprite *enemyb = dynamic_cast<UnitSprite*>(enemybs);
        if (enemyb) {
            if (collide(enemyb, m_ship)) {//�жϵ����ӵ�������
                if (m_ship->isActive()) {
                    enemyb->hurt();
                    m_ship->hurt();
                }
            }
            if (!m_screenRec.intersectsRect(enemyb->boundingBox())) {
                enemyb->destroy();
            }
        }
        
    }
}

void GameLayer::removeInactiveUnit(float dt)
{
        
    CCArray *children = this->getChildren();
    for (int i = 0; i < children->count(); ++i) 
	{
        CCSprite *selChild =  dynamic_cast<CCSprite*>(children->objectAtIndex(i));
        if (selChild) 
		{
            selChild->update(dt);
            int tag = selChild->getTag();

            if (( tag == 900) || (tag == 901 )|| (tag == 1000)) 
			{
                if (!((UnitSprite*)selChild)->isActive())
				{
                    ((UnitSprite*)selChild)->destroy();
                }
            }
        }
    }
    
    if (m_ship) {
        if (!m_ship->isActive()) {
            m_ship->destroy();
            m_ship = NULL;
        }
    }
    
}

void GameLayer::checkIsReborn()
{    
    if (Config::sharedConfig()->getLifeCount() > 0) {
        if (!m_ship) {
            
                m_ship = Ship::create();
                this->addChild(m_ship, m_ship->getZoder(), 1001);
        }
        
    }
    else if (Config::sharedConfig()->getLifeCount() <= 0 ) {
        m_state = stateGameOver;
        m_ship = NULL;
        CCCallFunc *gameOver = CCCallFunc::create(this, callfunc_selector(GameLayer::gameOver));
        this->runAction(CCSequence::create(CCDelayTime::create(0.2), gameOver, NULL));
    }
    
}

bool GameLayer::collide(UnitSprite *a, UnitSprite *b)
{
    if(!a || !b)
    {
        return false;
    }
    CCRect aRect = a->collideRect();
    CCRect bRect = b->collideRect();
    if (aRect.intersectsRect(bRect)) {
        return true;
    }
    return false;
}

void GameLayer::updateUI()
{
    if (m_tempScore < Config::sharedConfig()->getScoreValue()) {
        m_tempScore += 5;
    }
 //   char score[20];
   // char s[] = "Score:";
  //  sprintf(score, "%d", m_tempScore);
  //  m_lbScore->setString(strcat(s, score));
    char lifecount[2];
    sprintf(lifecount, "%d",Config::sharedConfig()->getLifeCount());
    m_lifeCount->setString(lifecount);
    
}

void GameLayer::onEnter()
{
    CCDirector* pDirector = CCDirector::sharedDirector();
    pDirector->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
    CCLayer::onEnter();
}

void GameLayer::onExit()
{
    CCDirector* pDirector = CCDirector::sharedDirector();
    pDirector->getTouchDispatcher()->removeDelegate(this);
    CCLayer::onExit();
}


bool GameLayer::ccTouchBegan(CCTouch *touch, CCEvent *event)
{
//    CCLog("touch began!");
    return true;
}

void GameLayer::ccTouchMoved(cocos2d::CCTouch *touch, cocos2d::CCEvent *event)
{
    if ((m_state == statePlaying) && m_ship) {
    CCPoint pos = touch->getDelta();
    CCPoint currentPos = m_ship->getPosition();
    currentPos = ccpAdd(currentPos, pos);
    currentPos = ccpClamp(currentPos, CCPointZero, ccp(winSize.width, winSize.height));
    m_ship->setPosition(currentPos);
    }
    
}

void GameLayer::ccTouchEnded(cocos2d::CCTouch *touch, cocos2d::CCEvent *event)
{
//    CCLog("touch end!");
}

// ���޹�����ͼ����������ͼѭ�����ع���
void GameLayer::initBackground()
{
    m_backSky = CCSprite::create(s_bg01);
    m_backSky->setAnchorPoint(ccp(0, 0));
    m_backSkyHeight = m_backSky->getContentSize().height;
    addChild(m_backSky, -10);
    
    // Tile map
    m_backTileMap = CCTMXTiledMap::create(s_level01);
    addChild(m_backTileMap, -9);
    m_backTileMapHeight = m_backTileMap->getMapSize().height * m_backTileMap->getTileSize().height;
    
    
    
    m_backSkyHeight -= 48;
    m_backTileMapHeight -= 200;
    m_backSky->runAction(CCMoveBy::create(3, ccp(0, -48)));
    m_backTileMap->runAction(CCMoveBy::create(3, ccp(0, -200)));
    
    schedule(schedule_selector(GameLayer:: movingBackground),3);
    
}

// ��������Ӳ����
void GameLayer::movingBackground(float dt)
{
    m_backSky->runAction(CCMoveBy::create(3, ccp(0, -48)));
    m_backTileMap->runAction(CCMoveBy::create(3, ccp(0, -200)));
    
    // ÿ���ƶ�48
    m_backSkyHeight -= 48;
    
    // ÿ���ƶ�200
    m_backTileMapHeight -= 200;
    

    // ͼ�Ķ���������Ļ����ʱ
    if (m_backSkyHeight <= winSize.height) {
        if (!m_isBackSkyReload) {
            
            // �����һ��ͼ��û������createһ��
            m_backSkyRe = CCSprite::create(s_bg01);
            m_backSkyRe->setAnchorPoint(ccp(0, 0));
            addChild(m_backSkyRe, -10);
            m_backSkyRe->setPosition(ccp(0, winSize.height));
            
            // ��ת��־λ
            m_isBackSkyReload = true;
        }
        // �ڶ���ͼ�����ŵ�һ��ͼ����
        m_backSkyRe->runAction(CCMoveBy::create(3, ccp(0, -48)));
    }
    
    // ��һ��ͼ��ȫ������Ļ
    if (m_backSkyHeight <= 0) {
        m_backSkyHeight = m_backSky->getContentSize().height;
        // �Ƴ���һ�ŵľ���
        this->removeChild(m_backSky, true);
        
        // ָ��ڶ���ͼ�ľ���
        m_backSky = m_backSkyRe;
        
        // �ڶ��ŵľ���ָ���ÿ�
        m_backSkyRe = NULL;
        
        // ��ת��־λ
        m_isBackSkyReload = false;
    }
    
    if (m_backTileMapHeight <= winSize.height) {
        if (!m_isBackTileReload) {
            m_backTileMapRe = CCTMXTiledMap::create(s_level01);
            this->addChild(m_backTileMapRe, -9);
            m_backTileMapRe->setPosition(0, winSize.height);
            m_isBackTileReload = true;
        }
        m_backTileMapRe->runAction(CCMoveBy::create(3, ccp(0, -200)));
    }
    
    if (m_backTileMapHeight <= 0) {
        m_backTileMapHeight = m_backTileMap->getMapSize().height * m_backTileMap->getTileSize().height;
        this->removeChild(m_backTileMap, true);
        m_backTileMap = m_backTileMapRe;
        m_backTileMapRe = NULL;
        m_isBackTileReload = false;
    }
}

void GameLayer::gameOver()
{
    CCScene * scene = GameOver::scene();
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(1.2, scene));
}

void GameLayer::doPause(CCObject *pSender)
{
    CCDirector::sharedDirector()->pause();
    SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->pauseAllEffects();
    PauseLayer *pauseLayer = PauseLayer::create();
    addChild(pauseLayer,9999);
}

Ship* GameLayer::getShip()
{
    return m_ship;
}