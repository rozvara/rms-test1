// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

void setup() {
  initTimer();
  Serial.begin(115200);
  #if defined(HW_VER_41) || defined(TARGET_MCU_LEONARDO)
    while (!Serial && exactMillis() < 2000); // čekej na USB CDC max 2s
  #endif
  systemEcho(F("Zapnuti "), false);
  Serial.print(FW_VERSION); Serial.println(FW_TAG);

  initLCD();
  initPins();
  initSerialDisplays();

  #ifdef HW_RTC_DS3231
    if (!rtc.begin()) systemEcho(F("RTC nereaguje"));
  #endif

  modulesInstalled = sizeof(modules) / sizeof(modules[0]);

  // TODO načíst výchozí z eeprom
  swapModule(0);
}


void loop() {
  timerFPS = false; timerHalfSec = false; timerOneSec = false;
  if (_fps_flag) { _fps_flag = false; timerFPS = true; }
  if (_tim_flag) { _tim_flag = false;
    if (++_tim_cnt == 2) { timerHalfSec = true; }
    if (  _tim_cnt == 4) { timerHalfSec = true; timerOneSec = true; _tim_cnt = 0; }
  }

  if (_isr_inStart) { _isr_inStart = false;  handlePinInterrupt(&inStart, &_time_inStart, &_us_inStart); }
  if (_isr_inReset) { _isr_inReset = false;  handlePinInterrupt(&inReset, &_time_inReset, &_us_inReset); }
  if (_isr_inLeft ) { _isr_inLeft  = false;  handlePinInterrupt(&inLeft,  &_time_inLeft,  &_us_inLeft ); }
  if (_isr_inRight) { _isr_inRight = false;  handlePinInterrupt(&inRight, &_time_inRight, &_us_inRight); }

  if (inReset.triggered && moduleState == MOD_IDLE) handleModuleSwap();

  moduleLoop();
  loops++;

  if (showPerfOverlay && timerHalfSec) { performanceOverlay(); loops = 0; }
  if (timerFPS) { updateLCD(); updateSerialDisplays(); }
  if (buzzCount > 0) updateBuzzer();
}


void handleModuleSwap() {
  delay(INPUT_DEBOUNCE); // R+ se zobrazuje, když je na chvíli blokující
  if (digitalRead(IN2) == LOW) {  // R
    bool changeInProgress = false;
    uint8_t futureModule = currentModule;
    dettachPinInterrupts();
    while (digitalRead(IN2) == LOW) {
      if (digitalRead(IN4) == LOW) { // P
        delay(INPUT_DEBOUNCE);
        futureModule = (futureModule == modulesInstalled - 1) ? 0 : futureModule + 1;
        changeInProgress = true;
      }
      if (digitalRead(IN3) == LOW) { // L
        delay(INPUT_DEBOUNCE);
        futureModule = (futureModule == 0) ? modulesInstalled - 1 : futureModule - 1;
        changeInProgress = true;
      }
      if (changeInProgress) {
        lcdClear();
        #ifdef LCD_16X2
          enum { c = 0 };
        #else
          enum { c = 2 };
        #endif
        lcdWrite(c, 0, F("Prepnuti modulu"));
        lcdWrite(c, 1, F("-->"));
        lcdWrite(c+4, 1, modules[futureModule].name);
        #ifdef LCD_20X4
          lcdWrite(0, 3, F("Zmena: uvolnit Reset"));
        #endif
        updateLCD();
      }
    }
    if (changeInProgress) swapModule(futureModule);
    else attachPinInterrupts();
  }
}


void swapModule(uint8_t newModule) {
  inStart.triggered = false;
  inReset.triggered = false; 
  inLeft.triggered  = false; 
  inRight.triggered = false;

  systemEcho(F("Zmena modulu"));
  if (moduleLoop != nullptr) {
    moduleState = MOD_SHUTDOWN;
    uint32_t now = exactMillis();
    while ((exactMillis() < now+50) && moduleState != MOD_EXITING) moduleLoop();
  }

  attachPinInterrupts();
  moduleState = MOD_STARTING;
  currentModule = newModule;
  moduleLoop = modules[currentModule].loop;

  systemEcho(F("Modul '"), false);
  Serial.print(modules[currentModule].name);
  Serial.println(F("' aktivni"));
}


void terminalEcho(const String& who, const String& msg, bool lf = true) {
  Serial.print(who);
  Serial.print(F("> "));
  if (lf) Serial.println(msg);
  else    Serial.print(msg);
}


void systemEcho(const String& msg, bool lf = true) {
  terminalEcho(F("System"), msg, lf);
}


void moduleEcho(const String& msg, bool lf = true) {
  terminalEcho(modules[currentModule].name, msg, lf);
}

