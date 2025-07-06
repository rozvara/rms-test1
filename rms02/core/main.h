// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

void setup();
void loop();

void handleModuleSwap();
void swapModule(uint8_t newModule);
void systemEcho(const String& msg, bool lf = true);
void moduleEcho(const String& msg, bool lf = true);
