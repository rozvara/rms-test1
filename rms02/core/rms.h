// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

// === MODUL
enum ModuleState {
  MOD_IDLE      = 0, // NUTNÝ stav pro přepnutí modulu
  MOD_READY     = 1, // připraven ke startu
  MOD_RUNNING   = 2, // běží závod
  MOD_PAUSED    = 3, // měření pozastaveno
  MOD_STOPPED   = 4, // závod skončil
  MOD_PREPARE   = 5, // příprava (na countdown)
  MOD_COUNTDOWN = 6, // odpočítává
  MOD_BUSY      = 7,
  MOD_ERROR     = 8,
  // ...
  // ...
  MOD_STARTING, // vstupní stav (init)
  MOD_SHUTDOWN, // pokyn "ukonči se"
  MOD_EXITING   // stav "skončil jsem"
};
ModuleState moduleState;

void (*moduleLoop)() = nullptr;
uint8_t currentModule = 0;
uint8_t modulesInstalled = 0;

void changeModuleState(ModuleState newState);
bool firstEntry = true; // FSM helpers
bool delayedFirstEntry = false;
uint32_t entryTime = 0;


// === DRÁHA
// šablona dvojteček (pro TrackState)  o=nic  k=svítí  h=bliká
const char trackColon[6] = {'k','h','k','h','o','o'};

// stav dráhy je současně index pro dvojtečku ext. displejů
enum TrackState { 
  TRACK_READY   = 0,
  TRACK_RUNNING = 1,
  TRACK_STOPPED = 2,
  TRACK_PAUSED  = 3,
  TRACK_OFF     = 4,
  TRACK_ERROR   = 5
};

struct Track {
  TrackState state;
  uint32_t startTime;
  uint32_t finishTime;
  char colon;

  void changeState(TrackState newState) {
    state = newState;
    colon = trackColon[newState];
  }
};
Track leftTrack;
Track rightTrack;


// === ČASOVÁNÍ
volatile uint32_t _exact_millis = 0;
volatile uint8_t _fps_ms = 0;
volatile uint8_t _tim_ms = 0;
volatile bool _fps_flag = false;
volatile bool _tim_flag = false;
uint8_t _tim_cnt = 0;

bool timerOneSec = false; // pro modul
bool timerHalfSec = false;
bool timerFPS = false;


// Timer1 na 1ms
inline void initTimer() {
  #if F_CPU == 16000000L
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    OCR1A = 249;
    TIMSK1 |= (1 << OCIE1A);
    sei();
  #else
    #error "Chybí nastavení pro zvolenou frekvenci"
  #endif
}

uint32_t exactMillis();
uint16_t exactMicros();
void resetTimer();


// ===
bool showPerfOverlay = false;
uint32_t loops = 0;
void performanceOverlay();


// ===
#define INFO_BEEP 0
#define WARN_BEEP 1
#define READY_BEEP 2
//#define LONG_BEEP 2
//#define CRITICAL_BEEP 3

void buzzer(uint8_t type);

uint8_t buzzCount;
uint16_t buzzFreq;
uint16_t buzzOnTime; // ms
uint16_t buzzOffTime;
uint32_t buzzTimer;
bool buzzState;


// === RTC
#ifdef HW_RTC_DS3231
  #include <RTClib.h>
  RTC_DS3231 rtc;
#endif


// === NASTAVENÍ
#include <EEPROM.h>
#define BLOCK_SIZE  16
#define BLOCK_COUNT 64    // 64*16 = 1024; TODO podle procesoru EEPROM.length()
#define DATA_LEN    12   // SIZE-4 (2=id, 2=crc)
// index je uint8_t -> max 255x16 (4kB)

struct Record {
    uint8_t idModul;  // bit 7 = deleted
    uint8_t idKey;    // bit 7 = typ (0=uint8_t, 1=char)
    uint8_t data[DATA_LEN];
    uint16_t crc;
};


bool getEeprom(uint8_t idModul, uint8_t idKey, char* buf);
bool getEeprom(uint8_t idModul, uint8_t idKey, uint8_t* buf);
