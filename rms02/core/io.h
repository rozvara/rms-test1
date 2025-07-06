// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

struct InputPin {
  bool triggered;
  uint32_t time;
  uint16_t us;
  uint8_t debounce;
  char name;
};
InputPin inStart = { false, 0, 0, INPUT_DEBOUNCE, 'S' };
InputPin inLeft  = { false, 0, 0, INPUT_DEBOUNCE, 'L' };
InputPin inRight = { false, 0, 0, INPUT_DEBOUNCE, 'P' };
InputPin inReset = { false, 0, 0, INPUT_DEBOUNCE, 'R' };

// === pro modul
bool checkInput(InputPin *input);

void turnOnLeftSignal();
void turnOnRightSignal();
void turnOffLeftSignal();
void turnOffRightSignal();
void turnOffSignals();


// === pro jádro
volatile bool _isr_inStart;
volatile uint32_t _time_inStart;
volatile uint16_t _us_inStart;

volatile bool _isr_inReset;
volatile uint32_t _time_inReset;
volatile uint16_t _us_inReset;

volatile bool _isr_inLeft;
volatile uint32_t _time_inLeft;
volatile uint16_t _us_inLeft;

volatile bool _isr_inRight;
volatile uint32_t _time_inRight;
volatile uint16_t _us_inRight;


void ISR_inStart();
void ISR_inReset();
void ISR_inLeft();
void ISR_inRight();

void handlePinInterrupt(InputPin *input, uint32_t *ptr);

void attachPinInterrupts();
void dettachPinInterrupts();
