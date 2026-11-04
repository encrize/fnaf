#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "Common.h"

namespace FNAF {

enum class SoundID {
    Ambient,
    Door,
    LightBuzz,
    CameraSwitch,
    CameraFlip,
    Footsteps,
    Jumpscare,
    WindowScare,
    AnimatronicMove,
    FoxKnock,
    PowerOut,
    PowerOutMusic,
    Kitchen,
    Chime6AM,
    Cheer6AM,
    PirateSong,
    Blip,
    FanHum,
    MenuMusic,
    PhoneCall1,
    PhoneCall2,
    PhoneCall3,
    PhoneCall4,
    PhoneCall5,
    COUNT
};

class Audio {
private:
    std::unordered_map<SoundID, Sound> m_sounds;
    std::unordered_map<SoundID, Music> m_music;
    float m_masterVolume = 1.0f;
    float m_sfxVolume = 1.0f;
    float m_musicVolume = 0.8f;
    bool m_initialized = false;
    bool m_fanPlaying = false;
    bool m_ambientPlaying = false;
    bool m_lightBuzzPlaying = false;

    Sound GenerateProceduralSound(SoundID id);
    Wave SynthesizeWave(SoundID id);

public:
    Audio();
    ~Audio();

    // Non-copyable
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    void Initialize();
    void Cleanup();
    void Update();

    void SetMasterVolume(float vol);
    void SetSFXVolume(float vol);
    void SetMusicVolume(float vol);

    void PlaySoundEffect(SoundID id, float volumeMultiplier = 1.0f, float pitch = 1.0f, float pan = 0.5f);

    // Convenience functions requested in specifications
    void PlayDoorSound(bool closing = true);
    void SetLightBuzz(bool active);
    void PlayFootsteps(float pan = 0.5f);
    void PlayJumpscare();
    void PlayWindowScare();
    void PlayAmbient(bool active = true);
    void PlayMenuMusic(bool active = true);
    void PlayFanHum(bool active = true);
    void PlayCameraSwitch();
    void PlayCameraFlip();
    void PlayPhoneCall(int night);
    void StopPhoneCall();
    void PlayAnimatronicMovement(float pan = 0.5f);
    void PlayFoxKnock();
    void PlayPowerOut();
    void PlayPowerOutMusic(bool active = true);
    void PlayKitchenAudio();
    void PlayChime6AM();
    void PlayCheer6AM();
    void PlayPirateSong();
    void PlayBlip();
    void StopAllSounds();
};

} // namespace FNAF
