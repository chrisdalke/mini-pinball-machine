#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "soundManager.h"


SoundManager *initSound(){
    SoundManager *sound = malloc(sizeof(SoundManager));
    InitAudioDevice();
    sound->menuMusic = LoadMusicStream("Resources/Audio/1.mp3");
    sound->gameMusic = LoadMusicStream("Resources/Audio/5.mp3");
    SetMusicLoopCount(sound->menuMusic,1000);
    SetMusicLoopCount(sound->gameMusic,1000);
    sound->redPowerup = malloc(sizeof(Sound) * 4);
    sound->bluePowerup = malloc(sizeof(Sound) * 4);
    sound->slowdown = malloc(sizeof(Sound) * 4);
    sound->speedup = malloc(sizeof(Sound) * 4);
    sound->upperBouncer = malloc(sizeof(Sound) * 4);
    sound->click = malloc(sizeof(Sound) * 4);
    sound->bounce1 = malloc(sizeof(Sound) * 4);
    sound->bounce2 = malloc(sizeof(Sound) * 4);
    sound->flipper = malloc(sizeof(Sound) * 4);
    sound->waterSplash = malloc(sizeof(Sound) * 4);
    sound->launch = LoadSound("Resources/Audio/Click_Heavy_00.wav");
    sound->water = LoadSound("Resources/Audio/water.wav");
    for (int i = 0; i < 4; i++){
        sound->redPowerup[i] = LoadSound("Resources/Audio/redPowerup.wav");
        sound->bluePowerup[i] = LoadSound("Resources/Audio/redPowerup.wav");
        sound->slowdown[i] = LoadSound("Resources/Audio/slowdown.wav");
        sound->speedup[i] = LoadSound("Resources/Audio/speedup.ogg");
        sound->upperBouncer[i] = LoadSound("Resources/Audio/upperBouncer.wav");
        sound->click[i] = LoadSound("Resources/Audio/Typewriter_02.wav");
        sound->bounce1[i] = LoadSound("Resources/Audio/Bounce3.wav");
        sound->bounce2[i] = LoadSound("Resources/Audio/redPowerup3.wav");
        sound->flipper[i] = LoadSound("Resources/Audio/Slide_Sharp_02.wav");
        sound->waterSplash[i] = LoadSound("Resources/Audio/water2.wav");
    }
    return sound;
}
void updateSound(SoundManager *sound, GameStruct *game){
    // Play and transition music streams based on game mode.
    if (game->gameState == 0){
        if (!IsMusicPlaying(sound->menuMusic)){
            PlayMusicStream(sound->menuMusic);
            StopMusicStream(sound->gameMusic);
        }
        UpdateMusicStream(sound->menuMusic);
    } else if (game->gameState == 1 || game->gameState == 2){
        if (!IsMusicPlaying(sound->gameMusic)){
            PlayMusicStream(sound->gameMusic);
            StopMusicStream(sound->menuMusic);
        }
        UpdateMusicStream(sound->gameMusic);
        if (game->gameState == 1){
            sound->gameMusicVolume = 1.0f;
        } else {
            sound->gameMusicVolume -= 0.01f;
            if (sound->gameMusicVolume < 0.3f){
                sound->gameMusicVolume = 0.3f;
            }
        }
        SetMusicVolume(sound->gameMusic,sound->gameMusicVolume);
        if (game->slowMotionFactor < 1.0f){
            SetMusicPitch(sound->gameMusic,0.7f);
        } else {
            SetMusicPitch(sound->gameMusic,1.0f);
        }
    }
}

void playBounce(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->bounce1[i])){
            PlaySound(sound->bounce1[i]);
            return;
        }
    }
}
void playBounce2(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->bounce2[i])){
            PlaySound(sound->bounce2[i]);
            return;
        }
    }
}
void playClick(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->click[i])){
            PlaySound(sound->click[i]);
            return;
        }
    }
}

void playSlowdownSound(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->slowdown[i])){
            PlaySound(sound->slowdown[i]);
            return;
        }
    }
}
void playSpeedupSound(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->speedup[i])){
            PlaySound(sound->speedup[i]);
            return;
        }
    }
}
void playRedPowerupSound(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->redPowerup[i])){
            PlaySound(sound->redPowerup[i]);
            return;
        }
    }
}
void playBluePowerupSound(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->bluePowerup[i])){
            PlaySound(sound->bluePowerup[i]);
            return;
        }
    }
}
void playUpperBouncerSound(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->upperBouncer[i])){
            PlaySound(sound->upperBouncer[i]);
            return;
        }
    }
}
void playLaunch(SoundManager *sound){
    if (!IsSoundPlaying(sound->launch)){
        PlaySound(sound->launch);
        return;
    }
}
void playFlipper(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->flipper[i])){
            PlaySound(sound->flipper[i]);
            return;
        }
    }
}
void playWater(SoundManager *sound){
    if (!IsSoundPlaying(sound->water)){
        PlaySound(sound->water);
        return;
    }
}
void playWaterSplash(SoundManager *sound){
    for (int i = 0; i < 4; i++){
        if (!IsSoundPlaying(sound->waterSplash[i])){
            PlaySound(sound->waterSplash[i]);
            return;
        }
    }
}
void shutdownSound(SoundManager *sound){
    UnloadMusicStream(sound->menuMusic);
    UnloadMusicStream(sound->gameMusic);
    CloseAudioDevice();
}
