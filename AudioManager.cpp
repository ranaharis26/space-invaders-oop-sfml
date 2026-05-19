#include "AudioManager.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <cstdint>

static const int SAMPLE_RATE = 44100;

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

void AudioManager::generateBeep(sf::SoundBuffer& buf, float freq, float duration, float vol) {
    int samples = (int)(SAMPLE_RATE * duration);
    std::vector<std::int16_t> data(samples);
    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float env = 1.0f - t / duration;
        data[i] = (std::int16_t)(vol * env * 28000 * std::sin(2*3.14159f*freq*t));
    }
    buf.loadFromSamples(data.data(), samples, 1, SAMPLE_RATE, {sf::SoundChannel::Mono});
}

void AudioManager::generateNoise(sf::SoundBuffer& buf, float duration, float vol) {
    int samples = (int)(SAMPLE_RATE * duration);
    std::vector<std::int16_t> data(samples);
    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float env = 1.0f - t / duration;
        data[i] = (std::int16_t)(vol * env * 28000 * ((std::rand()%2001 - 1000) / 1000.f));
    }
    buf.loadFromSamples(data.data(), samples, 1, SAMPLE_RATE, {sf::SoundChannel::Mono});
}

void AudioManager::generateSweep(sf::SoundBuffer& buf, float f1, float f2, float duration, float vol) {
    int samples = (int)(SAMPLE_RATE * duration);
    std::vector<std::int16_t> data(samples);
    float phase = 0;
    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float env = 1.0f - t / duration;
        float freq = f1 + (f2 - f1) * (t / duration);
        phase += 2*3.14159f*freq / SAMPLE_RATE;
        data[i] = (std::int16_t)(vol * env * 28000 * std::sin(phase));
    }
    buf.loadFromSamples(data.data(), samples, 1, SAMPLE_RATE, {sf::SoundChannel::Mono});
}

AudioManager::AudioManager()
    : sfxShoot(shootBuf), sfxSpread(spreadBuf), sfxPierce(pierceBuf),
      sfxExplosion(explosionBuf), sfxPlayerHit(playerHitBuf), sfxPowerUp(powerUpBuf),
      sfxEMP(empBuf), sfxShieldBreak(shieldBreakBuf), music(menuMusicBuf),
      currentTrack(-1)
{
    generateBeep(shootBuf,  880.f, 0.08f, 0.5f);
    generateBeep(spreadBuf, 660.f, 0.10f, 0.5f);
    generateSweep(pierceBuf, 440.f, 1200.f, 0.12f, 0.6f);
    generateNoise(explosionBuf, 0.3f, 0.7f);
    generateSweep(playerHitBuf, 200.f, 80.f, 0.2f, 0.8f);
    generateSweep(powerUpBuf, 400.f, 800.f, 0.2f, 0.6f);
    generateSweep(empBuf, 1200.f, 80.f, 0.5f, 0.9f);
    generateSweep(shieldBreakBuf, 600.f, 100.f, 0.3f, 0.8f);

    // Re-bind sounds to their (now-filled) buffers
    sfxShoot.setBuffer(shootBuf);
    sfxSpread.setBuffer(spreadBuf);
    sfxPierce.setBuffer(pierceBuf);
    sfxExplosion.setBuffer(explosionBuf);
    sfxPlayerHit.setBuffer(playerHitBuf);
    sfxPowerUp.setBuffer(powerUpBuf);
    sfxEMP.setBuffer(empBuf);
    sfxShieldBreak.setBuffer(shieldBreakBuf);

    // Generate looping music buffers (simple tone sequences)
    // Menu music: calm arpeggio
    {
        int len = SAMPLE_RATE * 4;
        std::vector<std::int16_t> data(len, 0);
        float freqs[] = {261.f, 329.f, 392.f, 523.f, 392.f, 329.f};
        int nf = 6, noteLen = len / nf;
        for (int n = 0; n < nf; n++) {
            float f = freqs[n];
            for (int i = 0; i < noteLen; i++) {
                float t = (float)i / SAMPLE_RATE;
                float env = (i < noteLen/4) ? (float)i/(noteLen/4) : 1.0f - (float)(i-noteLen/4*3)/noteLen;
                env = std::max(0.f, std::min(1.f, env));
                data[n*noteLen+i] += (std::int16_t)(6000 * env * std::sin(2*3.14159f*f*t));
            }
        }
        menuMusicBuf.loadFromSamples(data.data(), len, 1, SAMPLE_RATE, {sf::SoundChannel::Mono});
    }
    // Game music: faster pulsing
    {
        int len = SAMPLE_RATE * 2;
        std::vector<std::int16_t> data(len, 0);
        float freqs[] = {220.f, 277.f, 330.f, 220.f};
        int nf=4, noteLen=len/nf;
        for (int n=0;n<nf;n++) {
            float f=freqs[n];
            for (int i=0;i<noteLen;i++) {
                float t=(float)i/SAMPLE_RATE;
                float env=1.0f-(float)i/noteLen;
                data[n*noteLen+i]+=(std::int16_t)(8000*env*std::sin(2*3.14159f*f*t));
            }
        }
        gameMusicBuf.loadFromSamples(data.data(), len, 1, SAMPLE_RATE, {sf::SoundChannel::Mono});
    }
    // Boss music: intense
    {
        int len = SAMPLE_RATE * 2;
        std::vector<std::int16_t> data(len, 0);
        float freqs[] = {110.f, 138.f, 110.f, 92.f};
        int nf=4, noteLen=len/nf;
        for (int n=0;n<nf;n++) {
            float f=freqs[n];
            for (int i=0;i<noteLen;i++) {
                float t=(float)i/SAMPLE_RATE;
                float env=0.7f+0.3f*std::sin(20*t);
                data[n*noteLen+i]+=(std::int16_t)(10000*env*std::sin(2*3.14159f*f*t));
                // Add harmonic
                data[n*noteLen+i]+=(std::int16_t)(4000*env*std::sin(2*3.14159f*f*2*t));
            }
        }
        bossMusicBuf.loadFromSamples(data.data(), len, 1, SAMPLE_RATE, {sf::SoundChannel::Mono});
    }

    music.setLooping(true);
    music.setVolume(40.f);
}

AudioManager::~AudioManager() {}

void AudioManager::playShoot(int wt) {
    if (wt==0)      { sfxShoot.play(); }
    else if(wt==1)  { sfxSpread.play(); }
    else            { sfxPierce.play(); }
}
void AudioManager::playExplosion()  { sfxExplosion.play(); }
void AudioManager::playPlayerHit()  { sfxPlayerHit.play(); }
void AudioManager::playPowerUp()    { sfxPowerUp.play(); }
void AudioManager::playEMP()        { sfxEMP.play(); }
void AudioManager::playShieldBreak(){ sfxShieldBreak.play(); }

void AudioManager::playMenuMusic() {
    if (currentTrack == 0) return;
    music.setBuffer(menuMusicBuf);
    music.play();
    currentTrack = 0;
}
void AudioManager::playGameMusic() {
    if (currentTrack == 1) return;
    music.setBuffer(gameMusicBuf);
    music.play();
    currentTrack = 1;
}
void AudioManager::playBossMusic() {
    if (currentTrack == 2) return;
    music.setBuffer(bossMusicBuf);
    music.play();
    currentTrack = 2;
}
void AudioManager::stopMusic() {
    music.stop();
    currentTrack = -1;
}
