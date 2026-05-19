#include "PowerUp.h"
#include <cmath>
#include <string>

PowerUp::PowerUp(float x, float y, PowerUpType t)
    : Entity(x, y, 24.f, 24.f), type(t), animTimer(0.f),
      labelPtr(nullptr), fontLoaded(false)
{
    velocity = {0.f, 40.f};

    outerRing.setRadius(12.f);
    outerRing.setOrigin({12.f, 12.f});
    outerRing.setFillColor(sf::Color::Transparent);
    outerRing.setOutlineThickness(2.f);

    innerCircle.setRadius(6.f);
    innerCircle.setOrigin({6.f, 6.f});

    sf::Color baseColor;
    std::string textLabel = "";

    switch(type) {
        case PowerUpType::SPREAD:
            baseColor = sf::Color(80, 255, 150);
            textLabel = "S";
            break;
        case PowerUpType::PIERCING:
            baseColor = sf::Color(255, 80, 80);
            textLabel = "P";
            break;
        case PowerUpType::SHIELD:
            baseColor = sf::Color(80, 200, 255);
            textLabel = "O";
            break;
        case PowerUpType::EMP:
            baseColor = sf::Color(255, 220, 60);
            textLabel = "N";
            break;
    }

    outerRing.setOutlineColor(baseColor);
    innerCircle.setFillColor(baseColor);

    // Try to load font
    if (font.openFromFile("C:/Windows/Fonts/arial.ttf") ||
        font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
        fontLoaded = true;
        labelPtr = new sf::Text(font, textLabel, 14);
        labelPtr->setFillColor(sf::Color::White);
        // Center text approximately
        sf::FloatRect bounds = labelPtr->getLocalBounds();
        labelPtr->setOrigin({bounds.position.x + bounds.size.x/2.0f, bounds.position.y + bounds.size.y/2.0f});
    }
}

void PowerUp::update(float dt) {
    Entity::update(dt);
    animTimer += dt;

    // Float downward
    if (position.y > 750.f) {
        destroy();
    }
}

void PowerUp::draw(sf::RenderWindow& window) {
    outerRing.setPosition(position);
    innerCircle.setPosition(position);

    // Pulsing effect
    float scale = 1.0f + 0.15f * std::sin(animTimer * 5.0f);
    outerRing.setScale({scale, scale});

    window.draw(outerRing);
    window.draw(innerCircle);

    if (fontLoaded && labelPtr) {
        labelPtr->setPosition(position);
        window.draw(*labelPtr);
    }
}
