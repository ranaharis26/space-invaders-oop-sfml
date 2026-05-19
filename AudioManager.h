#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <cstdint>

class AudioManager {
public:
    static AudioManager& getInstance();

    void playShoot(int weaponType);   // 0=standard,1=spread,2=pierce
    void playExplosion();
    void playPlayerHit();
    void playPowerUp();
    void playEMP();
    void playShieldBreak();
    void playBossMusic();
    void playMenuMusic();
    void playGameMusic();
    void stopMusic();

private:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void generateBeep(sf::SoundBuffer& buf, float freq, float duration, float vol=1.0f);
    void generateNoise(sf::SoundBuffer& buf, float duration, float vol=1.0f);
    void generateSweep(sf::SoundBuffer& buf, float f1, float f2, float duration, float vol=1.0f);

    // Buffers (procedurally generated)
    sf::SoundBuffer shootBuf;
    sf::SoundBuffer spreadBuf;
    sf::SoundBuffer pierceBuf;
    sf::SoundBuffer explosionBuf;
    sf::SoundBuffer playerHitBuf;
    sf::SoundBuffer powerUpBuf;
    sf::SoundBuffer empBuf;
    sf::SoundBuffer shieldBreakBuf;

    sf::Sound sfxShoot;
    sf::Sound sfxSpread;
    sf::Sound sfxPierce;
    sf::Sound sfxExplosion;
    sf::Sound sfxPlayerHit;
    sf::Sound sfxPowerUp;
    sf::Sound sfxEMP;
    sf::Sound sfxShieldBreak;

    // Music (procedurally generated loops)
    sf::SoundBuffer menuMusicBuf;
    sf::SoundBuffer gameMusicBuf;
    sf::SoundBuffer bossMusicBuf;
    sf::Sound music;
    int currentTrack; // 0=menu,1=game,2=boss
};
