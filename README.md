**Testovací** repozitář pro ověření defaultní konfigurace a instalace.

Firmware ke stažení je ve složce [rms02/build/fw/](./rms02/build/fw/).

Je potřeba instalovat **správnou** verzi
- velikost LCD dispeje (16X2 nebo 20X4)
- adresa LCD dispeje (0x20, 0x27, 0x3F)
- procesor

Díky za vyzkoušení a zpětnou vazbu.

Defaultní konfigurace:
- displej 1: D0-D6, adresa 0 (dvojtečka na D0)
- displej 2: D0-D6, adresa 8 (dvojtečka na D0)

Do Stopek v4.1 se nevejde Nastavení, proto je zvlášť (M0).
M1M3 jsou moduly 1 a 3 - tedy "Režim1" a "Režim 3".
(Režim 2 udělám až po zpracování připomínek z testování M1 a M3.)

Build pro v5.0 obsahuje všechny (zatím vytvořené) moduly.

**Není určeno do produkce.**

Před nahráním se **ujistěte**, že máte k dispozici aktuálně používaný FW ve formátu .hex!

Nastavení ještě nemá dokumentaci (a obsahuje známou chybu).

Použití příkazu 'set' v Nastavení **přepíše** EEPROM!

Nastavení "HU" - Modul 1, klíč 0: odpočet HH,MM,SS a zpoždění aktualizace externích displejů S, ds (při přepínání ze závodu na odpočet)

Pro odpočet 3m 30s a zpoždění 2,9 sekundy:
`set 1 0   0, 3, 30,   2, 9`

Pro odpočet 1m 15s a okamžitou změnu:
`set 1 0   0, 1, 15,   0, 0`


Jestli potřebujete (chcete) testovat konfiguraci nastavení displejů, napište mi.

**Ovládání:**

V Modulu (pokud to podporuje) se jeho režim mění tlačítkem "Reset" (Menu->Odpočet->Závod->...)

Modul se mění podržením Reset a tlačítky L nebo P. Po uvolnění Resetu se přepne.
