#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

// Persistent config in EEPROM (ESP8266 emulated). Load on boot, store/clear via API or WebSerial.

// Load saved config and apply to LED effects. If no valid config, does nothing (caller applies defaults).
// Returns true if a valid config was loaded and applied, false otherwise.
bool configStoreLoad();

// Save current LED effect settings to EEPROM. Returns true on success.
bool configStoreSave();

// Invalidate saved config in EEPROM. Does not change runtime; caller should apply defaults.
// Returns true on success.
bool configStoreClear();

#endif /* CONFIG_STORE_H */
