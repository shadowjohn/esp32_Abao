# Abao v0.3 — OLED eyes + SG90 neck

ESP32-WROOM-32 / NodeMCU 38-pin; 128×64 SSD1306 I²C OLED; one SG90 servo.

## Wiring

| Device | Pin | ESP32 |
|---|---|---|
| OLED | VDD | 3V3 |
| OLED | GND | GND |
| OLED | SDA | GPIO21 |
| OLED | SCK | GPIO22 |
| SG90 | Orange (signal) | GPIO18 |
| SG90 | Red (power) | **External regulated 5V** |
| SG90 | Brown (ground) | External GND **and ESP32 GND** |

**Do not power the SG90 from ESP32 3V3 or a GPIO.** Use a stable external 5V supply, preferably with 470–1000 µF decoupling near the servo. Do not connect a single 18650 directly to LM2596 expecting a 5V output: LM2596 is step-down only. Verify OLED power pin labels before wiring.

## Arduino libraries

- Adafruit SSD1306
- Adafruit GFX Library
- ESP32Servo (compatible with Arduino-ESP32 3.x)

Select **ESP32 Dev Module**, upload `abao_servo/abao_servo.ino`, then open Serial Monitor at **115200 baud** with newline enabled.

The robot looks left/right smoothly, blinks randomly, and moves the servo between 65° and 115°. Send `90` or `a=90` to center it temporarily. Begin testing with the servo horn **removed** to avoid mechanical collisions. The pulse limits are conservative starting values, not a guarantee for every servo.

## Next milestones

- v0.4 WS2812B emotional lighting
- v0.5 Wi-Fi / WebSocket commands
- Android phone sensing and 3waAIHub integration (event-driven, not continuous inference)

The existing `oled_hello/` example is retained unchanged.
