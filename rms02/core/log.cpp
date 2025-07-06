// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#ifndef MAX_LOG_ENTRIES
  #define MAX_LOG_ENTRIES 16
#endif

static LogEntry logBuffer[MAX_LOG_ENTRIES];
static uint8_t logIndex = 0;


bool logEvent(LogEventType type, uint32_t time, const char* info) {

  if ((type != LOG_START) && (type != LOG_FINISH_LEFT) && (type != LOG_FINISH_RIGHT)) {
    if (logIndex>=MAX_LOG_ENTRIES-3) { // musí zůstat místo pro konec závodu
      systemEcho(F("NEULOZENO"));
      return false;
    }
  }

  logBuffer[logIndex].timestamp_ms = time;
  logBuffer[logIndex].type = type;
  logBuffer[logIndex].info = info;

  systemEcho(F("Log typ#"), false); Serial.print((uint8_t)type);
  Serial.println(F(" ulozen"));

  logIndex++;
  return true;
}


void printTime(Stream& out, uint32_t ms, bool hours, bool stamp) {

    uint32_t s = ms / 1000UL;
    uint16_t millis = ms % 1000;
    uint8_t sec = s % 60;
    uint8_t min = (s / 60) % 60;
    uint16_t hrs = s / 3600;

    char charbuf[16];

    if (stamp) out.print("[+");
    if (hours) {
      sprintf(charbuf, "%03u:%02u:%02u.%03u", hrs, min, sec, millis); 
    } else {
      sprintf(charbuf, "%02u:%02u.%03u", min, sec, millis);
    }
    out.print(charbuf);
    if (stamp) out.print("]");
}


void logDump(Stream& out) {

  out.println(F("\n=== Zaznam zavodu ==="));

  // TODO RTC
  // #ifdef HW_RTC_DS3231
  //   DateTime now = rtc.now();
  //   out.print(F("Log dump at "));
  //   out.print(now.timestamp());
  //   out.println();
  // #endif

  // Závěrečné vyhodnocení
  uint32_t tStart = 0xFFFFFFFF;
  uint32_t tLeft = 0;
  uint32_t tRight = 0;
  uint32_t tEnd = 0;

  bool hours = false;

  for (uint8_t i = 0; i < logIndex; i++) {  // najdi
    if (logBuffer[i].type == LOG_START) tStart = logBuffer[i].timestamp_ms;
    else if (logBuffer[i].type == LOG_FINISH_LEFT) tLeft = logBuffer[i].timestamp_ms;
    else if (logBuffer[i].type == LOG_FINISH_RIGHT) tRight = logBuffer[i].timestamp_ms;
  }

  tEnd = (tLeft > tRight) ? tLeft : tRight;

  if ((tStart != 0xFFFFFFFF) && (tEnd > 0)) {
    if (tEnd - tStart >= 3600000-1 ) {
      hours = true;
    }
  }

  for (uint16_t i = 0; i < logIndex; i++) {
    uint32_t rel = logBuffer[i].timestamp_ms - tStart;

    printTime(out, rel, hours, true);

    out.print(" ");

    switch (logBuffer[i].type) {
      case LOG_START: out.print(F("START")); break;
      case LOG_SPLIT: out.print(F("MEZICAS")); break;
      case LOG_FINISH_LEFT: out.print(F("CAS LEVA DRAHA")); break;
      case LOG_FINISH_RIGHT: out.print(F("CAS PRAVA DRAHA")); break;
      case LOG_RESET: out.print(F("RESET")); break;
      case LOG_INFO:  out.print(F("INFO")); break;
      case LOG_ERROR: out.print(F("CHYBA")); break;
    }

    if (logBuffer[i].info) {
        out.print(F(" - "));
        out.print(logBuffer[i].info);
    }
    out.println();
  }

  if (tStart != 0xFFFFFFFF && tLeft > 0 && tRight > 0) {
    uint32_t dLeft = tLeft - tStart;
    uint32_t dRight = tRight - tStart;

    out.println();
    out.println(F("--- Souhrn zavodu ---"));

    out.print(F("Levy cas:  ")); printTime(out, dLeft, hours, false); out.println();
    out.print(F("Pravy cas: ")); printTime(out, dRight, hours, false); out.println();

    if (dLeft < dRight) {
        out.println(F("Vitez:     LEVA DRAHA"));
        out.print(  F("Rozdil:    "));
        out.print((dRight - dLeft) / 1000.0, 3);
        out.println(" s");
    } else if (dRight < dLeft) {
        out.println(F("Vitez:     PRAVA DRAHA"));
        out.print(  F("Rozdil:    "));
        out.print((dLeft - dRight) / 1000.0, 3);
        out.println(" s");
    } else {
        out.println(F("Vysledek:  NEROZHODNE"));
    }
  }

  out.println(F("=== Konec zaznamu ===\n"));
}


void logClear() {
  for (uint16_t i = 0; i < MAX_LOG_ENTRIES; i++) {
    logBuffer[i].timestamp_ms = 0xFFFFFFFF;
    logBuffer[i].type = LOG_NONE;
    logBuffer[i].info = nullptr;
  }
  logIndex = 0;

  systemEcho(F("Log pripraven"));
}


// void logTest() {
//   logClear();
//   logEvent(LOG_START, 2796);
//   logEvent(LOG_FINISH_LEFT, 55980);
//   logEvent(LOG_FINISH_RIGHT, 31491617);
//   logDump(Serial);
// }
