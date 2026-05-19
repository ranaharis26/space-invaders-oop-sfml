#pragma once
#include <SFML/Graphics.hpp>

class GameObject {
public:
    virtual ~GameObject() {}
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void onCollision(GameObject* other) = 0;
    virtual bool isAlive() const { return alive; }
    virtual void destroy() { alive = false; }

protected:
    bool alive = true;
};
