#include "Entity.h"

Entity::Entity(float x, float y, float w, float h)
    : GameObject(), position(x, y), velocity(0, 0), width(w), height(h) {}

void Entity::update(float dt) {
    position += velocity * dt;
}

sf::FloatRect Entity::getHitbox() const {
    return sf::FloatRect({position.x - width/2.f, position.y - height/2.f}, {width, height});
}
