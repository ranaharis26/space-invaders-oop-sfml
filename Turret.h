#pragma once
#include "Bullet.h"
#include <SFML/Graphics.hpp>
#include <cmath>

class Turret {
public:
    Turret(float x, float y)
        : pos(x, y), hp(30), maxHP(30), destroyed(false),
          fireTimer(0), fireInterval(1.5f), angle(0)
    {
        base.setSize({36.f, 36.f});
        base.setOrigin({18.f, 18.f});
        base.setFillColor(sf::Color(80, 80, 120));
        base.setOutlineColor(sf::Color(140, 140, 200));
        base.setOutlineThickness(2.f);

        barrel.setSize({8.f, 24.f});
        barrel.setOrigin({4.f, 0.f});
        barrel.setFillColor(sf::Color(200, 60, 60));
    }

    void setPosition(float x, float y) { pos = {x, y}; }

    void update(float dt) {
        if (destroyed) return;
        fireTimer -= dt;
        angle += dt * 30.f;
    }

    Bullet* shoot() {
        if (destroyed || fireTimer > 0) return nullptr;
        fireTimer = fireInterval;
        return new Bullet(pos.x, pos.y + 18.f, 0.f, 280.f, 1, false, Bullet::Owner::ENEMY);
    }

    void draw(sf::RenderWindow& window) {
        if (destroyed) return;
        base.setPosition(pos);
        barrel.setPosition({pos.x, pos.y + 18.f});
        window.draw(base);
        window.draw(barrel);
    }

    void takeDamage(int dmg) {
        hp -= dmg;
        if (hp <= 0) { hp = 0; destroyed = true; }
    }

    bool isDestroyed() const { return destroyed; }
    int  getHP()       const { return hp; }
    int  getMaxHP()    const { return maxHP; }

    sf::FloatRect getHitbox() const {
        return sf::FloatRect({pos.x - 18.f, pos.y - 18.f}, {36.f, 36.f});
    }

private:
    sf::Vector2f pos;
    int hp, maxHP;
    bool destroyed;
    float fireTimer, fireInterval, angle;
    sf::RectangleShape base;
    sf::RectangleShape barrel;
};
