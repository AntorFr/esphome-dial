# ESPHome Dial Menu

A beautiful circular dial menu component for **M5Stack Dial** and similar ESP32-based rotary encoder devices, built with ESPHome and LVGL.

![M5Stack Dial](https://static-cdn.m5stack.com/resource/docs/products/core/Dial/img-1.webp)

## Features

✨ **Simple YAML Configuration** - Just define your apps, the component handles all LVGL complexity  
🎨 **Circular App Launcher** - Beautiful rotating menu with smooth animations  
🔄 **Encoder Navigation** - Rotate to select, click to open  
📱 **App System** - Multiple app types: Switch, Cover, Climate, Media Player  
⏰ **Idle Screen / Screensaver** - Clock display with time-based background colors  
🌍 **Multi-language Support** - English and French localization  
🔌 **Home Assistant Ready** - Control your smart home devices directly  

## Quick Start (ESPHome Dashboard)

**Copy-paste this into ESPHome Dashboard** - everything is imported from GitHub!

```yaml
substitutions:
  device_name: my-dial
  friendly_name: My Dial

esphome:
  name: ${device_name}
  friendly_name: ${friendly_name}

# Import packages from GitHub
packages:
  m5dial: github://antorfr/esphome-dial/packages/m5stack_dial.yaml@main
  lvgl_config: github://antorfr/esphome-dial/packages/dial_menu_lvgl.yaml@main

external_components:
  - source: github://antorfr/esphome-dial@main
    components: [dial_menu]

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

time:
  - platform: sntp
    id: sntp_time

sensor:
  - platform: rotary_encoder
    id: !extend dial_encoder
    on_clockwise:
      - lambda: id(menu_controller).on_encoder_rotate(1);
    on_anticlockwise:
      - lambda: id(menu_controller).on_encoder_rotate(-1);

binary_sensor:
  - platform: gpio
    id: !extend dial_button
    on_multi_click:
      - timing:
          - ON for at most 400ms
          - OFF for at least 50ms
        then:
          - lambda: id(menu_controller).on_button_click();
      - timing:
          - ON for at least 500ms
        then:
          - lambda: id(menu_controller).on_long_press();

switch:
  - platform: template
    id: my_light
    name: "Light"
    optimistic: true

dial_menu:
  id: menu_controller
  display_id: round_display
  time_id: sntp_time
  apps:
    - name: "Settings"
      icon_type: settings
      color: 0xFD5C4C
    - name: "Light"
      type: switch
      icon_type: light
      switch_id: my_light
      color: 0xFFB300
```

## Packages

| Package | Description | Import |
|---------|-------------|--------|
| `m5stack_dial.yaml` | M5Stack Dial hardware | `github://antorfr/esphome-dial/packages/m5stack_dial.yaml@main` |
| `dial_menu_lvgl.yaml` | LVGL initialization | `github://antorfr/esphome-dial/packages/dial_menu_lvgl.yaml@main` |
| `french_fonts.yaml` | French accent support | `github://antorfr/esphome-dial/packages/french_fonts.yaml@main` |

## French Language Support

For French with accented characters (é, è, à, ç...):

```yaml
packages:
  m5dial: github://antorfr/esphome-dial/packages/m5stack_dial.yaml@main
  lvgl_config: github://antorfr/esphome-dial/packages/dial_menu_lvgl.yaml@main
  french_fonts: github://antorfr/esphome-dial/packages/french_fonts.yaml@main

dial_menu:
  language: fr
  font_14: montserrat_fr_14
  font_18: montserrat_fr_18
  apps:
    - name: "Lumières"  # Accents work!
```

## Hardware Requirements

- **M5Stack Dial** (ESP32-S3, GC9A01A 240x240 display, rotary encoder)
- Or any ESP32 with:
  - Round display (GC9A01A or similar)
  - Rotary encoder with push button
  - Optional: Touchscreen

## Local Development

### Clone the repository

```bash
git clone https://github.com/your-username/esphome-dial.git
cd esphome-dial
```

### 2. Create your secrets file

```bash
cp secrets.yaml.example secrets.yaml
# Edit secrets.yaml with your WiFi credentials
```

### 3. Install ESPHome

```bash
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install esphome
```

### 4. Flash to your device

```bash
esphome run dial-menu.yaml
```

## Configuration

### Basic Example

```yaml
dial_menu:
  id: menu_controller
  display_id: round_display
  time_id: sntp_time
  idle_timeout: 30s
  language: fr  # Options: en, fr
  apps:
    - name: "Settings"
      icon_type: settings
      color: 0xFD5C4C
    - name: "Light"
      icon_type: light
      color: 0xFFB300
      type: switch
      switch_id: my_light_switch
```

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `display_id` | string | required | ID of the LVGL display |
| `time_id` | string | optional | ID of time component for clock |
| `idle_timeout` | time | `30s` | Time before showing screensaver |
| `language` | string | `en` | Display language (`en`, `fr`) |
| `radius` | int | `85` | Radius of the app circle |
| `button_size` | int | `50` | Size of app buttons |
| `button_size_focused` | int | `58` | Size when focused |
| `font_14` | font_id | optional | Custom font for small text |
| `font_18` | font_id | optional | Custom font for medium text |

### App Types

#### Generic App
```yaml
- name: "Settings"
  icon_type: settings
  color: 0xFD5C4C
```

#### Switch App
Control one or multiple switches:
```yaml
- name: "Lights"
  type: switch
  icon_type: light
  color: 0xFFB300
  switches:
    - switch_id: living_room_light
      name: "Living Room"
    - switch_id: bedroom_light
      name: "Bedroom"
```

#### Cover App
Control gates, blinds, garage doors:
```yaml
- name: "Gate"
  type: cover
  icon_type: gate
  color: 0x577EFF
  covers:
    - cover_id: front_gate
      name: "Front Gate"
```

#### Climate App
Control a thermostat with encoder temperature adjustment:
```yaml
- name: "Thermostat"
  type: climate
  icon_type: thermostat
  color: 0x03A964
  climate_id: living_room_climate
  temperature_step: 0.5  # Optional, default 0.5°C
```

#### Media Player App
Control a Home Assistant media player:
```yaml
- name: "Music"
  type: media_player
  icon_type: speaker
  color: 0x9C27B0
  media_player_id: living_room_speaker
  volume_step: 0.05  # Optional, default 5%
```

> **Note:** Media Player requires the `homeassistant_media_player` component (see below).

#### Generic App
```yaml
- name: "Living Room"
  icon_type: light
  color: 0xFFB300
  type: switch
  switch_id: living_room_light
```

### Available Icons

`settings`, `wifi`, `bluetooth`, `brightness`, `home`, `music`, `timer`, `temperature`, `power`, `light`, `fan`, `lock`, `play`, `pause`, `stop`, `next`, `info`, `warning`, `check`, `cross`, `gate`, `garage`, `blinds`, `window`, `thermostat`, `hvac`, `media_player`, `speaker`, `tv`

## Navigation

| Action | Result |
|--------|--------|
| **Rotate encoder** | Select next/previous app |
| **Short press** | Open selected app |
| **Long press** | Return to launcher |
| **30s inactivity** | Show clock screensaver |

## Idle Screen

The screensaver displays:
- Current time (large format)
- Day of week
- Date

Background color changes based on time of day:
- 🌙 Night (22:00-06:00): Dark blue
- 🌅 Morning (06:00-12:00): Warm tones
- ☀️ Afternoon (12:00-18:00): Light blue
- 🌆 Evening (18:00-22:00): Deep purple

## Examples

Check the [examples/](examples/) folder for:
- **minimal.yaml** - Simplest possible configuration

Or use **dial-menu.yaml** for a full local development example.

## Home Assistant Components

### homeassistant_climate
Import a climate entity from Home Assistant (not available natively in ESPHome):
```yaml
external_components:
  - source: github://antorfr/esphome-dial@main
    components: [dial_menu, homeassistant_climate]

homeassistant_climate:
  - id: my_thermostat
    entity_id: climate.living_room
    temperature_step: 0.5
    min_temperature: 15
    max_temperature: 30
```

### homeassistant_media_player
Import a media player entity from Home Assistant:
```yaml
external_components:
  - source: github://antorfr/esphome-dial@main
    components: [dial_menu, homeassistant_media_player]

homeassistant_media_player:
  - id: my_speaker
    entity_id: media_player.living_room
    volume_step: 0.05
```

## Project Structure

```
esphome-dial/
├── dial-menu.yaml           # Local dev example
├── packages/                # Reusable packages
│   ├── m5stack_dial.yaml      # Hardware config
│   ├── dial_menu_lvgl.yaml    # LVGL setup
│   └── french_fonts.yaml      # French support
├── examples/                # Example configurations
│   └── minimal.yaml           # Copy-paste ready
├── components/
│   ├── dial_menu/
│   │   ├── __init__.py
│   │   ├── dial_menu_controller.h/cpp
│   │   ├── idle_screen.h/cpp
│   │   ├── switch_app.h/cpp
│   │   ├── cover_app.h/cpp
│   │   ├── climate_app.h/cpp
│   │   └── media_player_app.h/cpp
│   ├── homeassistant_climate/
│   │   └── ...                # HA Climate component
│   └── homeassistant_media_player/
│       └── ...                # HA Media Player component
└── fonts/
    └── montserrat/          # Custom fonts

- [ESPHome](https://esphome.io/) - Amazing home automation firmware
- [LVGL](https://lvgl.io/) - Light and Versatile Graphics Library
- [M5Stack](https://m5stack.com/) - Great ESP32 hardware

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
