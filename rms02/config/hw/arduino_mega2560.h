// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#define HW_NAME "Arduino Mega 2560" // i SimulIDE

const uint8_t IN1 = 50; // S
const uint8_t IN2 = 53; // R
const uint8_t IN3 = 51; // L
const uint8_t IN4 = 52; // P

const uint8_t LEDdp = 22;
const uint8_t pinDispSerial = 14;

const uint8_t pinTONE = 12;

const uint8_t OUT3 = 39;
const uint8_t OUT4 = 40;

#define HW_OUT3
#define HW_OUT4

inline void initPins() {
  pinMode(LEDdp, OUTPUT);
  pinMode(pinTONE, OUTPUT);

  pinMode(IN1, INPUT_PULLUP);
  pinMode(IN2, INPUT_PULLUP);
  pinMode(IN3, INPUT_PULLUP);
  pinMode(IN4, INPUT_PULLUP);

  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
}
