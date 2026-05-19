#pragma once
#include "Entity.h"
#include <SFML/Graphics.hpp>

enum class PowerUpType { SPREAD, PIERCING, SHIELD, EMP };

class PowerUp : public Entity {
public:
    PowerUp(float x, float y, PowerUpType type);
    virtual ~PowerUp() { delete labelPtr; }

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override {}

    PowerUpType getType() const { return type; }

private:
    PowerUpType type;
    float animTimer;
    sf::CircleShape outerRing;
    sf::CircleShape innerCircle;
    sf::Text* labelPtr;
    sf::Font font;
    bool fontLoaded;
};
