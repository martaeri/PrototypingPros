#include "eyes_functions.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Diese Variablen kommen aus der Hauptdatei:
extern Adafruit_SSD1306 display;
extern int leftEyeX;
extern int rightEyeX;
extern int eyeY;
extern int eyeWidth;
extern int eyeHeight;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

// Non-blocking animation state
EyeAnimationType currentEyeAnimation = EYE_IDLE;
bool eyeAnimationActive = false;
unsigned long eyeAnimationStartTime = 0;
int eyeAnimationStep = 0;
unsigned long lastEyeUpdate = 0;

// Animation timing constants
const unsigned long BLINK_FRAME_DELAY = 40;
const unsigned long BLINK_CLOSED_DELAY = 150;
const unsigned long SAD_FRAME_DELAY = 60;
const unsigned long SAD_HOLD_DELAY = 1000;
const unsigned long HAPPY_FRAME_DELAY = 60;
const unsigned long HAPPY_HOLD_DELAY = 1000;
const unsigned long SHOCKED_VIBRATE_DELAY = 25;
const unsigned long DEAD_FADE_DELAY = 80;
const unsigned long DEAD_HOLD_DELAY = 3000;
const unsigned long SLEEP_FRAME_DELAY = 400;

void drawEyes(int moveX) {
  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.fillRect(leftEyeX + moveX, eyeY, eyeWidth, eyeHeight, SSD1306_BLACK);
  display.fillRect(rightEyeX + moveX, eyeY, eyeWidth, eyeHeight, SSD1306_BLACK);
  display.display();
}

void blinkEyes() {
  for (int h = eyeHeight; h >= 2; h -= 3) {
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.fillRect(leftEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.fillRect(rightEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.display();
    delay(40);
  }

  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.display();
  delay(150);

  for (int h = 2; h <= eyeHeight; h += 3) {
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.fillRect(leftEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.fillRect(rightEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.display();
    delay(40);
  }
}


void sadEyes() {
  int steps = 12;  // number of animation frames

  for (int step = 0; step <= steps; step++) {
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    float t = float(step) / steps;      // 0 → 1
    int newHeight = eyeHeight - t * 8;  // squish vertically
    int widen = t * 6;                  // slight widening
    int bottomNarrow = t * 6;           // bottom narrower (sad shape)
    int tilt = t * 3;                   // side tilt for diagonal look

    // --- LEFT EYE ---
    for (int y = 0; y < newHeight; y++) {
      float interp = float(y) / newHeight;
      // top is wider, bottom narrower
      int widthAtY = eyeWidth + widen - int((1.0 - interp) * bottomNarrow);
      int xCenter = leftEyeX + eyeWidth / 2;

      // MIRRORED tilt: left eye tilts outward (left edge drops)
      int xStart = xCenter - widthAtY / 2 + tilt - int(interp * 2 * tilt);

      int yPos = eyeY + (eyeHeight - newHeight) / 2 + y;
      display.drawFastHLine(xStart, yPos, widthAtY, SSD1306_BLACK);
    }

    // --- RIGHT EYE (mirror horizontally) ---
    for (int y = 0; y < newHeight; y++) {
      float interp = float(y) / newHeight;
      int widthAtY = eyeWidth + widen - int((1.0 - interp) * bottomNarrow);
      int xCenter = rightEyeX + eyeWidth / 2;

      // MIRRORED tilt: right eye tilts outward (right edge drops)
      int xStart = xCenter - widthAtY / 2 - tilt + int(interp * 2 * tilt);

      int yPos = eyeY + (eyeHeight - newHeight) / 2 + y;
      display.drawFastHLine(xStart, yPos, widthAtY, SSD1306_BLACK);
    }

    // --- EYEBROWS (sad expression) ---
    int browLength = 16;
    int browY = eyeY - 4;
    int browTilt = t * 6;
    int browDrop = t * 2;

    // Left eyebrow: inner up, outer down
    display.drawLine(
      leftEyeX - 2, browY + browDrop + browTilt,            // outer droops
      leftEyeX + browLength, browY + browDrop - browTilt,   // inner lifts
      SSD1306_BLACK
    );

    // Right eyebrow: mirror (outer droops)
    display.drawLine(
      rightEyeX + eyeWidth + 2, browY + browDrop + browTilt,          // outer droops
      rightEyeX + eyeWidth - browLength, browY + browDrop - browTilt, // inner lifts
      SSD1306_BLACK
    );

    display.display();
    delay(60);
  }

  delay(1000); // Hold final sad shape
}


void happyEyes() {
  int steps = 14;  // number of animation frames

  for (int step = 0; step <= steps; step++) {
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    float t = float(step) / steps;  // 0 → 1 morph progress

    // Parameters for the happy eye arcs
    int arcHeight = 4 + int(t * 4);       // max vertical height of the arc
    int arcWidth  = eyeWidth + 16;        // wide arcs
    int arcThickness = 6;                 // thicker arcs
    int cy = SCREEN_HEIGHT / 2;           // vertical center for arcs

    // --- LEFT EYE ---
    int cx = leftEyeX + eyeWidth / 2;
    for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
      for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
        float normX = float(x) / (arcWidth / 2);     
        int yOffset = int(-arcHeight * (1 - normX * normX)) + yOffsetShift; // parabola
        if (cy + yOffset >= 0 && cy + yOffset < SCREEN_HEIGHT)
          display.drawPixel(cx + x, cy + yOffset, SSD1306_BLACK);
      }
    }

    // --- RIGHT EYE ---
    cx = rightEyeX + eyeWidth / 2;
    for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
      for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
        float normX = float(x) / (arcWidth / 2);
        int yOffset = int(-arcHeight * (1 - normX * normX)) + yOffsetShift;
        if (cy + yOffset >= 0 && cy + yOffset < SCREEN_HEIGHT)
          display.drawPixel(cx + x, cy + yOffset, SSD1306_BLACK);
      }
    }

    display.display();
    delay(60);
  }

  delay(1000);  // Hold the happy pose
}

void sleepingEyes() {
  int arcWidth  = eyeWidth + 16;  // match the happy arcs
  int arcThickness = 6;           // thick lines
  int cy = SCREEN_HEIGHT - 8;     // base position near the bottom

  // Two-step drift: eyes at cy (up) and cy + 2 (down)
  int yShifts[2] = {0, 2};

  for (int i = 0; i < 4; i++) { // loop through up/down twice for smooth looping
    int yShift = yShifts[i % 2];

    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    // --- LEFT EYE ---
    int cx = leftEyeX + eyeWidth / 2;
    for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
      for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
        int yPos = cy + yOffsetShift + yShift;
        if (yPos >= 0 && yPos < SCREEN_HEIGHT)
          display.drawPixel(cx + x, yPos, SSD1306_BLACK);
      }
    }

    // --- RIGHT EYE ---
    cx = rightEyeX + eyeWidth / 2;
    for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
      for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
        int yPos = cy + yOffsetShift + yShift;
        if (yPos >= 0 && yPos < SCREEN_HEIGHT)
          display.drawPixel(cx + x, yPos, SSD1306_BLACK);
      }
    }

    // --- Z's for snoring ---
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(95, 2); display.print("Z");
    display.setCursor(105, 6); display.print("Z");
    display.setCursor(115, 10); display.print("Z");

    display.display();
    delay(400);  // slower delay for a smooth sleep rhythm
  }

  // Hold final sleeping pose
  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
    int yPos = cy + yOffsetShift;
    for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
      display.drawPixel(leftEyeX + eyeWidth / 2 + x, yPos, SSD1306_BLACK);
      display.drawPixel(rightEyeX + eyeWidth / 2 + x, yPos, SSD1306_BLACK);
    }
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(95, 2); display.print("Z");
  display.setCursor(105, 6); display.print("Z");
  display.setCursor(115, 10); display.print("Z");
  display.display();
  delay(1500);
}

void shockedEyes() {
  int stepsExpand = 5;   // frames for rapid widening
  int maxHeight   = 26;  // maximum eye height when shocked
  int maxWidth    = eyeWidth + 8;  // slightly wider than normal
  int eyeYCenter  = SCREEN_HEIGHT / 2 - maxHeight / 2;

  // --- Step 1: Rapid widening ---
  for (int step = 0; step <= stepsExpand; step++) {
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    // Interpolate height and width
    int h = eyeHeight + ((maxHeight - eyeHeight) * step) / stepsExpand;
    int w = eyeWidth + ((maxWidth - eyeWidth) * step) / stepsExpand;

    int leftX  = leftEyeX + (eyeWidth - w)/2;
    int rightX = rightEyeX + (eyeWidth - w)/2;
    int yTop   = eyeYCenter + (maxHeight - h)/2;

    display.fillRect(leftX, yTop, w, h, SSD1306_BLACK);
    display.fillRect(rightX, yTop, w, h, SSD1306_BLACK);

    display.display();
    delay(60);
  }

  // --- Step 2: Continuous vibration ---
  int leftXBase  = leftEyeX + (eyeWidth - maxWidth)/2;
  int rightXBase = rightEyeX + (eyeWidth - maxWidth)/2;
  int maxOffset  = 3;  // pixels to shake left/right

  while (true) {  // infinite vibration loop
    for (int offset = -maxOffset; offset <= maxOffset; offset += 2) {
      display.clearDisplay();
      display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

      display.fillRect(leftXBase + offset, eyeYCenter, maxWidth, maxHeight, SSD1306_BLACK);
      display.fillRect(rightXBase + offset, eyeYCenter, maxWidth, maxHeight, SSD1306_BLACK);

      display.display();
      delay(25);
    }
    for (int offset = maxOffset; offset >= -maxOffset; offset -= 2) {
      display.clearDisplay();
      display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

      display.fillRect(leftXBase + offset, eyeYCenter, maxWidth, maxHeight, SSD1306_BLACK);
      display.fillRect(rightXBase + offset, eyeYCenter, maxWidth, maxHeight, SSD1306_BLACK);

      display.display();
      delay(25);
    }
  }
}


void deadEyes() {
  int arcWidth = eyeWidth + 16;  // match wide arcs
  int arcThickness = 6;           // thickness for eyes and X's
  int cy = SCREEN_HEIGHT / 2.5;     // vertical center
  int stepsFade = 6;              // frames for fading/shrinking

  // --- Stage 1: Eyes fade downward ---
  for (int step = 0; step <= stepsFade; step++) {
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    int yShift = (step * 6) / stepsFade;
    int thickness = arcThickness - (step * arcThickness) / stepsFade;
    if (thickness < 1) thickness = 1;

    // --- LEFT EYE ---
    int cx = leftEyeX + eyeWidth / 2;
    for (int yOffset = 0; yOffset < thickness; yOffset++) {
      for (int x = -arcWidth/2; x <= arcWidth/2; x++) {
        int yPos = cy + yOffset + yShift;
        if (yPos >= 0 && yPos < SCREEN_HEIGHT)
          display.drawPixel(cx + x, yPos, SSD1306_BLACK);
      }
    }

    // --- RIGHT EYE ---
    cx = rightEyeX + eyeWidth / 2;
    for (int yOffset = 0; yOffset < thickness; yOffset++) {
      for (int x = -arcWidth/2; x <= arcWidth/2; x++) {
        int yPos = cy + yOffset + yShift;
        if (yPos >= 0 && yPos < SCREEN_HEIGHT)
          display.drawPixel(cx + x, yPos, SSD1306_BLACK);
      }
    }

    display.display();
    delay(80);
  }

  // --- Stage 2: Draw even bigger thick X's using lines ---
  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

  int xSize = 24; // wider X
  int ySize = 18; // taller X

  // --- LEFT X ---
  int leftXCenter = leftEyeX + eyeWidth/2;
  int leftYCenter = cy + 2;

  for (int t = 0; t < 6; t++) {
    display.drawLine(leftXCenter - xSize/2, leftYCenter - ySize/2 + t,
                     leftXCenter + xSize/2, leftYCenter + ySize/2 + t, SSD1306_BLACK);
    display.drawLine(leftXCenter - xSize/2, leftYCenter + ySize/2 + t,
                     leftXCenter + xSize/2, leftYCenter - ySize/2 + t, SSD1306_BLACK);
  }

  // --- RIGHT X ---
  int rightXCenter = rightEyeX + eyeWidth/2;
  int rightYCenter = cy + 2;

  for (int t = 0; t < 6; t++) {
    display.drawLine(rightXCenter - xSize/2, rightYCenter - ySize/2 + t,
                     rightXCenter + xSize/2, rightYCenter + ySize/2 + t, SSD1306_BLACK);
    display.drawLine(rightXCenter - xSize/2, rightYCenter + ySize/2 + t,
                     rightXCenter + xSize/2, rightYCenter - ySize/2 + t, SSD1306_BLACK);
  }

  display.display();
  delay(3000);  // hold dead pose longer
}

// ----- NON-BLOCKING ANIMATION CONTROL -----

void startEyeAnimation(EyeAnimationType type) {
  currentEyeAnimation = type;
  eyeAnimationActive = true;
  eyeAnimationStartTime = millis();
  eyeAnimationStep = 0;
  lastEyeUpdate = millis();
}

bool isEyeAnimationActive() {
  return eyeAnimationActive;
}

void updateEyeAnimations() {
  if (!eyeAnimationActive) {
    return;
  }

  unsigned long currentTime = millis();
  
  switch (currentEyeAnimation) {
    case EYE_BLINK:
      updateBlinkAnimation(currentTime);
      break;
    case EYE_SAD:
      updateSadAnimation(currentTime);
      break;
    case EYE_HAPPY:
      updateHappyAnimation(currentTime);
      break;
    case EYE_SHOCKED:
      updateShockedAnimation(currentTime);
      break;
    case EYE_DEAD:
      updateDeadAnimation(currentTime);
      break;
    case EYE_SLEEPING:
      updateSleepingAnimation(currentTime);
      break;
    default:
      eyeAnimationActive = false;
      break;
  }
}

void updateBlinkAnimation(unsigned long currentTime) {
  if (currentTime - lastEyeUpdate < BLINK_FRAME_DELAY) return;
  
  if (eyeAnimationStep < 6) {
    // Closing phase
    int h = eyeHeight - (eyeAnimationStep * 3);
    if (h < 2) h = 2;
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.fillRect(leftEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.fillRect(rightEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.display();
    
    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else if (eyeAnimationStep == 6) {
    // Closed phase
    if (currentTime - lastEyeUpdate < BLINK_CLOSED_DELAY) return;
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.display();
    
    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else if (eyeAnimationStep < 13) {
    // Opening phase
    int h = 2 + ((eyeAnimationStep - 6) * 3);
    if (h > eyeHeight) h = eyeHeight;
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.fillRect(leftEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.fillRect(rightEyeX, eyeY + (eyeHeight - h) / 2, eyeWidth, h, SSD1306_BLACK);
    display.display();
    
    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else {
    // Animation complete
    eyeAnimationActive = false;
  }
}

void updateSadAnimation(unsigned long currentTime) {
  if (eyeAnimationStep < 12) {
    if (currentTime - lastEyeUpdate < SAD_FRAME_DELAY) return;
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    float t = float(eyeAnimationStep) / 12;
    int newHeight = eyeHeight - t * 8;
    int widen = t * 6;
    int bottomNarrow = t * 6;
    int tilt = t * 3;

    // Draw sad eyes (simplified version)
    for (int y = 0; y < newHeight; y++) {
      float interp = float(y) / newHeight;
      int widthAtY = eyeWidth + widen - int((1.0 - interp) * bottomNarrow);
      
      // Left eye
      int xStart = leftEyeX + eyeWidth / 2 - widthAtY / 2 + tilt - int(interp * 2 * tilt);
      int yPos = eyeY + (eyeHeight - newHeight) / 2 + y;
      display.drawFastHLine(xStart, yPos, widthAtY, SSD1306_BLACK);
      
      // Right eye
      xStart = rightEyeX + eyeWidth / 2 - widthAtY / 2 - tilt + int(interp * 2 * tilt);
      display.drawFastHLine(xStart, yPos, widthAtY, SSD1306_BLACK);
    }

    // Eyebrows
    int browLength = 16;
    int browY = eyeY - 4;
    int browTilt = t * 6;
    int browDrop = t * 2;

    display.drawLine(leftEyeX - 2, browY + browDrop + browTilt,
                     leftEyeX + browLength, browY + browDrop - browTilt, SSD1306_BLACK);
    display.drawLine(rightEyeX + eyeWidth + 2, browY + browDrop + browTilt,
                     rightEyeX + eyeWidth - browLength, browY + browDrop - browTilt, SSD1306_BLACK);

    display.display();
    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else if (eyeAnimationStep == 12) {
    if (currentTime - lastEyeUpdate < SAD_HOLD_DELAY) return;
    eyeAnimationActive = false;
  }
}

void updateSleepingAnimation(unsigned long currentTime) {
  static bool eyesUp = true;

  if (currentTime - lastEyeUpdate < SLEEP_FRAME_DELAY) return;

  // toggle between eyes slightly up and down
  eyesUp = !eyesUp;
  int arcWidth  = eyeWidth + 16;
  int arcThickness = 6;
  int cyBase = SCREEN_HEIGHT - 8;
  int yShift = eyesUp ? 0 : 2;

  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

  // --- LEFT EYE ---
  int cx = leftEyeX + eyeWidth / 2;
  for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
    for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
      int yPos = cyBase + yOffsetShift + yShift;
      if (yPos >= 0 && yPos < SCREEN_HEIGHT)
        display.drawPixel(cx + x, yPos, SSD1306_BLACK);
    }
  }

  // --- RIGHT EYE ---
  cx = rightEyeX + eyeWidth / 2;
  for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
    for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
      int yPos = cyBase + yOffsetShift + yShift;
      if (yPos >= 0 && yPos < SCREEN_HEIGHT)
        display.drawPixel(cx + x, yPos, SSD1306_BLACK);
    }
  }

  // --- "Z" Snore bubbles ---
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(95, 2); display.print("Z");
  display.setCursor(105, 6); display.print("Z");
  display.setCursor(115, 10); display.print("Z");

  display.display();

  // keep looping this gently forever
  eyeAnimationActive = true;
  lastEyeUpdate = currentTime;
}


void updateHappyAnimation(unsigned long currentTime) {
  if (eyeAnimationStep < 14) {
    if (currentTime - lastEyeUpdate < HAPPY_FRAME_DELAY) return;
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    float t = float(eyeAnimationStep) / 14;
    int arcHeight = 4 + int(t * 4);
    int arcWidth = eyeWidth + 16;
    int arcThickness = 6;
    int cy = SCREEN_HEIGHT / 2;

    // Draw happy arcs (left eye)
    int cx = leftEyeX + eyeWidth / 2;
    for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
      for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
        float normX = float(x) / (arcWidth / 2);
        int yOffset = int(-arcHeight * (1 - normX * normX)) + yOffsetShift;
        if (cy + yOffset >= 0 && cy + yOffset < SCREEN_HEIGHT)
          display.drawPixel(cx + x, cy + yOffset, SSD1306_BLACK);
      }
    }

    // Draw happy arcs (right eye)
    cx = rightEyeX + eyeWidth / 2;
    for (int yOffsetShift = 0; yOffsetShift < arcThickness; yOffsetShift++) {
      for (int x = -arcWidth / 2; x <= arcWidth / 2; x++) {
        float normX = float(x) / (arcWidth / 2);
        int yOffset = int(-arcHeight * (1 - normX * normX)) + yOffsetShift;
        if (cy + yOffset >= 0 && cy + yOffset < SCREEN_HEIGHT)
          display.drawPixel(cx + x, cy + yOffset, SSD1306_BLACK);
      }
    }

    display.display();
    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else if (eyeAnimationStep == 14) {
    if (currentTime - lastEyeUpdate < HAPPY_HOLD_DELAY) return;
    eyeAnimationActive = false;
  }
}

void updateShockedAnimation(unsigned long currentTime) {
  if (eyeAnimationStep < 5) {
    // Expanding phase
    if (currentTime - lastEyeUpdate < SHOCKED_VIBRATE_DELAY * 2) return;
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    int maxHeight = 26;
    int maxWidth = eyeWidth + 8;
    int h = eyeHeight + ((maxHeight - eyeHeight) * eyeAnimationStep) / 5;
    int w = eyeWidth + ((maxWidth - eyeWidth) * eyeAnimationStep) / 5;

    int leftX = leftEyeX + (eyeWidth - w)/2;
    int rightX = rightEyeX + (eyeWidth - w)/2;
    int yTop = SCREEN_HEIGHT / 2 - h / 2;

    display.fillRect(leftX, yTop, w, h, SSD1306_BLACK);
    display.fillRect(rightX, yTop, w, h, SSD1306_BLACK);
    display.display();

    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else {
    // Vibration phase - continues indefinitely until stopped
    if (currentTime - lastEyeUpdate < SHOCKED_VIBRATE_DELAY) return;
    
    int maxHeight = 26;
    int maxWidth = eyeWidth + 8;
    int maxOffset = 3;
    int offset = (eyeAnimationStep % 4 - 2) * maxOffset / 2; // -3, -1, 1, 3 pattern
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    int leftX = leftEyeX + (eyeWidth - maxWidth)/2 + offset;
    int rightX = rightEyeX + (eyeWidth - maxWidth)/2 + offset;
    int yTop = SCREEN_HEIGHT / 2 - maxHeight / 2;

    display.fillRect(leftX, yTop, maxWidth, maxHeight, SSD1306_BLACK);
    display.fillRect(rightX, yTop, maxWidth, maxHeight, SSD1306_BLACK);
    display.display();

    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  }
}

void updateDeadAnimation(unsigned long currentTime) {
  if (eyeAnimationStep <= 6) {
    if (currentTime - lastEyeUpdate < DEAD_FADE_DELAY) return;
    
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    int arcWidth = eyeWidth + 16;
    int arcThickness = 6;
    int cy = SCREEN_HEIGHT / 2.5;

    int yShift = (eyeAnimationStep * 6) / 6;
    int thickness = arcThickness - (eyeAnimationStep * arcThickness) / 6;
    if (thickness < 1) thickness = 1;

    // Draw fading eyes
    int cx = leftEyeX + eyeWidth / 2;
    for (int yOffset = 0; yOffset < thickness; yOffset++) {
      for (int x = -arcWidth/2; x <= arcWidth/2; x++) {
        int yPos = cy + yOffset + yShift;
        if (yPos >= 0 && yPos < SCREEN_HEIGHT)
          display.drawPixel(cx + x, yPos, SSD1306_BLACK);
      }
    }

    cx = rightEyeX + eyeWidth / 2;
    for (int yOffset = 0; yOffset < thickness; yOffset++) {
      for (int x = -arcWidth/2; x <= arcWidth/2; x++) {
        int yPos = cy + yOffset + yShift;
        if (yPos >= 0 && yPos < SCREEN_HEIGHT)
          display.drawPixel(cx + x, yPos, SSD1306_BLACK);
      }
    }

    display.display();
    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else if (eyeAnimationStep == 7) {
    // Draw X's
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    int xSize = 24;
    int ySize = 18;
    int cy = SCREEN_HEIGHT / 2.5;

    // Left X
    int leftXCenter = leftEyeX + eyeWidth/2;
    int leftYCenter = cy + 2;
    for (int t = 0; t < 6; t++) {
      display.drawLine(leftXCenter - xSize/2, leftYCenter - ySize/2 + t,
                       leftXCenter + xSize/2, leftYCenter + ySize/2 + t, SSD1306_BLACK);
      display.drawLine(leftXCenter - xSize/2, leftYCenter + ySize/2 + t,
                       leftXCenter + xSize/2, leftYCenter - ySize/2 + t, SSD1306_BLACK);
    }

    // Right X
    int rightXCenter = rightEyeX + eyeWidth/2;
    int rightYCenter = cy + 2;
    for (int t = 0; t < 6; t++) {
      display.drawLine(rightXCenter - xSize/2, rightYCenter - ySize/2 + t,
                       rightXCenter + xSize/2, rightYCenter + ySize/2 + t, SSD1306_BLACK);
      display.drawLine(rightXCenter - xSize/2, rightYCenter + ySize/2 + t,
                       rightXCenter + xSize/2, rightYCenter - ySize/2 + t, SSD1306_BLACK);
    }

    display.display();
    eyeAnimationStep++;
    lastEyeUpdate = currentTime;
  } else if (eyeAnimationStep == 8) {
    if (currentTime - lastEyeUpdate < DEAD_HOLD_DELAY) return;
    // Dead eyes stay forever - don't stop animation
    eyeAnimationStep = 7; // Keep showing X's
  }
}