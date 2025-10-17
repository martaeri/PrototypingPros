#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32   // <-- Correct height for your display
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Eye properties (scaled for 32px height)
int eyeWidth = 12;       // slightly narrower
int eyeHeight = 20;      // smaller to fit within 32px
int eyeY = 6;            // start a bit lower (top padding)
int leftEyeX = 30;
int rightEyeX = 82;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

void loop() {
  for (int i = -3; i <= 3; i++) {
    drawEyes(i);
    delay(100);
  }
  for (int i = 3; i >= -3; i--) {
    drawEyes(i);
    delay(100);
  }

  blinkEyes();
  deadEyes();
  sadEyes2();
  happyEyes();
  sleepingEyes();
  shockedEyes();
  delay(1500);
}

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


void sadEyes2() {
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

  for (int t = 0; t < arcThickness; t++) {
    display.drawLine(leftXCenter - xSize/2, leftYCenter - ySize/2 + t,
                     leftXCenter + xSize/2, leftYCenter + ySize/2 + t, SSD1306_BLACK);
    display.drawLine(leftXCenter - xSize/2, leftYCenter + ySize/2 + t,
                     leftXCenter + xSize/2, leftYCenter - ySize/2 + t, SSD1306_BLACK);
  }

  // --- RIGHT X ---
  int rightXCenter = rightEyeX + eyeWidth/2;
  int rightYCenter = cy + 2;

  for (int t = 0; t < arcThickness; t++) {
    display.drawLine(rightXCenter - xSize/2, rightYCenter - ySize/2 + t,
                     rightXCenter + xSize/2, rightYCenter + ySize/2 + t, SSD1306_BLACK);
    display.drawLine(rightXCenter - xSize/2, rightYCenter + ySize/2 + t,
                     rightXCenter + xSize/2, rightYCenter - ySize/2 + t, SSD1306_BLACK);
  }

  display.display();
  delay(3000);  // hold dead pose longer
}

