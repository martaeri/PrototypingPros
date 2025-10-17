#include "pitches.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "eyes_functions.h"

// ----- CONFIG ------

int SCREEN_WIDTH = 128;
int SCREEN_HEIGHT = 32;
int OLED_RESET = -1;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int eyeWidth = 12;
int eyeHeight = 20;
int eyeY = 6;
int leftEyeX = 30;
int rightEyeX = 82;


int ledPins[] = {8, 9, 10, 11, 12};   // LEDs representing health
const int numLeds = 5;

int buttonPins[] = {4, 5, 6, 7};         // Buttons for digits 1, 2, 3, 4
const int numButtons = 4;

const int speakerPin = 3;

// ----- GAME LOGIC -----

const int codeLength = 4;
int input[codeLength];
int inputIndex = 0;

int numPlayers = 5;
int codes[5][4] = {
  {1, 4, 3, 1},
  {2, 4, 1, 4},
  {4,4,4,4},
  {3,3,3,3},
  {1, 2, 3, 4} // <- starting Owner
};

int currentOwner[4] = {1, 2, 3, 4};
int lastCorrectCode[4] = {-1, -1, -1, -1};
int pendingOwner[4] = {-1, -1, -1, -1};

int health = 3;                        // Start with 3 lights on
unsigned long lastDecayTime = 0;
const unsigned long decayInterval = 30000; // 30 seconds per health loss

bool inputLocked = false;
unsigned long lockStartTime = 0;
const unsigned long lockDuration = 500;   // 0.5 second lockout

bool isDead = false;

// ----- CYCLE -----
bool cycleProtectionActive = true;
int protectedInputsRemaining = 2;

// ----- PENDING -----
bool isPending = false;
unsigned long pendingStartTime = 0;
const unsigned long pendingTimeout = 30000; // 30 seconds
int pendingLedIndex = 0;
unsigned long lastPendingLedTime = 0;
const unsigned long pendingLedDelay = 100; // LED chase speed

// ----- NON-BLOCKING EYE ANIMATIONS
extern EyeAnimationType currentEyeAnimation;
extern EyeAnimationType currentEyeAnimation;
extern bool eyeAnimationActive;
extern unsigned long eyeAnimationStartTime;
extern int eyeAnimationStep;
extern unsigned long lastEyeUpdate;

// ----- SETUP -----

void setup() {
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(speakerPin, OUTPUT);

  randomSeed(42);
  // if not for this line the third input would always trigger PENDING. Maybe useful for demo

  // OLED Setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  drawEyes(0);

  updateHealthDisplay();
  // maybe play alive sound
}


void loop() {
  unsigned long currentTime = millis();

  updateEyeAnimations();

  // Dead eyes if dead
  if (isDead) {
    if (!isEyeAnimationActive()) {
      startEyeAnimation(EYE_DEAD);
    }
    return;
  }

  // PENDING MODE
  if (isPending) {
    runPendingAnimation();
    if (!isEyeAnimationActive()) {
      startEyeAnimation(EYE_SHOCKED);
    }
    // Timeout check
    if (currentTime - pendingStartTime > pendingTimeout) {
      cancelPending();
    }
    handlePendingInput();
    return; // skip normal logic
  }

  // Handle health decay over time
  if (!isDead && currentTime - lastDecayTime > decayInterval && health > 0) {
    health--;
    updateHealthDisplay();
    playSadTone();
    startEyeAnimation(EYE_SAD);
    lastDecayTime = currentTime;

    // Check if now dead
    if (health == 0) {
      isDead = true;
      startEyeAnimation(EYE_DEAD);
    }
  }

  // Handle input lockout timing
  if (inputLocked && currentTime - lockStartTime >= lockDuration) {
    inputLocked = false; // unlock input after delay
  }

  // Skip reading buttons if input locked or dead
  if (inputLocked || isDead) return;

  // Handle button input
  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttonPins[i]) == LOW) { // button pressed
      delay(50); // debounce
      while (digitalRead(buttonPins[i]) == LOW); // wait for release

      input[inputIndex] = i + 1; // store digit (1,2,3)
      inputIndex++;

      if (inputIndex == codeLength) {
        checkCode();
        inputIndex = 0;
      }
    }
  }

  // Default eyes blink every few seconds
  static unsigned long lastBlink = 0;
  if (currentTime - lastBlink > 4000 && !isEyeAnimationActive()) {
    startEyeAnimation(EYE_BLINK);
    lastBlink = currentTime;
  }

  // Show idle eyes when no animation is active
  static bool idleEyesShown = false;
  if (!isEyeAnimationActive() && !idleEyesShown) {
    drawEyes(0);
    idleEyesShown = true;
  } else if (isEyeAnimationActive()) {
    idleEyesShown = false;
  }
}

void checkCode() {
  bool valid = false;
  bool sameAsOwner = true;
  bool sameAsLast = true;
  int codeIndex = -1;

  // check for shutdown code
  int shutdownCode[4] = {1,2,1,2};
  bool isShutdown = true;
  for (int i = 0; i < 4; i++) {
    if (input[i] != shutdownCode[i]) {
      isShutdown = false;
      break;
    }
  }

  if (isShutdown) {
    startEyeAnimation(EYE_SLEEPING);
    playShutdownTone();
    delay(2000);
    // turn everything off
    display.clearDisplay();
    display.display();
    for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], LOW);
    noTone(speakerPin);
    while (true); // halt device
  }

  // check if code exists
  for (int c = 0; c < numPlayers; c++) {
    bool match = true;
    for (int j = 0; j < codeLength; j++) {
      if (input[j] != codes[c][j]) match = false;
    }
    if (match) {
      valid = true;
      codeIndex = c;
      break;
    }
  }

  // check if same as current owner
  for (int j = 0; j < codeLength; j++) {
    if (input[j] != currentOwner[j]) sameAsOwner = false;
  }

  // check if same as last correct code
  for (int j = 0; j < codeLength; j++) {
    if (input[j] != lastCorrectCode[j]) sameAsLast = false;
  }


  if (valid && !sameAsOwner && !sameAsLast && !isDead) {
    // Successful code & not current owner & not last input
    if (health < numLeds) {
      health++;
      startEyeAnimation(EYE_HAPPY);
    }
    updateHealthDisplay();
    playSuccessTone();

    // handle potential owner change
    if (cycleProtectionActive) {
      // Too few correct code inputs to be possible new owner
      protectedInputsRemaining--;
      if (protectedInputsRemaining <= 0) {
        cycleProtectionActive = false;
      }
    } else {
      int chance = random(0, 100);
      if (chance < 30) {  // 30% chance for new owner
        enterPendingState();
        for (int j = 0; j < codeLength; j++) {
          pendingOwner[j] = input[j];
        }
        startEyeAnimation(EYE_SHOCKED);
        playPendingTone();
        return;
      }
    }

    // remember last used correct code
    for (int j = 0; j < codeLength; j++) {
      lastCorrectCode[j] = input[j];
    }

  } else if (valid) {
    // Valid code but same as owner or same as last correct
    playErrorTone();
    startEyeAnimation(EYE_SAD);
    blinkActiveLedsTwice();
  } else {
    // Invalid code
    playErrorTone();
    startEyeAnimation(EYE_SAD);
    blinkActiveLedsTwice();
  }

  inputLocked = true;
  lockStartTime = millis();
}

// ----- PENDING MODE -----

void enterPendingState() {
  isPending = true;
  pendingStartTime = millis();
  pendingLedIndex = 0;
  for (int i = 0; i < numLeds; i++) digitalWrite(ledPins[i], LOW);
  startEyeAnimation(EYE_SHOCKED);
}

void cancelPending() {
  isPending = false;
  updateHealthDisplay();
  startEyeAnimation(EYE_BLINK);
}

void confirmPending() {
  for (int j = 0; j < codeLength; j++) {
    currentOwner[j] = pendingOwner[j];
  }
  cycleProtectionActive = true;
  protectedInputsRemaining = 2;
  isPending = false;
  playCelebrationTone();
  startEyeAnimation(EYE_HAPPY);
  updateHealthDisplay();
}

void handlePendingInput() {
  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(50);
      while (digitalRead(buttonPins[i]) == LOW);
      input[inputIndex] = i + 1;
      inputIndex++;
      if (inputIndex == codeLength) {
        // compare with pendingOwner
        bool same = true;
        for (int j = 0; j < codeLength; j++) {
          if (input[j] != pendingOwner[j]) same = false;
        }
        if (same) {
          confirmPending();
        } else {
          cancelPending();
          startEyeAnimation(EYE_SAD);
          playErrorTone();
        }
        inputIndex = 0;
      }
    }
  }
}


// ------ Sound Functions -----

void playSadTone() {
  tone(speakerPin, NOTE_C4, 200);
  delay(200);
  tone(speakerPin, NOTE_A3, 400);
  delay(400);
  noTone(speakerPin);
}

void playSuccessTone() {
  tone(speakerPin, NOTE_G5, 120);
  delay(150);
  tone(speakerPin, NOTE_C6, 150);
  delay(150);
  noTone(speakerPin);
}

void playErrorTone() {
  tone(speakerPin, NOTE_C4, 200);
  delay(150);
  tone(speakerPin, NOTE_C3, 200);
  delay(200);
  noTone(speakerPin);
}

void playCelebrationTone() {
  tone(speakerPin, NOTE_C6, 150);
  delay(150);
  tone(speakerPin, NOTE_E6, 150);
  delay(150);
  tone(speakerPin, NOTE_G6, 150);
  delay(150);
  tone(speakerPin, NOTE_C7, 300);
  delay(300);
  tone(speakerPin, NOTE_G6, 150);
  delay(150);
  tone(speakerPin, NOTE_E6, 150);
  delay(150);
  tone(speakerPin, NOTE_C6, 400);
  delay(400);
  noTone(speakerPin);
}

void playPendingTone() {
  tone(speakerPin, NOTE_D6, 150);
  delay(150);
  tone(speakerPin, NOTE_G6, 150);
  delay(150);
  tone(speakerPin, NOTE_D7, 300);
  delay(300);
  noTone(speakerPin);
}

// ---- OLD SOUNDS
void playHealthLostSound() {
  tone(speakerPin, 600, 150);
  delay(150);
  tone(speakerPin, 400, 200);
  delay(200);
  tone(speakerPin, 300, 300);
  delay(300);
  noTone(speakerPin);
}

void playHealthRestoredSound() {
  tone(speakerPin, 500, 150);
  delay(150);
  tone(speakerPin, 700, 150);
  delay(150);
  tone(speakerPin, 900, 200);
  delay(200);
  noTone(speakerPin);
}

void playWrongCodeSound() {
  for (int i = 0; i < 2; i++) {
    tone(speakerPin, 200, 100);
    delay(150);
  }
  noTone(speakerPin);
}

void playShutdownTone() {
  tone(speakerPin, NOTE_C5, 200);
  delay(200);
  tone(speakerPin, NOTE_G4, 200);
  delay(200);
  tone(speakerPin, NOTE_E4, 400);
  delay(400);
  tone(speakerPin, NOTE_C4, 800);
  delay(800);
  noTone(speakerPin);
}


// ----- LED Functions ------

void updateHealthDisplay() {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], i < health ? HIGH : LOW);
  }
}

void blinkActiveLedsTwice() {
  for (int b = 0; b < 2; b++) {
    blinkActiveLeds(HIGH);
    delay(150);
    blinkActiveLeds(LOW);
    delay(150);
  }
  updateHealthDisplay();
}

void blinkActiveLeds(int state) {
  for (int i = 0; i < health; i++) {
    digitalWrite(ledPins[i], state);
  }
}

void runPendingAnimation() {
  unsigned long now = millis();
  if (now - lastPendingLedTime >= pendingLedDelay) {
    for (int i = 0; i < numLeds; i++)
      digitalWrite(ledPins[i], (i == pendingLedIndex) ? HIGH : LOW);
    pendingLedIndex = (pendingLedIndex + 1) % numLeds;
    lastPendingLedTime = now;
  }
}