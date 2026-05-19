#pragma once
#include "Entity.h"
#include <SFML/Graphics.hpp>

class Bullet : public Entity {
public:
    enum class Owner { PLAYER, ENEMY };

    Bullet(float x, float y, float vx, float vy, int dmg, bool pierce, Owner own);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;

    int  getDamage()    const { return damage; }
    bool isPiercing()   const { return piercing; }
    bool hasAlreadyPierced() const { return alreadyPierced; }
    void setPierced() { alreadyPierced = true; }
    Owner getOwner()    const { return owner; }

    // Previous frame centre position — used for swept collision
    float getPrevX() const { return prevX; }
    float getPrevY() const { return prevY; }

private:
    int   damage;
    bool  piercing;
    bool  alreadyPierced;
    Owner owner;
    sf::Color bulletColor;
    sf::RectangleShape shape;

    float prevX, prevY; // centre position from last frame
};