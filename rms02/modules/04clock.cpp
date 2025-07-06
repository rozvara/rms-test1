// RMS - firmware pro modulární stopky
// Modul "RTC Hodiny"
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

  
#ifndef HW_RTC_DS3231
  #error "Modul Hodiny vyžaduje RTC"
#endif

void module04Loop() {

  switch (moduleState) {

    case MOD_EXITING:
      return;

    case MOD_SHUTDOWN:
      moduleEcho(F("Konec"));
      changeModuleState(MOD_EXITING);
      return;

    case MOD_STARTING:
      lcdClear();
      extDis1.fill(' ');
      extDis2.fill(' ');
      changeModuleState(MOD_IDLE);
      break;

    case MOD_IDLE:
      if (timerOneSec) {
        const char dayOfWeek[7][3] = {"Ne", "Po", "Ut", "St", "Ct", "Pa", "So"};
        char charbuf[15];
        char reversed[7];

        DateTime rtcNow = rtc.now();

        // IMPR: blikat dvojtečkou (timerHalfSec)?

        #ifdef LCD_16X2
          enum { r = 0 };
        #else
          enum { r = 1 };
        #endif
        sprintf(charbuf, "%2d:%02d:%02d", rtcNow.hour(), rtcNow.minute(), rtcNow.second());
        enum { c0 = (LCD_COLS - 8) / 2 };
        lcdWrite(c0, 0+r, charbuf);
        sprintf(charbuf, "%s  %2d.%2d.%04d", dayOfWeek[rtcNow.dayOfTheWeek()], rtcNow.day(), rtcNow.month(), rtcNow.year());
        enum { c1 = (LCD_COLS - 14) / 2 };
        lcdWrite(c1, 1+r, charbuf);

        sprintf(charbuf, "%2d%02d%02d:", rtcNow.hour(), rtcNow.minute(), rtcNow.second());
        reverse(&charbuf[0], &reversed[0], 7);
        intDisp.write(reversed);

        sprintf(charbuf, "  %2d%02d:", rtcNow.hour(), rtcNow.minute());
        reverse(&charbuf[0], &reversed[0], 7);
        extDis1.write(reversed);
      }
      break;
  }
}
