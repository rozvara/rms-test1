// RMS - firmware pro modulární stopky
// Modul "Hasičský útok" (závod ve dvou drahách s odpočtem času na přípravu)
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.


void m01UpdateRaceDisplays() {
  bool notUpdated = true;

  uint32_t currTime = exactMillis();
  calcTimeParts(&rightTrack.startTime, &currTime);
  tpToChars(&timeStr[0]);

  #ifdef LCD_16X2
    enum { c = 2, r = 0 };
  #else
    enum { c = 6, r = 2 };
  #endif

  if (rightTrack.state == TRACK_RUNNING) {
    extDis2.applyLayout(&timeStr[0], rightTrack.colon);
    intDisp.applyLayout(&timeStr[0], rightTrack.colon);
    lcdWrite(c, r+1, timeStr);
    notUpdated = false;
  }

  if (leftTrack.state == TRACK_RUNNING) {
    extDis1.applyLayout(&timeStr[0], leftTrack.colon);
    if (notUpdated) {
      intDisp.applyLayout(&timeStr[0], leftTrack.colon);
      lcdWrite(c, r, timeStr);
    }
  }
}


// NOTE: Pravá - celkový čas, Levá - zbytkový čas
void m01UpdateCountdownDisplays() {
  if (rightTrack.state == TRACK_READY) {
    calcTimeParts(&leftTrack.startTime, &leftTrack.finishTime);
  } else { // odpočítávaný čas
    calcTimeParts(&rightTrack.startTime, &rightTrack.finishTime);
  }
  tpToChars(&timeStr[0]);
  if (rightTrack.state == TRACK_RUNNING) {
    #ifdef LCD_16X2
      enum { c = 2+4, r = 1 };
    #else
      enum { c = 6+4, r = 3 };
    #endif
    lcdWrite(c, r, timeStr);
  }
  extDis1.fill(' ');
  extDis2.applyLayout(&timeStr[0], rightTrack.colon);
  intDisp.applyLayout(&timeStr[0], rightTrack.colon);
}


void m01LeftFinished() {
  leftTrack.finishTime = inLeft.time;
  leftTrack.changeState(TRACK_STOPPED);

  logEvent(LOG_FINISH_LEFT, leftTrack.finishTime);
  turnOnLeftSignal();

  calcTimeParts(&leftTrack.startTime, &leftTrack.finishTime);
  tpToChars(&timeStr[0]);
  extDis1.applyLayout(&timeStr[0], leftTrack.colon);
  #ifdef LCD_16X2
    enum { c = 2, r = 0 };
  #else
    enum { c = 6, r = 2 };
  #endif
  timeStr[7] = ':';
  lcdWrite(c, r, timeStr);
  lcdWrite(0, r, F("Leva"));

  moduleEcho(F("Leva draha"), false); Serial.println(timeStr);
}


void m01RightFinished() {
  rightTrack.finishTime = inRight.time;
  rightTrack.changeState(TRACK_STOPPED);

  logEvent(LOG_FINISH_RIGHT, rightTrack.finishTime);
  turnOnRightSignal();

  calcTimeParts(&rightTrack.startTime, &rightTrack.finishTime);
  tpToChars(&timeStr[0]);
  extDis2.applyLayout(&timeStr[0], rightTrack.colon);
  #ifdef LCD_16X2
    enum { c = 2, r = 1 };
  #else
    enum { c = 6, r = 3 };
  #endif
  timeStr[7] = ':';
  lcdWrite(c, r, timeStr);
  lcdWrite(0, r, F("Prava"));

  moduleEcho(F("Prava draha"), false); Serial.println(timeStr);
}


void m01RaceFinished() {
  #ifdef LCD_20X4
    lcdStateTitle(F("Zavod skoncil"));
  #endif

  if (rightTrack.finishTime > leftTrack.finishTime) {
    calcTimeParts(&rightTrack.startTime, &rightTrack.finishTime);
  } else {
    calcTimeParts(&leftTrack.startTime, &leftTrack.finishTime);
  }
  tpToChars(&timeStr[0]);
  intDisp.applyLayout(&timeStr[0], leftTrack.colon);

  changeModuleState(MOD_STOPPED);
}


void module01Loop() {
  static uint32_t m01CountdownTime = 1000UL* (5*60UL + 0UL); // default: 5m 0s
  static uint32_t m01DisplayDelay = 2500;                    // default: 2,5 s

  switch (moduleState) {

    case MOD_EXITING:
      return;

    case MOD_SHUTDOWN:
      moduleEcho(F("Konec"));
      changeModuleState(MOD_EXITING);
      turnOffSignals();
      return;

    case MOD_STARTING:
      intDisp.fill(' ');
      extDis1.fill(' ');
      extDis2.fill(' ');
      turnOffSignals();

      uint8_t buf[DATA_LEN];
      if (getEeprom(MODULE_ID_01, 0, buf)) {
        m01CountdownTime = 1000UL* (buf[0]*3600UL + buf[1]*60UL + buf[2]); // h,m,s
        m01DisplayDelay = buf[3]*1000UL + buf[4]*100UL; // s, ds
      }

      leftTrack.changeState(TRACK_OFF);
      rightTrack.changeState(TRACK_OFF);

      changeModuleState(MOD_IDLE);
      break;

    case MOD_IDLE:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdModuleTitle(F("STOPKY HU"));
        #ifdef LCD_16X2
          lcdWrite(0, 1, F("R>Fce  R+L/P>Mod"));
        #else
          lcdWrite(3, 2, F("Reset > Funkce"));
          lcdWrite(1, 3, F("Reset+L/P > Moduly"));
        #endif
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_PREPARE);
      }
      break;

    case MOD_PREPARE:
      if (firstEntry) {
        firstEntry = false;
        delayedFirstEntry = true;
        lcdClear();
        lcdStateTitle(F("ODPOCET"));
        #ifdef LCD_16X2
          lcdWrite(0, 1, F("S>Start  R>Zavod"));
        #else
          lcdWrite(0, 2, F("Start> Zahaji odpoc."));
          lcdWrite(0, 3, F("Reset> Mereni zavodu"));
        #endif

        leftTrack.changeState(TRACK_READY);
        leftTrack.startTime = 0;
        leftTrack.finishTime = m01CountdownTime; //délka odpočtu v L

        rightTrack.changeState(TRACK_READY);

        // TODO layout odpočtu pro Dis2 do eeprom nastavení
        char newLayout[7] = {'c','d','f','g','i','j',':'};
        memcpy(extDis2.layout, newLayout, 7);
        memcpy(intDisp.layout , newLayout, 7);
      }
      if (delayedFirstEntry) {
        if (exactMillis() > m01DisplayDelay + entryTime) {
          delayedFirstEntry = false;
          m01UpdateCountdownDisplays();
        }
      }
      if (checkInput(&inStart)) {
        changeModuleState(MOD_COUNTDOWN);
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_READY);
      }
      break;

    case MOD_READY:
      if (firstEntry) {
        firstEntry = false;
        delayedFirstEntry = true;
        lcdClear();
        lcdStateTitle(F("ZAVOD"));
        #ifdef LCD_16X2
          lcdWrite(0, 1, F("S>Start   R>Zpet"));
        #else
          lcdWrite(0, 2, F("Start > Zahaji zavod"));
          lcdWrite(3, 3, F("Reset > Zpatky"));
        #endif

        checkInput(&inStart); // zahazujeme současné stisky S+R

        leftTrack.startTime = 0;
        leftTrack.finishTime = 0;
        rightTrack.startTime = 0;
        rightTrack.finishTime = 0;

        leftTrack.changeState(TRACK_READY);
        rightTrack.changeState(TRACK_READY);
        calcTimeParts(&leftTrack.startTime, &leftTrack.finishTime);
        tpToChars(&timeStr[0]);

        memcpy(intDisp.layout, intDispLayout, intDisp.len);
        memcpy(extDis1.layout, extDis1Layout, extDis1.len);
        memcpy(extDis2.layout, extDis2Layout, extDis2.len);
      }
      if (delayedFirstEntry) {
        if (exactMillis() > m01DisplayDelay + entryTime) {
          delayedFirstEntry = false;
          intDisp.applyLayout(&timeStr[0], leftTrack.colon);
          extDis1.applyLayout(&timeStr[0], leftTrack.colon);
          extDis2.applyLayout(&timeStr[0], leftTrack.colon);
        }
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_IDLE);
        return;
      }
      if (checkInput(&inLeft) || checkInput(&inRight) || digitalRead(IN3) == LOW || digitalRead(IN4) == LOW) {
        changeModuleState(MOD_ERROR);
        return;
      }
      if (checkInput(&inStart)) {
        resetTimer();
        moduleEcho(F("Zavod zahajen"));

        logClear();
        logEvent(LOG_START, inStart.time);

        leftTrack.startTime = inStart.time;
        rightTrack.startTime = inStart.time;

        leftTrack.changeState(TRACK_RUNNING);
        rightTrack.changeState(TRACK_RUNNING);

        changeModuleState(MOD_RUNNING);
      }
      break;

    case MOD_ERROR:
      if (firstEntry) {
        firstEntry = false;
        moduleEcho(F("CHYBA koncaku"));
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_IDLE);
        return;
      }
      if (digitalRead(IN3) == LOW) {
        leftTrack.changeState(TRACK_ERROR);
      } else {
        leftTrack.changeState(TRACK_READY);
      }
      if (digitalRead(IN4) == LOW) {
        rightTrack.changeState(TRACK_ERROR);
      } else {
        rightTrack.changeState(TRACK_READY);
      }
      if (leftTrack.state == TRACK_ERROR || rightTrack.state == TRACK_ERROR) {
        if (timerOneSec) buzzer(WARN_BEEP);
        #ifdef LCD_16X2
          lcdClearRow(1);
        #else
          lcdClearRow(2);
        #endif
      }
      if (leftTrack.state == TRACK_ERROR) {
        #ifdef LCD_16X2
          enum { r = 1 };
        #else
          enum { r = 2 };
        #endif
        lcdWrite(0, r, F("L:CHYBA"));
        extDis1.write(F(" -E- "));
        intDisp.write(F(" -E- "));
      } else {
        extDis1.fill(' ');
      }
      if (rightTrack.state == TRACK_ERROR) {
        #ifdef LCD_16X2
          enum { c = 9, r = 1 };
        #else
          enum { c = 13, r = 2 };
        #endif
        lcdWrite(c, r, F("P:CHYBA"));
        extDis2.write(F(" -E- "));
        intDisp.write(F(" -E- "));
      } else {
        extDis2.fill(' ');
      }
      if (leftTrack.state == TRACK_READY && rightTrack.state == TRACK_READY) {
        moduleEcho(F("Koncaky OK"));
        intDisp.fill(' ');
        checkInput(&inLeft);  // NOTE:
        checkInput(&inRight); // po zotavení z chyby zahoď všechny flagy
        checkInput(&inStart);
        changeModuleState(MOD_READY);
      }
      break;

    case MOD_RUNNING:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdStateTitle(F("Zavod bezi"));
      }
      // levý v cíli?
      if (leftTrack.state == TRACK_RUNNING) {
        if (checkInput(&inLeft)) m01LeftFinished();
      }
      // pravý v cíli?
      if (rightTrack.state == TRACK_RUNNING) {
        if (checkInput(&inRight)) m01RightFinished();
      }
      // konec?
      if  ((leftTrack.state == TRACK_STOPPED) && 
          (rightTrack.state == TRACK_STOPPED)) {
        m01RaceFinished();
      } else {
        // stále se závodí...
        if (timerFPS) m01UpdateRaceDisplays();
        // mezičas?
        if (checkInput(&inStart)) {
          moduleEcho(F("Mezicas"));
          logEvent(LOG_SPLIT, inStart.time);
          #ifdef LCD_20X4
            calcTimeParts(&rightTrack.startTime, &inStart.time);
            tpToChars(&timeStr[0]);
            timeStr[7] = ':';
            lcdWrite(6, 1, timeStr);
            lcdWrite(0, 1, "Mezicas");
          #endif
        }
        // reset?
        if (checkInput(&inReset)) {
          moduleEcho(F("Mereni zruseno"));
          logEvent(LOG_RESET, inReset.time);

          lcdStateTitle(F("Zavod zrusen"));

          leftTrack.changeState(TRACK_STOPPED);
          rightTrack.changeState(TRACK_STOPPED);
          changeModuleState(MOD_STOPPED);
        }
      }
      break;

    case MOD_STOPPED:
      if (firstEntry) {
        firstEntry = false;
        moduleEcho(F("Zadost o vypis"));
        logDump(Serial);
        // TODO SD karta
      }
      if (checkInput(&inReset)) {
        // TODO zjistit požadované chování (kdy vypínat)
        turnOffSignals();
        changeModuleState(MOD_IDLE);
      }
      break;
  
    case MOD_COUNTDOWN:
      if (firstEntry) {
        resetTimer();

        firstEntry = false;
        moduleEcho(F("Zahajen odpocet"));
        lcdClear();
        lcdStateTitle(F("Odpocet bezi"));

        rightTrack.changeState(TRACK_RUNNING);
        rightTrack.startTime = exactMillis();
        rightTrack.finishTime = rightTrack.startTime;
        rightTrack.finishTime += leftTrack.finishTime;
      }
      if (checkInput(&inReset)) {
        moduleEcho(F("Zruseni odpoctu"));
        buzzer(INFO_BEEP);
        changeModuleState(MOD_READY);
      } else {
        // běží odpočet
        if (timerFPS) {
          rightTrack.startTime = exactMillis();
          if (rightTrack.startTime <= rightTrack.finishTime) {
            m01UpdateCountdownDisplays();
          } else {
            moduleEcho(F("Odpocet skoncil"));
            changeModuleState(MOD_READY);
            buzzer(READY_BEEP);
          }
        }
      }
      break;
  }
}
