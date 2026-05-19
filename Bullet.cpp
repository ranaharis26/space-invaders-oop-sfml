#include "Bullet.h"
#include <SFML/Graphics.hpp>

Bullet::Bullet(float x, float y, float vx, float vy, int dmg, bool pierce, Owner own)
    : Entity(x, y, 6.f, 16.f), damage(dmg), piercing(pierce),
    alreadyPierced(false), owner(own),
    prevX(x), prevY(y)   // initialise prev to spawn position
{
    velocity = { vx, vy };
    if (owner == Owner::PLAYER) {
        bulletColor = (pierce) ? sf::Color(255, 80, 80) : sf::Color(80, 220, 255);
    }
    else {
        bulletColor = sf::Color(255, 60, 200);
    }
    shape.setSize({ width, height });
    shape.setOrigin({width / 2.f, height / 2.f});
    shape.setFillColor(bulletColor);
}

void Bullet::update(float dt) {
    // Save centre position BEFORE moving so CollisionManager can sweep
    prevX = position.x;
    prevY = position.y;

    Entity::update(dt); // position += velocity * dt

    // Kill if off screen
    if (position.y < -20.f || position.y > 740.f ||
        position.x < -20.f || position.x > 920.f) {
        destroy();
    }
}

void Bullet::draw(sf::RenderWindow& window) {
    shape.setPosition(position);
    sf::RectangleShape glow;
    glow.setSize({ width + 4.f, height + 4.f });
    glow.setOrigin({(width + 4.f) / 2.f, (height + 4.f) / 2.f});
    glow.setPosition(position);
    sf::Color glowCol = bulletColor;
    glowCol.a = 80;
    glow.setFillColor(glowCol);
    window.draw(glow);
    window.draw(shape);
}

void Bullet::onCollision(GameObject* other) {
    if (!piercing) {
        destroy();
    }
    else if (alreadyPierced) {
        destroy();
    }
}