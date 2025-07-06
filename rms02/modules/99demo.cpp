// RMS - firmware pro modulární stopky
// Modul "Demo"
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.
  
/*
Modul demonstruje:
- kontrolu přerušení na vstupech
- použití flagů časování
- výpočet času pro zobrazení a aktualizaci displejů
- chování dvojtečky podle stavu dráhy
- neblokující pípák
- výkonostní rozdíl při špatné optimalizaci

Rychlost loopu v jiných modulech můžeš ověřit zapnutím "performance overlay"
  (Nastavení -> perf 1)
*/


void moduleDemoLoop() {
  static bool notThisWay = false;

  switch (moduleState) {

    case MOD_EXITING:
      return;

    case MOD_SHUTDOWN:
      moduleEcho(F("Demo se louci"));
      changeModuleState(MOD_EXITING);
      return;

    case MOD_STARTING:
      lcdClear();
      lcdModuleTitle(F("Demo"));
      #ifdef LCD_20X4
        lcdWrite(4, 3, F("--> terminal"));
      #endif

      memcpy(intDisp.layout, intDispLayout, intDisp.len);
      memcpy(extDis1.layout, extDis1Layout, extDis1.len);
      memcpy(extDis2.layout, extDis2Layout, extDis2.len);

      leftTrack.startTime = 0;
      leftTrack.changeState(TRACK_READY);

      changeModuleState(MOD_IDLE);
      break;

    case MOD_IDLE:

      // vstupy kontrolujeme v každým loopu

      if (checkInput(&inStart)) { // v závodě se použije čas z inStart.time
        leftTrack.changeState(TRACK_RUNNING);
        while (exactMillis() % 1000 != 0);   // počkáme na 0ms
        leftTrack.startTime = exactMillis(); 
        resetTimer();                        // zarovnáme časovače a budeme pozorovat
        buzzWakeUp(3, 1500, 80, 920);
      }

      if (checkInput(&inReset)) { // Reset -> přepíná performance overlay
        showPerfOverlay = !showPerfOverlay;  // 1000000/počet je prům. rychlost v us
        lcdModuleTitle(F("Demo"));
        buzzer(INFO_BEEP);
      }

      if (checkInput(&inRight)) { // Pravá -> the Right Way - podle FPS
        notThisWay = false;
        buzzer(INFO_BEEP);
      }

      if (checkInput(&inLeft)) { // Levá -> v každém loopu všechno
        notThisWay = true;
        buzzer(WARN_BEEP);
      }

      // UI a displeje stačí aktualizovat jen před jejich vykreslením

      if (timerFPS || notThisWay) {
        uint32_t time = exactMillis();
        calcTimeParts(&leftTrack.startTime, &time);
        tpToChars(&timeStr[0]);
        #ifdef LCD_16X2
           enum { c = 2 };
        #else
           enum { c = 6 };
        #endif
        lcdWrite(c, 1, timeStr);
        if (leftTrack.state == TRACK_READY)
          lcdWrite(0, 1, F("Uptime"));
        else                // TRACK_RUNNING
          lcdWrite(0, 1, F("Demo  "));
        extDis1.applyLayout(&timeStr[0], leftTrack.colon);
        extDis2.applyLayout(&timeStr[0], leftTrack.colon);
        intDisp.applyLayout(&timeStr[0], leftTrack.colon);
      }

      if (timerFPS) { // NOTE: běží záměrně tak, aby nebyl na celé vteřiny
        Serial.print(F(".")); // při 27x37 je offset 1ms za vteřinu
      }
      if (timerOneSec) {
        Serial.print(F("| "));
        Serial.println(exactMillis());
      } else if (timerHalfSec) {
        Serial.print(F(":"));
      }
      break;
  }
}
