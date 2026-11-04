#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ESP32-WROOM-32 NodeMCU 38-pin; OLED 128x64 I2C
constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 22;
constexpr int SERVO_PIN = 18;
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr int SERVO_MIN_US = 1000; // conservative initial calibration
constexpr int SERVO_MAX_US = 2000;
constexpr int ANGLE_MIN = 65;
constexpr int ANGLE_MAX = 115;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Servo neck;
float currentAngle = 90.0f;
float targetAngle = 90.0f;
float eyeX = 0.0f;
int targetEyeX = 0;
bool blinking = false;
uint32_t blinkUntil = 0, nextBlink = 0, nextLook = 0;
uint32_t lastMotion = 0, lastFrame = 0;

void setTarget(int angle) {
  targetAngle = constrain(angle, ANGLE_MIN, ANGLE_MAX);
  targetEyeX = map((int)targetAngle, ANGLE_MIN, ANGLE_MAX, -7, 7);
}

void drawFace() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(30, 3);
  oled.print("ABAO v0.3");
  const int offset = (int)roundf(eyeX);
  if (blinking) {
    oled.drawFastHLine(23 + offset, 43, 24, SSD1306_WHITE);
    oled.drawFastHLine(81 + offset, 43, 24, SSD1306_WHITE);
  } else {
    oled.fillRoundRect(27 + offset, 29, 16, 25, 7, SSD1306_WHITE);
    oled.fillRoundRect(85 + offset, 29, 16, 25, 7, SSD1306_WHITE);
  }
  oled.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED initialization failed");
    while (true) delay(1000);
  }
  // Servo power MUST come from external regulated 5V; share GND with ESP32.
  neck.setPeriodHertz(50);
  neck.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  neck.write(90);
  randomSeed(micros());
  uint32_t now = millis();
  nextBlink = now + 2000;
  nextLook = now + 1500;
  lastMotion = lastFrame = now;
  drawFace();
  Serial.println("Abao v0.3 ready; type angle 65..115 or a=90 in Serial Monitor");
}

void loop() {
  const uint32_t now = millis();
  // Serial input: a number followed by newline, e.g. 90
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.startsWith("a=")) command.remove(0, 2);
    bool valid = command.length() > 0;
    for (size_t i = 0; i < command.length(); ++i) {
      if (!isDigit(command[i])) { valid = false; break; }
    }
    if (valid) {
      int requested = command.toInt();
      if (requested >= ANGLE_MIN && requested <= ANGLE_MAX) {
        setTarget(requested);
        nextLook = now + 10000;
        Serial.printf("Target: %d degrees\n", requested);
      } else Serial.println("Out of range: 65..115");
    }
  }

  if (now >= nextLook) {
    setTarget(random(ANGLE_MIN, ANGLE_MAX + 1));
    nextLook = now + random(2500, 5500);
  }
  if (!blinking && now >= nextBlink) {
    blinking = true;
    blinkUntil = now + 130;
  }
  if (blinking && now >= blinkUntil) {
    blinking = false;
    nextBlink = now + random(2200, 6000);
  }
  if (now - lastMotion >= 20) {
    lastMotion = now;
    float delta = targetAngle - currentAngle;
    if (fabsf(delta) > 0.5f) {
      currentAngle += constrain(delta, -1.0f, 1.0f);
      neck.write((int)roundf(currentAngle));
    }
    eyeX += (targetEyeX - eyeX) * 0.18f;
  }
  if (now - lastFrame >= 50) {
    lastFrame = now;
    drawFace();
  }
}
