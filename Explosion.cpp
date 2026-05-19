#include "Explosion.h"
#include <cstdlib>
#include <cmath>
#include <cstdint>

Explosion::Explosion(float x, float y, sf::Color color, int count, float radius) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.pos = {x, y};
        float angle = (float)(std::rand() % 360) * 3.14159f / 180.f;
        float speed = (float)(std::rand() % (int)radius + 20);
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.maxLife = p.life = 0.4f + (std::rand() % 40) / 100.f;
        p.color = color;
        p.size  = 2.f + (std::rand() % 5);
        particles.push_back(p);
    }
}

void Explosion::update(float dt) {
    for (auto& p : particles) {
        p.pos  += p.vel * dt;
        p.vel  *= (1.f - 3.f * dt); // drag
        p.life -= dt;
    }
    // Remove dead particles
    for (int i = (int)particles.size()-1; i >= 0; i--) {
        if (particles[i].life <= 0) {
            particles.erase(particles.begin() + i);
        }
    }
}

void Explosion::draw(sf::RenderWindow& window) {
    sf::CircleShape dot;
    for (auto& p : particles) {
        float ratio = p.life / p.maxLife;
        std::uint8_t alpha = (std::uint8_t)(255 * ratio);
        dot.setRadius(p.size * ratio);
        dot.setOrigin({p.size * ratio, p.size * ratio});
        dot.setPosition(p.pos);
        dot.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, alpha));
        window.draw(dot);
    }
}
