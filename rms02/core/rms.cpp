// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

void changeModuleState(ModuleState newState) {
  moduleState = newState;
  firstEntry = true;
  delayedFirstEntry = false; // stav si musí sám nastavit v entry init
  entryTime = exactMillis();
}


ISR(TIMER1_COMPA_vect) {
  _exact_millis++;
  if (++_fps_ms >= 37)  { _fps_flag = true; _fps_ms = 0; }
  if (++_tim_ms >= 250) { _tim_flag = true; _tim_ms = 0; }
  TCNT1 = 0;
}


uint32_t exactMillis() {
  uint32_t ms;
  uint8_t oldSREG = SREG;
  cli();
  ms = _exact_millis;
  SREG = oldSREG;
  return ms;
}


// 0–999 µs od poslední millis
uint16_t exactMicros() {
  uint16_t t;
  uint8_t oldSREG = SREG;
  cli();
  t = TCNT1;
  SREG = oldSREG;
  return t * 4;
}


// sync tikání fps
void resetTimer() {
  uint8_t sreg = SREG;
  cli();
  _fps_ms = 0;
  _tim_ms = 0;
  _fps_flag = false;
  _tim_flag = false;
  _tim_cnt = 0;
  SREG = sreg;
}



void performanceOverlay() {
  uint32_t lp = loops * 2UL;
  char perf[10];
  perf[0] = '[';
  perf[1] = '0'+moduleState;
  perf[2] = ':';
  perf[3] = '0' + (lp / 10000)  % 10;
  perf[4] = '0' + (lp / 1000)   % 10;
  perf[5] = '0' + (lp / 100)    % 10;
  perf[6] = '0' + (lp / 10)     % 10;
  perf[7] = '0' + (lp)          % 10;
  perf[8] = ']';
  perf[9] = '\0';
  lcdWrite(0, 0, perf);
}


void updateBuzzer() {
  uint32_t now = exactMillis();
  if (!buzzState) {
    if (now - buzzTimer >= buzzOffTime) {
      tone(pinTONE, buzzFreq);
      buzzState = true;
      buzzTimer = now;
    }
  } else {
    if (now - buzzTimer >= buzzOnTime) {
      noTone(pinTONE);
      buzzState = false;
      buzzTimer = now;
      buzzCount--;
    }
  }
}


void buzzWakeUp(uint8_t count, uint16_t freq, uint16_t duration, uint16_t pause = 0) {
  buzzCount = count;
  buzzFreq = freq;
  buzzOnTime = duration;
  buzzOffTime = pause;
  buzzTimer = exactMillis();
  buzzState = false;
  noTone(pinTONE);
}


void buzzer(uint8_t type) {
  switch (type) {
    case INFO_BEEP:
      buzzWakeUp(1, 2500, 80);
      break;
    case WARN_BEEP:
      buzzWakeUp(3, 2000, 80, 100); // krátký (volání á vteřina)
      break;
    case READY_BEEP:
      buzzWakeUp(1, 200, 2000); // TODO nastavení
      break;
    // case LONG_BEEP:
    //   buzzWakeUp(1, 1500, 2500);
    //   break;
  }
}



// ===== EEPROM
uint16_t crc16(const uint8_t* data, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= ((uint16_t)data[i]) << 8;
    for (uint8_t j = 0; j < 8; j++)
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
  }
  return crc;
}


bool validCrc(const Record& r) {
  uint16_t crc = crc16((uint8_t*)&r, 2 + DATA_LEN);
  return (crc == r.crc);
}


void readBlock(uint8_t index, Record& out) {
  uint16_t addr = index * BLOCK_SIZE;
  for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
    ((uint8_t*)&out)[i] = EEPROM.read(addr + i);
  }
}


// bool getEeprom(uint8_t idModul, uint8_t idKey, uint8_t* buf) {
//   for (int8_t i = BLOCK_COUNT - 1; i >= 0; i--) {
//     Record r;
//     readBlock(i, r);
//     if (!validCrc(r)) continue;
//     if ((r.idModul & 0x7F) != idModul) continue;
//     if ((r.idKey   & 0x7F) != idKey) continue;
//     memcpy(buf, r.data, DATA_LEN);
//     return true;
//   }
//   return false;
// }

bool getEeprom(uint8_t idModul, uint8_t idKey, uint8_t* buf) {
  bool found = false;
  uint8_t i = 0;
  while (!found && i < BLOCK_COUNT-1) {
    Record r;
    readBlock(i, r);
    if (validCrc(r) && (r.idModul == idModul) && (r.idKey & 0x7F) == idKey) {
      memcpy(buf, r.data, DATA_LEN);
      found = true;
    }
    i++;
  }
  return found;
}

bool getEeprom(uint8_t idModul, uint8_t idKey, char* buf) {
  return getEeprom(idModul, idKey, (uint8_t*)buf);
}
