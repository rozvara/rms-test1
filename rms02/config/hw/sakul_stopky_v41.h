// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

// převzato od sakul.cz z free verze firmware pro Stopky v4.1

#define HW_NAME "Sakul Stopky v4.1"

const uint8_t IN1 = 8;  // Tlačítko (Vstup) START
const uint8_t IN2 = 9;  // Tlačítko (Vstup) RESET
const uint8_t IN3 = 10; // Tlačítko (Vstup) Levá Dráha
const uint8_t IN4 = 11; // Tlačítko (Vstup) Pravá Dráha

const uint8_t LEDdp = A0;
const uint8_t pinDispSerial = 6; // Tx pro interní sériový displej

const uint8_t pinTONE = 12; // Definice výstupu pro pípák


inline void initPins() {
  pinMode(LEDdp, OUTPUT);
  pinMode(pinTONE, OUTPUT);

  pinMode(IN1, INPUT_PULLUP);
  pinMode(IN2, INPUT_PULLUP);
  pinMode(IN3, INPUT_PULLUP);
  pinMode(IN4, INPUT_PULLUP);
}
