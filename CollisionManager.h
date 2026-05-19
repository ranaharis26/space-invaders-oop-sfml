#pragma once
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Boss.h"
#include "TwinCannons.h"
#include "PowerUp.h"
#include "Asteroid.h"
#include "Explosion.h"
#include <SFML/Graphics.hpp>

// Utility: check AABB overlap
inline bool overlaps(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

// Utility: circle vs AABB
inline bool circleAABB(float cx, float cy, float r, const sf::FloatRect& rect) {
    float nearX = std::max(rect.position.x, std::min(cx, rect.position.x + rect.size.x));
    float nearY = std::max(rect.position.y,  std::min(cy, rect.position.y  + rect.size.y));
    float dx = cx - nearX, dy = cy - nearY;
    return (dx*dx + dy*dy) <= (r*r);
}

struct CollisionResult {
    int scoreGained;
    bool playerDied;
    bool empActivated; // not collision but we track here
};

class CollisionManager {
public:
    // Returns score gained from kills
    static int runAll(
        Player* player,
        Bullet** playerBullets, int& pbCount,
        Bullet** enemyBullets,  int& ebCount,
        Enemy**  enemies,       int& eCount,
        Boss*    boss,          // may be nullptr
        TwinCannons* twin,      // may be nullptr (subset of boss for turret hits)
        PowerUp** powerUps,     int& puCount,
        Asteroid** asteroids,   int& astCount,
        // outputs
        Explosion** explosions, int& expCount, int expCap,
        int& livesLost
    );
};
