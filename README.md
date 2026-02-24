# halo

LED strip controller for ESP8266 (NodeMCU). Web UI over WiFi, REST API, and a bunch of effects. OTA updates so you can flash it without plugging in USB.

## Hardware

- ESP8266 (tested on NodeMCU v2/v3, board `huzzah` in PlatformIO)
- WS2812B/WS2815 strip on GPIO 2
- Pin and length are in `include/config.h` (`LED_PIN`, `LED_COUNT`)

## Setup

1. Clone and open in PlatformIO (VS Code or CLI).
2. **WiFi:** Copy `include/wifi_credentials.h.example` to `include/wifi_credentials.h` and set your SSID and password. The real `wifi_credentials.h` is gitignored so you don’t commit secrets.
3. Build and upload: `pio run -t upload`. Default is OTA to the IP in `platformio.ini` (`upload_port`); set your device IP there or pass `--upload-port <ip>`.

(Initial OS flash required; afterwards OTA updates will work, make sure to keep including the OTA setup each time)


## What it does

- Connects to your WiFi and runs a small web server.
- **WebSerial:** Open the device IP in a browser for a serial-style console to change effects, brightness, speed, colours, etc.
- **REST API:** Same controls via HTTP (see `postman/` for a collection if you use Postman).
- **Persistent config:** Use the API or WebSerial to run `config-store` (or `POST /api/config/store`) to save current settings to EEPROM, or `config-clear` to reset to defaults. Settings are loaded on boot.

Effects live in `src/effects/`; the engine and config are in `src/led_effects.cpp` and the rest of `src/`. There’s a more detailed refactor guide in `docs/EFFECTS_SPLIT_GUIDE.md` if you’re poking around the effect code.

## Build / upload

```bash
pio run
pio run -t upload
pio run -t upload --upload-port 10.0.0.5   # OTA to a specific IP
```

## License

Licensed under MIT — see [LICENSE](LICENSE).