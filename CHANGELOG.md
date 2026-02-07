# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2026-02-07

### Added
- **CoverApp** - Control gates, blinds, garage doors with Open/Stop/Close buttons
- **ClimateApp** - Thermostat control with encoder temperature adjustment
- **MediaPlayerApp** - Media player control with play/pause, prev/next, volume control
- **homeassistant_cover** - New ESPHome component to import Home Assistant cover entities (gates, blinds, garage doors)
- **homeassistant_climate** - New ESPHome component to import Home Assistant climate entities
- **homeassistant_media_player** - New ESPHome component to import Home Assistant media player entities
- New icons: `gate`, `garage`, `blinds`, `window`, `thermostat`, `hvac`, `media_player`, `speaker`, `tv`
- Volume control via encoder in MediaPlayerApp
- Temperature control via encoder in ClimateApp
- Debounced commands to avoid API flooding

### Changed
- Updated README with new app types documentation
- Improved project structure documentation

## [0.1.0] - 2026-02-03

### Added
- Initial release of ESPHome Dial Menu component
- Circular app launcher with LVGL
- Rotary encoder navigation (rotate to select, click to open)
- Long press to return to launcher from any app
- Idle screen / screensaver with clock display
- Time-based background colors (night, morning, afternoon, evening)
- Multi-language support (English, French)
- SwitchApp for controlling Home Assistant switches
- FontAwesome icons for apps
- Configurable button sizes and layout radius

### Hardware Support
- M5Stack Dial (ESP32-S3, GC9A01A display)
- Generic ESP32 with rotary encoder and round display
