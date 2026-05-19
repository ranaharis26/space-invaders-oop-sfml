#pragma once
#include "Boss.h"
#include "Bullet.h"
#include <SFML/Graphics.hpp>

class Cruiser : public Boss {
public:
    explicit Cruiser(float screenW);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    Bullet** attack(int& count) override;

private:
    void buildSprite();
    void drawShip(sf::RenderWindow& window);

    float screenWidth;
    float sweepSpeed;
    bool  movingRight;

    int   safeGap;
    bool  laserWarning;
    float warningTimer;
    bool  firingNow;

    // ship body parts
    sf::RectangleShape body;        // main green square hull
    sf::RectangleShape topPanel;    // dark strip along top
    sf::RectangleShape botPanel;    // dark strip along bottom
    sf::RectangleShape wingL;       // left wing
    sf::RectangleShape wingR;       // right wing
    sf::RectangleShape engineL;     // left engine block
    sf::RectangleShape engineR;     // right engine block
    sf::RectangleShape antennaL;    // left antenna
    sf::RectangleShape antennaR;    // right antenna
    sf::CircleShape    antTipL;     // antenna tip
    sf::CircleShape    antTipR;

    // windows (3 across the hull)
    sf::RectangleShape win[3];
    sf::RectangleShape winGlow[3];  // inner glow of window

    // core pulsing circle in centre
    sf::CircleShape    core;

    // engine glow
    sf::RectangleShape engGlowL;
    sf::RectangleShape engGlowR;
};