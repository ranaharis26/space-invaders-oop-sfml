#pragma once
#include "GameObject.h"
#include <SFML/Graphics.hpp>

class Entity : public GameObject {
public:
    Entity(float x, float y, float w, float h);
    virtual ~Entity() {}

    sf::Vector2f getPosition() const { return position; }
    sf::Vector2f getVelocity() const { return velocity; }
    sf::FloatRect getHitbox() const;
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    void setPosition(float x, float y) { position = {x, y}; }
    void setVelocity(float vx, float vy) { velocity = {vx, vy}; }

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override {}
    virtual void onCollision(GameObject* other) override {}

protected:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float width, height;
};
