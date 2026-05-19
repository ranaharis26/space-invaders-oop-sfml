#include "Enemy.h"

Enemy::Enemy(float x, float y, float w, float h, int hp)
    : Entity(x, y, w, h), hp(hp), maxHP(hp),
      fireTimer(0), fireInterval(2.0f),
      speedScale(1.f), fireRateScale(1.f),
      diedThisFrame(false)
{}

void Enemy::update(float dt) {
    Entity::update(dt);
    fireTimer -= dt;
}

void Enemy::onCollision(GameObject* other) {
    // base: do nothing, subclasses override
}

void Enemy::takeDamage(int dmg) {
    hp -= dmg;
    if (hp <= 0) {
        hp = 0;
        diedThisFrame = true;
        destroy();
    }
}
