// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

enum LogEventType : uint8_t {
    LOG_NONE = 0,
    LOG_START,
    LOG_SPLIT, // mezičas
    LOG_FINISH_LEFT,
    LOG_FINISH_RIGHT,
    LOG_RESET,
    LOG_INFO,
    LOG_ERROR
};

struct LogEntry {
    uint32_t timestamp_ms;
    LogEventType type;
    const char* info; // volitelný text (musí být v paměti při dumpu)
};

bool logEvent(LogEventType type, uint32_t now_ms, const char* info = nullptr);
void logDump(Stream& out);
void logClear();
