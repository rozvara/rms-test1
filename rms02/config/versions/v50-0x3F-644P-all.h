// B-0096 
#define HW_VER_50
#define LCD_20X4
#define LCD_I2C0X3F
#define TARGET_MCU_ATMEGA644P

#define INPUT_DEBOUNCE 200	//max 255

#define FW_TAG "+v50-20X4-0x3F-all" // firmware build tag (for customized versions)

#define MAX_LOG_ENTRIES 16

#define MODULE_01 // HU
	//#define MODULE_02 // S-S
#define MODULE_03 // S-LP
#define MODULE_04 // Hodiny
#define MODULE_99 // Demo
#define MODULE_SETUP // Nastavení

#define HW_RTC_DS3231
