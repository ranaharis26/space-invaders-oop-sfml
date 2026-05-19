#include "Starfield.h"
#include <cstdlib>
#include <cstdint>

Starfield::Starfield(float w, float h, int count)
    : screenW(w), screenH(h), starCount(count)
{
    stars = new Star[count];
    for (int i = 0; i < count; i++) {
        float layer  = (float)(std::rand() % 3);  // 0=far,1=mid,2=near
        float radius = 0.5f + layer * 0.7f;
        float speed  = 20.f + layer * 40.f;
        stars[i].speed = speed;
        stars[i].shape.setRadius(radius);
        stars[i].shape.setOrigin({radius, radius});
        std::uint8_t brightness = (std::uint8_t)(100 + (int)(layer * 60) + std::rand()%60);
        stars[i].shape.setFillColor(sf::Color(brightness, brightness, (std::uint8_t)(brightness + 20)));
        stars[i].shape.setPosition({
            (float)(std::rand() % (int)w),
            (float)(std::rand() % (int)h)
        });
    }
}

void Starfield::update(float dt) {
    for (int i = 0; i < starCount; i++) {
        sf::Vector2f pos = stars[i].shape.getPosition();
        pos.y += stars[i].speed * dt;
        if (pos.y > screenH + 2) {
            pos.y = -2.f;
            pos.x = (float)(std::rand() % (int)screenW);
        }
        stars[i].shape.setPosition(pos);
    }
}

void Starfield::draw(sf::RenderWindow& window) {
    for (int i = 0; i < starCount; i++)
        window.draw(stars[i].shape);
}
