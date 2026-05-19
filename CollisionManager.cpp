#include "CollisionManager.h"
#include "TwinCannons.h"
#include "AudioManager.h"
#include <cstdlib>
#include <cmath>
#include <cstdio>

static bool sweptBulletHitsRect(const sf::FloatRect& current,
    float prevCX, float prevCY,
    const sf::FloatRect& target)
{
    if (current.findIntersection(target).has_value()) return true;

    float curCX = current.position.x + current.size.x / 2.f;
    float curCY = current.position.y + current.size.y / 2.f;

    if (prevCX == curCX && prevCY == curCY) return false;

    float minX = std::min(prevCX, curCX) - current.size.x / 2.f;
    float minY = std::min(prevCY, curCY) - current.size.y / 2.f;
    float maxX = std::max(prevCX, curCX) + current.size.x / 2.f;
    float maxY = std::max(prevCY, curCY) + current.size.y / 2.f;

    sf::FloatRect swept({minX, minY}, {maxX - minX, maxY - minY});
    return swept.findIntersection(target).has_value();
}

int CollisionManager::runAll(
    Player* player,
    Bullet** playerBullets, int& pbCount,
    Bullet** enemyBullets, int& ebCount,
    Enemy** enemies, int& eCount,
    Boss* boss,
    TwinCannons* twin,
    PowerUp** powerUps, int& puCount,
    Asteroid** asteroids, int& astCount,
    Explosion** explosions, int& expCount, int expCap,
    int& livesLost
) {
    int scoreGained = 0;
    livesLost = 0;

    auto addExp = [&](float x, float y, sf::Color c, int cnt = 20, float r = 70.f) {
        if (expCount < expCap)
            explosions[expCount++] = new Explosion(x, y, c, cnt, r);
        };

    for (int b = 0; b < pbCount; b++) {
        Bullet* bul = playerBullets[b];
        if (!bul || !bul->isAlive()) continue;

        sf::FloatRect br = bul->getHitbox();
        float prevX = bul->getPrevX();
        float prevY = bul->getPrevY();

        for (int e = 0; e < eCount; e++) {
            Enemy* en = enemies[e];
            if (!en || !en->isAlive()) continue;
            if (sweptBulletHitsRect(br, prevX, prevY, en->getHitbox())) {
                en->takeDamage(bul->getDamage());
                AudioManager::getInstance().playExplosion();
                if (!en->isAlive()) {
                    addExp(en->getPosition().x, en->getPosition().y,
                        sf::Color(255, 140, 60), 18, 60);
                    scoreGained += 100;
                }
                if (bul->isPiercing() && !bul->hasAlreadyPierced())
                    bul->setPierced();
                else { bul->destroy(); break; }
            }
        }

        if (bul->isAlive() && boss && boss->isAlive()) {
            TwinCannons* tc = dynamic_cast<TwinCannons*>(boss);
            sf::FloatRect targetHB = tc ? tc->getActiveHitbox() : boss->getHitbox();

            if (sweptBulletHitsRect(br, prevX, prevY, targetHB)) {
                AudioManager::getInstance().playExplosion();
                bul->destroy();

                if (tc) {
                    tc->hitByBullet(bul->getDamage());
                    sf::Vector2f hitPos(targetHB.position.x + targetHB.size.x / 2.f,
                        targetHB.position.y + targetHB.size.y / 2.f);
                    addExp(hitPos.x, hitPos.y, sf::Color(255, 160, 60), 8, 25.f);
                }
                else {
                    boss->takeDamage(bul->getDamage());
                }

                if (!boss->isAlive()) {
                    addExp(boss->getPosition().x, boss->getPosition().y,
                        sf::Color(255, 80, 200), 50, 140);
                    scoreGained += 2000;
                }
            }
        }

        if (bul->isAlive()) {
            for (int a = 0; a < astCount; a++) {
                if (!asteroids[a]) continue;
                sf::Vector2f ap = asteroids[a]->getPosition();
                float ar = asteroids[a]->getRadius();
                float cx = br.position.x + br.size.x / 2.f;
                float cy = br.position.y + br.size.y / 2.f;
                float dx = cx - ap.x, dy = cy - ap.y;
                if (dx * dx + dy * dy < ar * ar) {
                    addExp(cx, cy, sf::Color(180, 140, 80), 6, 20.f);
                    bul->destroy();
                    break;
                }
            }
        }
    }

    sf::FloatRect pr = player->getHitbox();
    for (int b = 0; b < ebCount; b++) {
        Bullet* bul = enemyBullets[b];
        if (!bul || !bul->isAlive()) continue;
        if (overlaps(bul->getHitbox(), pr)) {
            if (!player->isInvincible()) {
                player->takeDamage(1);
                bul->destroy();
                if (player->wasHitThisFrame())
                    addExp(player->getPosition().x, player->getPosition().y,
                        sf::Color(255, 60, 60), 10, 30);
            }
        }
    }

    for (int e = 0; e < eCount; e++) {
        Enemy* en = enemies[e];
        if (!en || !en->isAlive()) continue;
        if (overlaps(en->getHitbox(), pr)) {
            if (!player->isInvincible()) {
                player->takeDamage(1);
                en->takeDamage(en->getHP());
                addExp(en->getPosition().x, en->getPosition().y,
                    sf::Color(255, 140, 60), 14, 50);
                if (!en->isAlive()) scoreGained += 100;
            }
        }
    }

    for (int b = 0; b < ebCount; b++) {
        Bullet* bul = enemyBullets[b];
        if (!bul || !bul->isAlive()) continue;
        for (int a = 0; a < astCount; a++) {
            if (!asteroids[a]) continue;
            sf::Vector2f ap = asteroids[a]->getPosition();
            float ar = asteroids[a]->getRadius();
            sf::FloatRect br2 = bul->getHitbox();
            float cx = br2.position.x + br2.size.x / 2.f;
            float cy = br2.position.y + br2.size.y / 2.f;
            float dx = cx - ap.x, dy = cy - ap.y;
            if (dx * dx + dy * dy < ar * ar) { bul->destroy(); break; }
        }
    }

    for (int a = 0; a < astCount; a++) {
        if (!asteroids[a]) continue;
        sf::Vector2f ap = asteroids[a]->getPosition();
        float ar = asteroids[a]->getRadius();
        sf::Vector2f pp = player->getPosition();
        float dx = pp.x - ap.x, dy = pp.y - ap.y;
        if (!player->isInvincible() &&
            dx * dx + dy * dy < (ar + 16.f) * (ar + 16.f)) {
            player->takeDamage(1);
            addExp(pp.x, pp.y, sf::Color(200, 140, 80), 12, 40);
        }
    }

    for (int i = 0; i < puCount; i++) {
        PowerUp* pu = powerUps[i];
        if (!pu || !pu->isAlive()) continue;
        if (overlaps(pu->getHitbox(), pr)) {
            switch (pu->getType()) {
            case PowerUpType::SPREAD:   player->collectWeapon(WeaponType::SPREAD);   break;
            case PowerUpType::PIERCING: player->collectWeapon(WeaponType::PIERCING); break;
            case PowerUpType::SHIELD:   player->collectShield(); break;
            case PowerUpType::EMP:      player->collectEMP();    break;
            }
            pu->destroy();
        }
    }

    return scoreGained;
}
