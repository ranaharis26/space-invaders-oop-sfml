#pragma once
#include "Boss.h"
#include "Seeker.h"
#include <SFML/Graphics.hpp>

class Mothership : public Boss {
public:
    Mothership(float screenW, float playerX);
    virtual ~Mothership() {}

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    Bullet** attack(int& count) override;
    Enemy** spawnEnemies(int& count) override;

    void setPlayerX(float px) { playerXRef = px; }
    float getLaserY()    const { return position.y + height / 2.f + 10.f; }
    bool isLaserFiring() const { return laserFiring; }

private:
    float screenWidth;
    float playerXRef;
    float sweepSpeed;
    bool  movingRight;

    bool  laserWarning;
    float laserWarningTimer;
    bool  laserFiring;
    float laserFireTimer;
    float laserFireDuration;

    bool  aimedShotWarning;
    float aimedShotWarningTimer;
    float aimedShotX;
    bool  pendingAimedShot;

    float fireFlicker;
    float seekerSpawnTimer;
    float seekerSpawnInterval;

    sf::RectangleShape bodyRect;
    sf::ConvexShape    wingL, wingR;
    sf::CircleShape    core;
    sf::RectangleShape laserOuter;
    sf::RectangleShape laserMid;
    sf::RectangleShape laserCore;
    sf::RectangleShape warnRect;
    sf::RectangleShape aimedWarnRect;

    void buildSprite();
    void drawFireLaser(sf::RenderWindow& window, float laserY);
    void drawLaserWarning(sf::RenderWindow& window, float laserY);
    void drawAimedWarning(sf::RenderWindow& window);
};