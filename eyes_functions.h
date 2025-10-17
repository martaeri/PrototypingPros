#ifndef EYES_FUNCTIONS_H
#define EYES_FUNCTIONS_H

#include <Arduino.h>

// Eye animation types
enum EyeAnimationType {
  EYE_IDLE,
  EYE_BLINK,
  EYE_SAD,
  EYE_HAPPY,
  EYE_SLEEPING,
  EYE_SHOCKED,
  EYE_DEAD
};

// Non-blocking animation functions
void startEyeAnimation(EyeAnimationType type);
void updateEyeAnimations();
bool isEyeAnimationActive();

// Helper functions for non-blocking animations
void updateBlinkAnimation(unsigned long currentTime);
void updateSadAnimation(unsigned long currentTime);
void updateHappyAnimation(unsigned long currentTime);
void updateShockedAnimation(unsigned long currentTime);
void updateDeadAnimation(unsigned long currentTime);
void updateSleepingAnimation(unsigned long currentTime);

// Original blocking functions (kept for reference/emergency use)
void drawEyes(int moveX);
void blinkEyes();
void sadEyes();
void happyEyes();
void sleepingEyes();
void shockedEyes();
void deadEyes();

#endif
