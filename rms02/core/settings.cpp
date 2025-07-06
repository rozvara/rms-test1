// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

void writeBlock(uint8_t index, const Record& rec) {
  uint16_t addr = index * BLOCK_SIZE;
  for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
    EEPROM.update(addr + i, ((const uint8_t*)&rec)[i]);
  }
}


bool delEeprom(uint8_t idModul, uint8_t idKey) {
  Record r;
  for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
    readBlock(i, r);
    if (!validCrc(r)) continue;
    if ((r.idModul == idModul) && (r.idKey & 0x7F) == idKey) {
      uint8_t buf[12] = {0};
      memcpy(r.data, buf, DATA_LEN);
      r.idModul = idModul | 0x80; // 7.bit=smazaný záznam
      r.crc = crc16((uint8_t*)&r, 2 + DATA_LEN);
      writeBlock(i, r);
      return true;
    }
  }
  return false;
}


bool setEeprom(uint8_t idModul, uint8_t idKey, const uint8_t* buf) {
  Record r;

  // existuje? -> aktualizovat
  for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
    readBlock(i, r);
    if (!validCrc(r)) continue;
    if ((r.idModul == idModul) && (r.idKey & 0x7F) == idKey) {
      if (r.idKey != idKey) return false; // text/uint mišmaš
      memcpy(r.data, buf, DATA_LEN);
      r.crc = crc16((uint8_t*)&r, 2 + DATA_LEN);
      writeBlock(i, r);
      return true;
    }
  }

  int8_t unused = -1;
  int8_t deleted = -1;

  // neexistuje? -> první nepoužitý/vadný blok (nemá CRC)
  for (int8_t i = 0; i < BLOCK_COUNT; i++) {
    readBlock(i, r);
    if (!validCrc(r)) {
      unused = i;
      break;
    }
  }

  // není žádný volný? -> první smazaný
  if (unused == -1) {
    for (int8_t i = 0; i < BLOCK_COUNT; i++) {
      readBlock(i, r);
      if (validCrc(r) && (r.idModul & 0x80)) {
        deleted = i;
        break;
      }
    }
  }

  r.idModul = idModul;
  r.idKey = idKey;
  memcpy(r.data, buf, DATA_LEN);
  r.crc = crc16((uint8_t*)&r, 2 + DATA_LEN);

  if (unused != -1) {
    writeBlock((uint8_t)unused, r);
    return true;
  } else if (deleted != -1) {
    writeBlock((uint8_t)deleted, r);
    return true;
  } else {
    systemEcho(F("EEPROM plna!"));
    return false;
  }
}


bool setEeprom(uint8_t idModul, uint8_t idKey, const char* buf) {
  return setEeprom(idModul, idKey | 0x80, (const uint8_t*)buf);
}


void showLcdSetupInstructions() {
  lcdClear();
  #ifdef LCD_16X2
    lcdModuleTitle(F("Terminal"));
    lcdWrite(0, 1, F("115200bit 'help'"));
  #endif
  #ifdef LCD_20X4
    lcdModuleTitle(F("Nastaveni"));
    lcdWrite(0, 2, F("Serial 115200/8/1bit"));
    lcdWrite(2, 3, F("Terminal: 'help'"));
  #endif
  intDisp.write(F("--55-- ")); // jako System Setup :)
  extDis1.fill(' ');
  extDis2.fill(' ');  
}


void showSystemInfo() {

  Serial.print(F("HW: ")); Serial.println(F(HW_NAME)); 

  #define BUILD_DATE __DATE__
  #define BUILD_TIME __TIME__
  Serial.print(F("FW: ")); Serial.print(F(FW_VERSION)); Serial.print(F(FW_TAG));
  Serial.print(F(" (")); Serial.print(F(BUILD_DATE)); Serial.write(' '); Serial.print(F(BUILD_TIME)); Serial.println(F(")"));
  Serial.print(F("    Pocet modulu: ")); Serial.println(modulesInstalled);
  Serial.print(F("    Log zaznamu: ")); Serial.println(MAX_LOG_ENTRIES);
  // TODO EEPROM records
}


void showHelp() {
  Serial.println(F("\nPrikazy:"));
  Serial.println(F("  list [m] [k]"));
  Serial.println(F("  set <m> <k> \"text\""));
  Serial.println(F("  set <m> <k> H [,H,H...]"));
  Serial.println(F("  del <m> <k>"));

#ifdef HW_RTC_DS3231
  Serial.println(F("  rtc YYYY-MM-DD HH:MM:SS"));
#endif

//  TODO interní hodiny
//  Serial.println(F("  time HH:MM:SS        - nastaví čas bez RTC"));

  Serial.println(F("  perf <0|1>"));
  Serial.println(F("  info"));
  Serial.println(F("  help"));
}


void moduleSetupLoop() {

  if (moduleState == MOD_EXITING) return;
  if (moduleState == MOD_SHUTDOWN) { 
    moduleEcho(F("Konec"));
    intDisp.fill(' ');
    moduleState = MOD_EXITING;
    return;
  }
  if (moduleState == MOD_STARTING) {
    showLcdSetupInstructions();
    while (Serial.available()) Serial.read(); // zahoď buffer před spuštěním
    moduleState = MOD_IDLE;
    return;
  }

  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  moduleEcho(F("Prijato '"), false); Serial.print(cmd); Serial.println("'");

  bool cmdDone = false;
  bool cmdError = false;

  if (cmd.startsWith("list")) {
    int modFilter = -1, keyFilter = -1;
    sscanf(cmd.c_str(), "list %d %d", &modFilter, &keyFilter);

    Serial.println(F("Modul Klic Data"));
    Serial.println(F("---------------"));
    uint8_t count = 0;
    for (int i = 0; i < BLOCK_COUNT; i++) {
      Record r;
      readBlock(i, r);
      if (!validCrc(r) || (r.idModul & 0x80)) continue; // vadný nebo smazaný

      uint8_t mod = r.idModul & 0x7F;
      uint8_t key = r.idKey & 0x7F;
      bool isText = r.idKey & 0x80;

      if ((modFilter >= 0 && mod != modFilter) ||
          (keyFilter >= 0 && key != keyFilter)) continue;

      Serial.print("  ");
      Serial.print(mod < 100 ? (mod < 10 ? F("  ") : F(" ")) : F(""));
      Serial.print(mod); Serial.print("  ");
      Serial.print(key < 100 ? (key < 10 ? F("  ") : F(" ")) : F(""));
      Serial.print(key); Serial.write(' ');
//            Serial.print(isText ? F("znaky ") : F("cisla "));

      if (isText) {
        Serial.print('"');
        for (uint8_t j = 0; j < DATA_LEN; j++) {
          char c = (char)r.data[j];
          if (c == 0) break;
          Serial.print(c);
        }
        Serial.println('"');
      } else {
        for (uint8_t j = 0; j < DATA_LEN; j++) {
          if (j > 0) Serial.write(',');
          Serial.print(r.data[j] < 100 ? (r.data[j] < 10 ? F("  ") : F(" ")) : F(""));
          Serial.print(r.data[j]);
        }
        Serial.println();
      }
      count++;
    }
    if (count > 0) Serial.println(F("---------------"));
    Serial.print(F("Nalezeno: ")); Serial.println(count);
    cmdDone = true;
  }

  if (cmd.startsWith("set ")) {
    int m, k;
    String msg;
    msg = F("Chybny vstup.");
    const char* src = cmd.c_str();
    if (sscanf(src, "set %d %d", &m, &k) == 2) {
      // set
      if (m>=0 || m<=127 || k>=0 || k<=127) {
        const char* p = strchr(src, ' ');
        if (!p) goto fail;
        p = strchr(p + 1, ' ');
        if (!p) goto fail;
        p = strchr(p + 1, ' ');
        if (!p) goto fail;
        while (*p == ' ') p++;

        if (*p == '"') { // text
          p++;
          const char* end = strchr(p, '"');
          if (!end) goto fail;
          char buf[12] = {0};
          strncpy(buf, p, min(12, (int)(end - p)));
          if (setEeprom(m, k | 0x80, buf))
            msg = F("OK. Text ulozen.");
          else 
            // FIXME err do core
            msg = F("Chyba. (Text pres hodnoty?)");
        } else { // hodnoty oddělené čárkami
          uint8_t buf[12] = {0};
          uint8_t count = 0;
          while (*p && count < 12) {
            while (*p == ' ') p++;
            int val = atoi(p);
            if (val < 0 || val > 255) goto fail;
            buf[count++] = val;
            p = strchr(p, ',');
            if (!p) break;
            p++;
          }
          if (setEeprom(m, k & 0x7F, buf))
            msg = ("OK. Hodnoty ulozeny.");
          else 
            msg = F("Chyba. (Hodnoty pres text?)");
        }
        fail:
      }
    }
    Serial.println(msg);
    return;
  }

  if (cmd.startsWith("del ")) {
    // FIXME maže i smazaný/odsmaže
    int m, k;
    if (sscanf(cmd.c_str(), "del %d %d", &m, &k) == 2) {
      if (m>=0 || m<=127 || k>=0 || k<=127) {
        if (delEeprom(m, k)) {
          Serial.println(F("Zaznam smazan."));
          cmdDone = true;
        }
        else {
          Serial.print(F("Nenalezeno. "));
          cmdError = true; cmdDone = true;
        }
      } else {
        cmdError = true; cmdDone = true;
      }
    }
  }

  #ifdef HW_RTC_DS3231
    if (cmd.startsWith("rtc ")) {
      String rest = cmd.substring(4);
      rest.trim();

      const char* raw = rest.c_str();
      int y, mo, d, h, mi, s;
      if (6 == sscanf(raw, "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s)) {
        rtc.adjust(DateTime(y, mo, d, h, mi, s));
        Serial.print(F("Cas v RTC: "));
        DateTime dt = rtc.now();
        char buffer[20];
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
          dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
        Serial.println(buffer);
      } else {
        cmdError = true;
      }
      cmdDone = true; 
    }
  #endif

  if (cmd.startsWith("perf ")) {
    String rest = cmd.substring(5);
    int val = rest.toInt();
    if (val == 0) { showPerfOverlay = false; showLcdSetupInstructions(); }
    else if (val == 1) { showPerfOverlay = true; }
    else { cmdError = true; }
    cmdDone = true;
  }

  if (cmd == "help") {
    showHelp();
    cmdDone = true;
  }

  if (cmd == "info") {
    showSystemInfo();
    cmdDone = true;
  }

  if (!cmdDone) {
    Serial.print(F("Neznamy prikaz."));
  }
  if (cmdError) {
    Serial.print(F("Chyba."));
  }
  if (!cmdDone || cmdError) {
    Serial.println(F(" Zadej 'help'.")); buzzer(WARN_BEEP);
  } else {
    Serial.println(); buzzer(INFO_BEEP);
  }
}
