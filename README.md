# ESPHome Dial Menu

A beautiful circular dial menu component for **M5Stack Dial** and similar ESP32-based rotary encoder devices, built with ESPHome and LVGL.

![M5Stack Dial](https://static-cdn.m5stack.com/resource/docs/products/core/Dial/img-1.webp)

## Features

✨ **Simple YAML Configuration** - Just define your apps, the component handles all LVGL complexity  
🎨 **Circular App Launcher** - Beautiful rotating menu with smooth animations  
🔄 **Encoder Navigation** - Rotate to select, click to open  
📱 **App System** - Extensible app framework with built-in Switch control  
⏰ **Idle Screen / Screensaver** - Clock display with time-based background colors  
🌍 **Multi-language Support** - English and French localization  
🔌 **Home Assistant Ready** - Control your smart home devices  

## Hardware Requirements

- **M5Stack Dial** (ESP32-S3, GC9A01A 240x240 display, rotary encoder)
- Or any ESP32 with:
  - Round display (GC9A01A or similar)
  - Rotary encoder with push button
  - Optional: Touchscreen

## Quick Start

### 1. Clone the repository

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

### App Types

#### Generic App
```yaml
- name: "Settings"
  icon_type: settings
  color: 0xFD5C4C
```

#### Switch App (Home Assistant)
```yaml
- name: "Living Room"
  icon_type: light
  color: 0xFFB300
  type: switch
  switch_id: living_room_light
```

### Available Icons

`settings`, `wifi`, `bluetooth`, `brightness`, `home`, `music`, `timer`, `temperature`, `power`, `light`, `fan`, `lock`, `play`, `pause`, `stop`, `next`, `info`, `warning`, `check`, `cross`

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

## Project Structure

```
esphome-dial/
├── dial-menu.yaml           # Main configuration
├── secrets.yaml.example     # Template for secrets
├── components/
│   └── dial_menu/
│       ├── __init__.py      # ESPHome component definition
│       ├── dial_menu_controller.h/cpp  # Main controller
│       ├── idle_screen.h/cpp           # Screensaver
│       └── switch_app.h/cpp            # Switch app type
└── fonts/
    └── montserrat/          # Custom fonts (downloaded)
```

## Development

### Adding a New App Type

1. Create `my_app.h` and `my_app.cpp` in `components/dial_menu/`
2. Inherit from `DialApp` class
3. Override `create_app_ui()`, `on_enter()`, `on_exit()`
4. Register in `__init__.py`

### Building

```bash
esphome compile dial-menu.yaml
```

### Flashing

```bash
esphome upload dial-menu.yaml --device /dev/cu.usbmodem1101
```

## License

MIT License - see [LICENSE](LICENSE) file.

## Credits

- [ESPHome](https://esphome.io/) - Amazing home automation firmware
- [LVGL](https://lvgl.io/) - Light and Versatile Graphics Library
- [M5Stack](https://m5stack.com/) - Great ESP32 hardware

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
