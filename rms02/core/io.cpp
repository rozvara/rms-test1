// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#include <PinChangeInterrupt.h>

void handlePinInterrupt(InputPin *input, uint32_t *int_ms, uint16_t *int_us) {
  uint32_t new_ms;
  uint16_t new_us;
  uint8_t sreg = SREG;
  cli();
  new_ms = *int_ms;
  new_us = *int_us;
  SREG = sreg;
  if (new_ms - input->time >= input->debounce) {
    input->time = new_ms;
    input->us   = new_us;
    input->triggered = true;
    systemEcho(F("Pulz "), false); Serial.print(input->name);
    if (input == &inReset && moduleState == MOD_IDLE) Serial.write('+');
    Serial.print(F(" @ ")); Serial.print(new_ms); Serial.write('.');
    Serial.write('0' + (new_us / 100));
    Serial.write('0' + (new_us / 10 % 10));
    Serial.write('0' + (new_us % 10));
    Serial.println(F(" ms"));
  }
}


bool checkInput(InputPin *input) {
  if (input->triggered) {
    input->triggered = false;
    return true;
  } else {
    return false;
  }
}


void ISR_inStart() {
  uint32_t ms;
  uint16_t t;

  uint8_t sreg = SREG;
  cli();
  ms = _exact_millis;
  t = TCNT1;
  SREG = sreg;

  _time_inStart = ms;
  _us_inStart = t*4;
  _isr_inStart  = true;
}


void ISR_inReset() {
  uint32_t ms;
  uint16_t t;

  uint8_t sreg = SREG;
  cli();
  ms = _exact_millis;
  t = TCNT1;
  SREG = sreg;

  _time_inReset = ms;
  _us_inReset = t*4;
  _isr_inReset  = true;
}


void ISR_inLeft() {
  uint32_t ms;
  uint16_t t;

  uint8_t sreg = SREG;
  cli();
  ms = _exact_millis;
  t = TCNT1;
  SREG = sreg;

  _time_inLeft = ms;
  _us_inLeft = t*4;
  _isr_inLeft  = true;
}


void ISR_inRight() {
  uint32_t ms;
  uint16_t t;

  uint8_t sreg = SREG;
  cli();
  ms = _exact_millis;
  t = TCNT1;
  SREG = sreg;

  _time_inRight = ms;
  _us_inRight = t*4;
  _isr_inRight  = true;
}


void attachPinInterrupts() {
  attachPCINT(digitalPinToPCINT(IN1), ISR_inStart, FALLING);
  attachPCINT(digitalPinToPCINT(IN2), ISR_inReset, FALLING);
  attachPCINT(digitalPinToPCINT(IN3), ISR_inLeft,  FALLING);
  attachPCINT(digitalPinToPCINT(IN4), ISR_inRight, FALLING);
}


void dettachPinInterrupts() {
  detachPCINT(digitalPinToPCINT(IN1));
  detachPCINT(digitalPinToPCINT(IN2));
  detachPCINT(digitalPinToPCINT(IN3));
  detachPCINT(digitalPinToPCINT(IN4));
}


void turnOnLeftSignal() {
  #ifdef HW_OUT3
    digitalWrite(OUT3, HIGH);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOnRightSignal() {
  #ifdef HW_OUT4
    digitalWrite(OUT4, HIGH);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOffLeftSignal() {
  #ifdef HW_OUT3
    digitalWrite(OUT3, LOW);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOffRightSignal() {
  #ifdef HW_OUT4
    digitalWrite(OUT4, LOW);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOffSignals() {
	turnOffLeftSignal();
	turnOffRightSignal();
}
