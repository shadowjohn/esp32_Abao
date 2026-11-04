
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_W 128
#define OLED_H 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(
  OLED_W, OLED_H, &Wire, -1
);

enum Mood {
  NORMAL,
  HAPPY,
  ANGRY,
  SURPRISED,
  SLEEP
};

Mood mood = NORMAL;

bool blinking = false;
int eyeOffset = 0;

unsigned long nextBlink = 0;
unsigned long blinkEnd = 0;
unsigned long nextLook = 0;
unsigned long nextMood = 0;
unsigned long lastFrame = 0;

void scheduleBlink(unsigned long now) {
  nextBlink = now + random(2000, 6000);
}

void drawEyes() {
  const int y = 31;

  if (blinking || mood == SLEEP) {
    display.drawLine(25, y + 9, 47, y + 9, WHITE);
    display.drawLine(81, y + 9, 103, y + 9, WHITE);
    return;
  }

  int leftX = 36 + eyeOffset;
  int rightX = 92 + eyeOffset;

  switch (mood) {
    case NORMAL:
      display.fillRoundRect(
        leftX - 8, y, 16, 24, 7, WHITE
      );
      display.fillRoundRect(
        rightX - 8, y, 16, 24, 7, WHITE
      );
      break;

    case HAPPY:
      display.drawCircle(leftX, y + 14, 12, WHITE);
      display.drawCircle(rightX, y + 14, 12, WHITE);
      display.fillRect(20, y + 14, 88, 16, BLACK);
      break;

    case ANGRY:
      display.fillRoundRect(
        leftX - 8, y + 8, 16, 16, 4, WHITE
      );
      display.fillRoundRect(
        rightX - 8, y + 8, 16, 16, 4, WHITE
      );
      display.drawLine(
        leftX - 12, y, leftX + 10, y + 8, WHITE
      );
      display.drawLine(
        rightX - 10, y + 8, rightX + 12, y, WHITE
      );
      break;

    case SURPRISED:
      display.drawCircle(leftX, y + 12, 12, WHITE);
      display.drawCircle(rightX, y + 12, 12, WHITE);
      display.fillCircle(leftX, y + 12, 4, WHITE);
      display.fillCircle(rightX, y + 12, 4, WHITE);
      break;

    case SLEEP:
      break;
  }
}

void drawFace() {
  display.clearDisplay();

  // OLED 上方黃色區域
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(34, 3);
  display.print("ABAO v0.2");

  // OLED 下方藍色區域
  drawEyes();

  display.display();
}

void updateAnimation(unsigned long now) {
  if (mood != SLEEP) {
    if (!blinking && now >= nextBlink) {
      blinking = true;
      blinkEnd = now + 120;
    }

    if (blinking && now >= blinkEnd) {
      blinking = false;
      scheduleBlink(now);
    }
  }

  if (now >= nextLook) {
    eyeOffset = random(-6, 7);
    nextLook = now + random(1200, 3500);
  }

  if (now >= nextMood) {
    mood = (Mood)random(0, 5);
    nextMood = now + random(4000, 9000);
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  if (!display.begin(
      SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (true) {
      delay(1000);
    }
  }

  randomSeed(micros());

  unsigned long now = millis();

  scheduleBlink(now);
  nextLook = now + 1500;
  nextMood = now + 5000;

  Serial.println("Abao v0.2 ready!");
  drawFace();
}

void loop() {
  unsigned long now = millis();

  updateAnimation(now);

  // 約 30 FPS，非阻塞式更新
  if (now - lastFrame >= 33) {
    lastFrame = now;
    drawFace();
  }
}
