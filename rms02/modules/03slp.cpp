// RMS - firmware pro modulární stopky
// Modul "Stopky 3: jedna dráha, ukončení L nebo P"
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.


void mod03UpdateDisplays(bool colon = false) {
  tpToChars(&timeStr[0]);
  #ifdef LCD_16X2
    enum { c = 2, r = 1 };
  #else
    enum { c = 6, r = 3 };
  #endif
  if (colon) timeStr[7] = ':';
  lcdWrite(c, r, timeStr);
  intDisp.applyLayout(&timeStr[0], rightTrack.colon);
  extDis2.applyLayout(&timeStr[0], rightTrack.colon);
}


void mod03Finished() {
  rightTrack.changeState(TRACK_STOPPED);
  calcTimeParts(&rightTrack.startTime, &rightTrack.finishTime);
  mod03UpdateDisplays(true);
  lcdStateTitle(F("Ukonceno"));
  changeModuleState(MOD_STOPPED);
}


void module03Loop() {

  switch (moduleState) {

    case MOD_EXITING:
      return;

    case MOD_SHUTDOWN:
      moduleEcho(F("Konec"));
      changeModuleState(MOD_EXITING);
      return;

    case MOD_STARTING:
      intDisp.fill(' ');
      extDis1.fill(' ');
      extDis2.fill(' ');
      turnOffSignals();

      // TODO zjistit, zda nemá raději běžet v prvním displeji
      memcpy(intDisp.layout, intDispLayout, intDisp.len);
      memcpy(extDis2.layout, extDis2Layout, extDis2.len);

      leftTrack.changeState(TRACK_OFF);
      rightTrack.changeState(TRACK_OFF);
      
      changeModuleState(MOD_IDLE);
      break;

    case MOD_IDLE:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdModuleTitle(F("STOPKY S-LP"));
        #ifdef LCD_16X2
          lcdWrite(0, 1, F("R>Fce  R+L/P>Mod"));
        #else
          lcdWrite(3, 2, F("Reset > Funkce"));
          lcdWrite(1, 3, F("Reset+L/P > Moduly"));
        #endif
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_READY);
      }
      break;

    case MOD_READY:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdStateTitle(F("STOPKY"));
        #ifdef LCD_16X2
          lcdWrite(0, 1, F("S>Start   R>Zpet"));
        #else
          lcdWrite(0, 2, F("Start > Zahaji zavod"));
          lcdWrite(3, 3, F("Reset > Zpatky"));
        #endif

        rightTrack.startTime = 0;
        rightTrack.finishTime = 0;

        checkInput(&inStart); // zahazujeme současné stisky S+R

        rightTrack.changeState(TRACK_READY);
        calcTimeParts(&rightTrack.startTime, &rightTrack.finishTime);
        tpToChars(&timeStr[0]);

        intDisp.applyLayout(&timeStr[0], rightTrack.colon);
        extDis2.applyLayout(&timeStr[0], rightTrack.colon);
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_IDLE);
        return;
      }
      checkInput(&inLeft); // NOTE: ignorujeme "současný"
      checkInput(&inRight); // stisk Start+Konec
      if (checkInput(&inStart)) {
        resetTimer();
        moduleEcho(F("Stopky spusteny"));
        logClear();
        logEvent(LOG_START, inStart.time);
        rightTrack.startTime = inStart.time;
        rightTrack.changeState(TRACK_RUNNING);
        changeModuleState(MOD_RUNNING);
      }
      break;

    case MOD_RUNNING:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdStateTitle(F("Stopky bezi"));
        #ifdef LCD_20X4
          lcdWrite(0, 1, F("S>Mezicas  L/P>Konec"));
        #endif
      }
      if (checkInput(&inLeft)) {
        rightTrack.finishTime = inLeft.time;
        logEvent(LOG_FINISH_LEFT, rightTrack.finishTime);
        mod03Finished();
        return;
      }  
      if (checkInput(&inRight)) {
        rightTrack.finishTime = inRight.time;
        logEvent(LOG_FINISH_RIGHT, rightTrack.finishTime);
        mod03Finished();
        return;
      }
      if (checkInput(&inStart)) {
        moduleEcho(F("Mezicas"));
        logEvent(LOG_SPLIT, inStart.time);
        calcTimeParts(&rightTrack.startTime, &inStart.time);
        tpToChars(&timeStr[0]);
        timeStr[7] = ':';
        #ifdef LCD_16X2
          enum { c = 2, r = 0 };
          lcdWrite(c, r, timeStr);
          lcdWrite(0, r, "M-cas");
        #else
          enum { c = 6, r = 2 };
          lcdWrite(c, r, timeStr);
          lcdWrite(0, r, "Mezicas");
        #endif
      }
      if (timerFPS) {
        uint32_t currTime = exactMillis();
        calcTimeParts(&rightTrack.startTime, &currTime);
        mod03UpdateDisplays();
      }
      if (checkInput(&inReset)) {
        moduleEcho(F("Mereni zruseno"));
        logEvent(LOG_RESET, inReset.time);
        calcTimeParts(&rightTrack.startTime, &inReset.time);
        mod03UpdateDisplays(true);

        lcdStateTitle(F("Zruseno"));

        rightTrack.changeState(TRACK_STOPPED);
        changeModuleState(MOD_STOPPED);
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
        changeModuleState(MOD_IDLE);
      }
      break;
  
  }
}
