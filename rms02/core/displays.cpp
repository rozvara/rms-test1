// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

void initSerialDisplays() {

  Serial1.begin(19200);    // oba externí na Serial1
  LEDDisplay.begin(19200); // LED display (interní)

  intDisp = {
    .target = &LEDDisplay,
    .addr = 0,
    .offset = 0,
    .len = 7,
    .colonPin = LEDdp
  };

  extDis1 = {
    .target = &Serial1,
    .addr = 0,
    .offset = 0,
    .len = 7,
    .colonPin = 0
  };

 extDis2 = {
    .target = &Serial1,
    .addr = 8,
    .offset = 0,
    .len = 7,
    .colonPin = 0
  };

  uint8_t buf[DATA_LEN];
  char charbuf[DATA_LEN];
  if (getEeprom(0, 0, buf)) { // pokud displeje nejsou defaultní, musí být i jejich layouty
    systemEcho(F("Nastaveni displeju"));
    extDis1.len = buf[0]; extDis1.addr = buf[1]; extDis1.offset = buf[2];
    extDis2.len = buf[3]; extDis2.addr = buf[4]; extDis2.offset = buf[5];
    if (getEeprom(0, 1, charbuf)) {
      memcpy(extDis1Layout, charbuf, extDis1.len);
      Serial.println(extDis1Layout);
    } else {
      systemEcho(F("Chyba Displej 1"));
    }

    if (getEeprom(0, 2, charbuf)) { // musí být i jejich layouty
      memcpy(extDis2Layout, charbuf, extDis1.len);
      Serial.println(extDis2Layout);
    } else {
      systemEcho(F("Chyba Displej 2"));
    }
  }
}

// rozloží časový interval na komponenty
void calcTimeParts(const uint32_t *start, const uint32_t *end) {
  uint32_t elapsed = *end - *start;
  uint32_t totalSeconds = elapsed / 1000UL;

  tp.milliseconds = elapsed % 1000UL;
  tp.seconds = totalSeconds % 60UL;
  tp.minutes = (totalSeconds / 60UL) % 60UL;
  tp.hours   = (totalSeconds / 3600UL);

  tp.hrThousands = tp.hours / 1000;
  tp.hrHundreds  = (tp.hours / 100) % 10;
  tp.hrTens      = (tp.hours / 10) % 10;
  tp.hrUnits     = tp.hours % 10;

  tp.minTens    = tp.minutes / 10;
  tp.minUnits   = tp.minutes % 10;
  tp.secTens    = tp.seconds / 10;
  tp.secUnits   = tp.seconds % 10;
  tp.msHundreds = tp.milliseconds / 100;
  tp.msTens     = (tp.milliseconds / 10) % 10;
  tp.msUnits    = tp.milliseconds % 10;
}

// převede komponenty času na znaky pro zobrazení
void tpToChars(char *buf) {
  if (tp.hours>0) {
    buf[0] = (tp.hrThousands>0) ? '0' + tp.hrThousands : ' ';
    buf[1] = (tp.hrHundreds>0) ? '0' + tp.hrHundreds : ' ';
    buf[2] = (tp.hrTens>0) ? '0' + tp.hrTens : ' ';
    buf[3] = '0' + tp.hrUnits;
    buf[4] = ':';
  } else {
    buf[0] = ' ';
    buf[1] = ' ';
    buf[2] = ' ';
    buf[3] = ' ';
    buf[4] = ' ';
  }
  buf[5] = (tp.minTens>0 || tp.hours>0) ? '0' + tp.minTens : ' ';
  buf[6] = '0' + tp.minUnits;
  buf[7] = (tp.msHundreds > 4) ? ' ' : ':'; // vstup do šablon bliká vždy
  buf[8]  = '0' + tp.secTens;
  buf[9]  = '0' + tp.secUnits;
  buf[10] = '.';
  buf[11] = '0' + tp.msHundreds;
  buf[12] = '0' + tp.msTens;
  buf[13] = '0' + tp.msUnits;
}

// posílají se jen změny, proto ochrana před výpadkem napájení
void updateSerialDisplays() {
  static bool everyOther; // jednou za dvě vteřiny pošle všechny znaky
  bool sendAll = false;
  if (timerOneSec) { sendAll = true; everyOther = !everyOther; } 
  intDisp.updateDisplay();
  extDis1.updateDisplay(sendAll && everyOther);
  extDis2.updateDisplay(sendAll && !everyOther);
}

void reverse(const char* src, char* dst, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    dst[i] = src[len - 1 - i];
  }
} 


// void testExternalDisplays() {
//   char testChars[16];
//   testChars[0] = ':';
//   testChars[1] = '1';
//   testChars[2] = '2';
//   testChars[3] = '3';
//   testChars[4] = '4';
//   testChars[5] = 'L';
//   extDis1.write(&testChars[0]);
//   testChars[0] = ' ';
//   testChars[1] = '9';
//   testChars[2] = '8';
//   testChars[3] = '7';
//   testChars[4] = '6';
//   testChars[5] = 'P';
//   extDis2.write(&testChars[0]);
// }
