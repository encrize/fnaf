#include "Audio.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace FNAF {

Audio::Audio() = default;

Audio::~Audio() {
    Cleanup();
}

void Audio::Initialize() {
    if (m_initialized) return;

    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
    }
    m_initialized = IsAudioDeviceReady();

    if (!m_initialized) {
        std::cerr << "[Audio] Warning: Failed to initialize raylib audio device.\n";
        return;
    }

    // Try loading custom audio files from disk first, fallback to procedural synthesis
    struct SoundMapping {
        SoundID id;
        const char* filename;
    };

    const SoundMapping mappings[] = {
        { SoundID::Ambient, "assets/audio/ambient.wav" },
        { SoundID::Door, "assets/audio/door.wav" },
        { SoundID::LightBuzz, "assets/audio/light_buzz.wav" },
        { SoundID::CameraSwitch, "assets/audio/camera_switch.wav" },
        { SoundID::CameraFlip, "assets/audio/camera_flip.wav" },
        { SoundID::Footsteps, "assets/audio/footsteps.wav" },
        { SoundID::Jumpscare, "assets/audio/jumpscare.wav" },
        { SoundID::WindowScare, "assets/audio/windowscare.wav" },
        { SoundID::AnimatronicMove, "assets/audio/animatronic_move.wav" },
        { SoundID::FoxKnock, "assets/audio/fox_knock.wav" },
        { SoundID::PowerOut, "assets/audio/power_out.wav" },
        { SoundID::PowerOutMusic, "assets/audio/power_music.wav" },
        { SoundID::Kitchen, "assets/audio/kitchen.wav" },
        { SoundID::Chime6AM, "assets/audio/chime_6am.wav" },
        { SoundID::Cheer6AM, "assets/audio/cheer.wav" },
        { SoundID::PirateSong, "assets/audio/pirate_song.wav" },
        { SoundID::Blip, "assets/audio/blip.wav" },
        { SoundID::FanHum, "assets/audio/fan.wav" },
        { SoundID::MenuMusic, "assets/audio/menu_music.wav" },
        { SoundID::PhoneCall1, "assets/audio/call_night1.wav" },
        { SoundID::PhoneCall2, "assets/audio/call_night2.wav" },
        { SoundID::PhoneCall3, "assets/audio/call_night3.wav" },
        { SoundID::PhoneCall4, "assets/audio/call_night4.wav" },
        { SoundID::PhoneCall5, "assets/audio/call_night5.wav" }
    };

    for (const auto& item : mappings) {
        if (FileExists(item.filename)) {
            Sound snd = LoadSound(item.filename);
            if (snd.frameCount > 0) {
                m_sounds[item.id] = snd;
                continue;
            }
        }
        // Procedural synthesis fallback
        m_sounds[item.id] = GenerateProceduralSound(item.id);
    }
}

Wave Audio::SynthesizeWave(SoundID id) {
    const unsigned int sampleRate = 44100;
    unsigned int channels = 1;
    unsigned int sampleSize = 16;
    unsigned int frameCount = 0;

    float duration = 0.5f;

    switch (id) {
        case SoundID::Ambient: duration = 2.0f; break;
        case SoundID::FanHum: duration = 1.0f; break;
        case SoundID::Door: duration = 0.4f; break;
        case SoundID::LightBuzz: duration = 0.6f; break;
        case SoundID::CameraSwitch: duration = 0.08f; break;
        case SoundID::Footsteps: duration = 0.25f; break;
        case SoundID::Jumpscare: duration = 1.8f; break;
        case SoundID::AnimatronicMove: duration = 0.8f; break;
        case SoundID::FoxKnock: duration = 0.7f; break;
        case SoundID::PowerOut: duration = 1.5f; break;
        case SoundID::PowerOutMusic: duration = 4.0f; break;
        case SoundID::Kitchen: duration = 1.0f; break;
        case SoundID::Chime6AM: duration = 3.0f; break;
        case SoundID::Cheer6AM: duration = 2.5f; break;
        case SoundID::PirateSong: duration = 3.5f; break;
        case SoundID::Blip: duration = 0.05f; break;
        default: duration = 0.5f; break;
    }

    frameCount = static_cast<unsigned int>(duration * sampleRate);
    int16_t* buffer = static_cast<int16_t*>(std::malloc(frameCount * sizeof(int16_t)));
    if (!buffer) {
        return Wave{ 0, sampleRate, sampleSize, channels, nullptr };
    }

    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        float progress = static_cast<float>(i) / static_cast<float>(frameCount);
        float sample = 0.0f;

        switch (id) {
            case SoundID::Ambient: {
                // Low drone 55Hz + 110Hz harmonic + soft pink noise
                float osc1 = std::sin(2.0f * PI * 55.0f * t);
                float osc2 = std::sin(2.0f * PI * 110.0f * t) * 0.4f;
                float noise = ((std::rand() % 2000) / 1000.0f - 1.0f) * 0.05f;
                sample = (osc1 + osc2 + noise) * 0.4f;
                break;
            }
            case SoundID::FanHum: {
                // 120Hz drone + slight motor fluctuation
                float osc = std::sin(2.0f * PI * (120.0f + std::sin(2.0f * PI * 5.0f * t) * 3.0f) * t);
                float air = ((std::rand() % 2000) / 1000.0f - 1.0f) * 0.15f;
                sample = (osc * 0.25f + air) * 0.3f;
                break;
            }
            case SoundID::Door: {
                // Hydraulic motor slide followed by deep impact thud
                float envelope = std::exp(-progress * 5.0f);
                float freq = 220.0f * (1.0f - progress * 0.7f);
                float tone = std::sin(2.0f * PI * freq * t);
                float thud = (progress > 0.6f) ? std::sin(2.0f * PI * 50.0f * (t - 0.24f)) * std::exp(-(progress - 0.6f) * 12.0f) : 0.0f;
                sample = (tone * 0.4f + thud * 0.8f) * envelope;
                break;
            }
            case SoundID::LightBuzz: {
                // 120Hz electrical buzz with harsh odd harmonics
                float osc1 = std::sin(2.0f * PI * 120.0f * t);
                float osc3 = std::sin(2.0f * PI * 360.0f * t) * 0.5f;
                float osc5 = std::sin(2.0f * PI * 600.0f * t) * 0.25f;
                float spark = ((std::rand() % 100) > 95) ? 0.3f : 0.0f;
                sample = (osc1 + osc3 + osc5 + spark) * 0.25f;
                break;
            }
            case SoundID::CameraSwitch: {
                // Short click and burst of static
                float noise = ((std::rand() % 2000) / 1000.0f - 1.0f);
                float click = std::sin(2.0f * PI * 1200.0f * t) * std::exp(-progress * 25.0f);
                sample = (noise * 0.3f + click * 0.5f) * std::exp(-progress * 15.0f);
                break;
            }
            case SoundID::Footsteps: {
                // Muffled heavy thud
                float env = std::sin(progress * PI);
                float freq = 70.0f * (1.0f - progress * 0.5f);
                sample = std::sin(2.0f * PI * freq * t) * env * 0.6f;
                break;
            }
            case SoundID::Jumpscare: {
                // Terrifying discordant screech with aggressive frequency modulation and distortion
                float fMod1 = std::sin(2.0f * PI * 28.0f * t) * 180.0f;
                float fMod2 = std::sin(2.0f * PI * 42.0f * t) * 250.0f;
                float carrier1 = std::sin(2.0f * PI * (580.0f + fMod1) * t);
                float carrier2 = std::sin(2.0f * PI * (820.0f + fMod2) * t);
                float harshNoise = ((std::rand() % 2000) / 1000.0f - 1.0f) * 0.5f;
                float raw = (carrier1 * 0.6f + carrier2 * 0.6f + harshNoise);
                // Hard clipping distortion
                if (raw > 0.8f) raw = 0.8f;
                if (raw < -0.8f) raw = -0.8f;
                sample = raw * 0.9f;
                break;
            }
            case SoundID::AnimatronicMove: {
                // Deep mechanical groan
                float freq = 65.0f + std::sin(2.0f * PI * 4.0f * t) * 15.0f;
                float env = std::sin(progress * PI);
                sample = std::sin(2.0f * PI * freq * t) * env * 0.7f;
                break;
            }
            case SoundID::FoxKnock: {
                // 3 door hits: at 0.0s, 0.2s, 0.4s
                float hit = 0.0f;
                for (int h = 0; h < 3; ++h) {
                    float ht = t - h * 0.22f;
                    if (ht >= 0.0f && ht < 0.18f) {
                        float hProg = ht / 0.18f;
                        float env = std::exp(-hProg * 18.0f);
                        hit += std::sin(2.0f * PI * 85.0f * ht) * env;
                    }
                }
                sample = hit * 0.8f;
                break;
            }
            case SoundID::PowerOut: {
                // Pitch falling downwards to 0
                float fall = std::max(0.0f, 1.0f - progress);
                float freq = 300.0f * fall * fall;
                sample = std::sin(2.0f * PI * freq * t) * fall * 0.6f;
                break;
            }
            case SoundID::PowerOutMusic: {
                // Eerie music box Toreador chime
                // Notes: G4, E4, F4, G4, A4, G4, F4, E4, D4
                float noteDuration = 0.4f;
                int noteIdx = static_cast<int>(t / noteDuration);
                float noteT = std::fmod(t, noteDuration);
                float freqs[] = { 392.0f, 329.63f, 349.23f, 392.0f, 440.0f, 392.0f, 349.23f, 329.63f, 293.66f, 329.63f };
                float noteFreq = (noteIdx < 10) ? freqs[noteIdx] : 392.0f;
                float env = std::exp(-noteT * 6.0f);
                sample = std::sin(2.0f * PI * noteFreq * noteT) * env * 0.5f;
                break;
            }
            case SoundID::Kitchen: {
                // Metallic clanging of pans
                float noise = ((std::rand() % 2000) / 1000.0f - 1.0f) * 0.4f;
                float metallic = std::sin(2.0f * PI * 1800.0f * t) * 0.5f + std::sin(2.0f * PI * 2400.0f * t) * 0.3f;
                float env = std::exp(-std::fmod(progress * 4.0f, 1.0f) * 8.0f);
                sample = (metallic + noise) * env * 0.4f;
                break;
            }
            case SoundID::Chime6AM: {
                // Cheerful 4-bell chime
                float chimeDur = 0.6f;
                int bIdx = static_cast<int>(t / chimeDur);
                float bT = std::fmod(t, chimeDur);
                float bellFreqs[] = { 659.25f, 587.33f, 523.25f, 392.0f }; // E5, D5, C5, G4
                float freq = (bIdx < 4) ? bellFreqs[bIdx] : 523.25f;
                float bell = std::sin(2.0f * PI * freq * bT) * std::exp(-bT * 4.0f);
                sample = bell * 0.6f;
                break;
            }
            case SoundID::Blip: {
                float env = 1.0f - progress;
                sample = std::sin(2.0f * PI * 880.0f * t) * env * 0.3f;
                break;
            }
            default: break;
        }

        // Clamp to 16-bit signed integer range
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        buffer[i] = static_cast<int16_t>(sample * 32767.0f);
    }

    Wave wave;
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = sampleSize;
    wave.channels = channels;
    wave.data = buffer;

    return wave;
}

Sound Audio::GenerateProceduralSound(SoundID id) {
    Wave wave = SynthesizeWave(id);
    if (!wave.data) return Sound{};
    Sound snd = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return snd;
}

void Audio::SetMasterVolume(float vol) {
    m_masterVolume = vol;
    ::SetMasterVolume(vol);
}

void Audio::SetSFXVolume(float vol) {
    m_sfxVolume = vol;
}

void Audio::SetMusicVolume(float vol) {
    m_musicVolume = vol;
}

void Audio::PlaySoundEffect(SoundID id, float volumeMultiplier, float pitch, float pan) {
    if (!m_initialized) return;

    auto it = m_sounds.find(id);
    if (it != m_sounds.end() && it->second.frameCount > 0) {
        SetSoundVolume(it->second, m_sfxVolume * volumeMultiplier);
        SetSoundPitch(it->second, pitch);
        SetSoundPan(it->second, pan);
        PlaySound(it->second);
    }
}

void Audio::PlayDoorSound(bool /*closing*/) {
    PlaySoundEffect(SoundID::Door, 0.7f, 1.0f);
}

void Audio::SetLightBuzz(bool active) {
    if (!m_initialized) return;
    auto it = m_sounds.find(SoundID::LightBuzz);
    if (it == m_sounds.end() || it->second.frameCount == 0) return;

    if (active && !m_lightBuzzPlaying) {
        SetSoundVolume(it->second, m_sfxVolume * 0.4f);
        PlaySound(it->second);
        m_lightBuzzPlaying = true;
    } else if (!active && m_lightBuzzPlaying) {
        StopSound(it->second);
        m_lightBuzzPlaying = false;
    }
}

void Audio::PlayFootsteps(float pan) {
    PlaySoundEffect(SoundID::Footsteps, 0.6f, 1.0f + ((std::rand() % 20) - 10) * 0.01f, pan);
}

void Audio::PlayJumpscare() {
    PlaySoundEffect(SoundID::Jumpscare, 1.0f, 1.0f);
}

void Audio::PlayAmbient(bool active) {
    if (!m_initialized) return;
    auto it = m_sounds.find(SoundID::Ambient);
    if (it == m_sounds.end() || it->second.frameCount == 0) return;

    if (active && !m_ambientPlaying) {
        SetSoundVolume(it->second, m_musicVolume * 0.35f);
        PlaySound(it->second);
        m_ambientPlaying = true;
    } else if (!active && m_ambientPlaying) {
        StopSound(it->second);
        m_ambientPlaying = false;
    }
}

void Audio::PlayFanHum(bool active) {
    if (!m_initialized) return;
    auto it = m_sounds.find(SoundID::FanHum);
    if (it == m_sounds.end() || it->second.frameCount == 0) return;

    if (active && !m_fanPlaying) {
        SetSoundVolume(it->second, m_sfxVolume * 0.25f);
        PlaySound(it->second);
        m_fanPlaying = true;
    } else if (!active && m_fanPlaying) {
        StopSound(it->second);
        m_fanPlaying = false;
    }
}

void Audio::PlayCameraSwitch() {
    PlaySoundEffect(SoundID::CameraSwitch, 0.5f, 1.0f);
}

void Audio::PlayCameraFlip() {
    PlaySoundEffect(SoundID::CameraFlip, 0.7f, 1.0f);
}

void Audio::PlayWindowScare() {
    PlaySoundEffect(SoundID::WindowScare, 0.8f, 1.0f);
}

void Audio::PlayMenuMusic(bool active) {
    if (!m_initialized) return;
    auto it = m_sounds.find(SoundID::MenuMusic);
    if (it == m_sounds.end() || it->second.frameCount == 0) return;

    if (active && !IsSoundPlaying(it->second)) {
        SetSoundVolume(it->second, m_musicVolume * 0.45f);
        PlaySound(it->second);
    } else if (!active && IsSoundPlaying(it->second)) {
        StopSound(it->second);
    }
}

void Audio::PlayPhoneCall(int night) {
    StopPhoneCall();
    SoundID callID = SoundID::PhoneCall1;
    if (night == 2) callID = SoundID::PhoneCall2;
    else if (night == 3) callID = SoundID::PhoneCall3;
    else if (night == 4) callID = SoundID::PhoneCall4;
    else if (night >= 5) callID = SoundID::PhoneCall5;

    PlaySoundEffect(callID, 0.85f, 1.0f);
}

void Audio::StopPhoneCall() {
    if (!m_initialized) return;
    SoundID calls[] = { SoundID::PhoneCall1, SoundID::PhoneCall2, SoundID::PhoneCall3, SoundID::PhoneCall4, SoundID::PhoneCall5 };
    for (SoundID cid : calls) {
        auto it = m_sounds.find(cid);
        if (it != m_sounds.end() && it->second.frameCount > 0) {
            StopSound(it->second);
        }
    }
}

void Audio::PlayAnimatronicMovement(float pan) {
    PlaySoundEffect(SoundID::AnimatronicMove, 0.7f, 0.9f + ((std::rand() % 30) * 0.01f), pan);
}

void Audio::PlayFoxKnock() {
    PlaySoundEffect(SoundID::FoxKnock, 0.9f, 1.0f, 0.15f); // Left-panned knock
}

void Audio::PlayPowerOut() {
    PlaySoundEffect(SoundID::PowerOut, 0.8f, 1.0f);
}

void Audio::PlayPowerOutMusic(bool active) {
    if (!m_initialized) return;
    auto it = m_sounds.find(SoundID::PowerOutMusic);
    if (it == m_sounds.end() || it->second.frameCount == 0) return;

    if (active) {
        SetSoundVolume(it->second, m_musicVolume * 0.6f);
        PlaySound(it->second);
    } else {
        StopSound(it->second);
    }
}

void Audio::PlayKitchenAudio() {
    PlaySoundEffect(SoundID::Kitchen, 0.6f, 1.0f, 0.85f); // Right-panned kitchen
}

void Audio::PlayChime6AM() {
    PlaySoundEffect(SoundID::Chime6AM, 0.8f, 1.0f);
}

void Audio::PlayCheer6AM() {
    PlaySoundEffect(SoundID::Cheer6AM, 0.85f, 1.0f);
}

void Audio::PlayPirateSong() {
    PlaySoundEffect(SoundID::PirateSong, 0.7f, 1.0f, 0.25f);
}

void Audio::PlayBlip() {
    PlaySoundEffect(SoundID::Blip, 0.4f, 1.0f);
}

void Audio::StopAllSounds() {
    if (!m_initialized) return;
    for (auto& pair : m_sounds) {
        if (pair.second.frameCount > 0) {
            StopSound(pair.second);
        }
    }
    m_fanPlaying = false;
    m_ambientPlaying = false;
    m_lightBuzzPlaying = false;
}

void Audio::Update() {
    if (!m_initialized) return;
    // Loop ambient and fan hum if active
    if (m_ambientPlaying) {
        auto it = m_sounds.find(SoundID::Ambient);
        if (it != m_sounds.end() && !IsSoundPlaying(it->second)) {
            PlaySound(it->second);
        }
    }
    if (m_fanPlaying) {
        auto it = m_sounds.find(SoundID::FanHum);
        if (it != m_sounds.end() && !IsSoundPlaying(it->second)) {
            PlaySound(it->second);
        }
    }
    if (m_lightBuzzPlaying) {
        auto it = m_sounds.find(SoundID::LightBuzz);
        if (it != m_sounds.end() && !IsSoundPlaying(it->second)) {
            PlaySound(it->second);
        }
    }
}

void Audio::Cleanup() {
    if (!m_initialized) return;

    StopAllSounds();

    for (auto& pair : m_sounds) {
        if (pair.second.frameCount > 0) {
            UnloadSound(pair.second);
        }
    }
    m_sounds.clear();

    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
    m_initialized = false;
}

} // namespace FNAF
