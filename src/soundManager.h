#ifndef HEADER_SOUND
#define HEADER_SOUND

#include "gameStruct.h"

SoundManager *initSound();
void updateSound(SoundManager *soundManager, GameStruct *game);
void shutdownSound(SoundManager *soundManager);
void playSlowdownSound(SoundManager *sound);
void playSpeedupSound(SoundManager *sound);
void playRedPowerupSound(SoundManager *sound);
void playBluePowerupSound(SoundManager *sound);
void playUpperBouncerSound(SoundManager *sound);
void playClick(SoundManager *sound);
void playBounce(SoundManager *sound);
void playBounce2(SoundManager *sound);
void playLaunch(SoundManager *sound);
void playFlipper(SoundManager *sound);
void playWater(SoundManager *sound);
void playWaterSplash(SoundManager *sound);

#endif
