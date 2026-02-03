"""
dial_menu component for ESPHome - LVGL Backend
Generates complete LVGL configuration from simple dial_menu YAML

The user only needs to declare:
  dial_menu:
    display: my_display
    touchscreen: my_touch
    encoder: my_encoder  
    button: my_button
    apps:
      - name: "Settings"
        icon_type: settings
        color: 0xFD5C4C

And this component automatically generates the full LVGL configuration.
"""
import math
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_TYPE,
    CONF_DISPLAY_ID,
)
from esphome.components import switch
from esphome.components import time as time_component

CODEOWNERS = ["@antorfr"]
DEPENDENCIES = ["lvgl"]
AUTO_LOAD = []

# Configuration keys
CONF_ENCODER_ID = "encoder"
CONF_BUTTON_ID = "button"
CONF_TOUCHSCREEN_ID = "touchscreen"
CONF_APPS = "apps"
CONF_COLOR = "color"
CONF_ICON_TYPE = "icon_type"
CONF_RADIUS = "radius"
CONF_BUTTON_SIZE = "button_size"
CONF_BUTTON_SIZE_FOCUSED = "button_size_focused"
CONF_SWITCH_ID = "switch_id"
CONF_IDLE_TIMEOUT = "idle_timeout"
CONF_TIME_ID = "time_id"
CONF_LANGUAGE = "language"

# Supported languages
LANGUAGES = ["en", "fr"]

# FontAwesome icons mapping (LVGL built-in)
ICON_FONTAWESOME = {
    "none": "",
    "settings": "\uF013",      # gear
    "wifi": "\uF1EB",          # wifi
    "bluetooth": "\uF293",     # bluetooth-b
    "brightness": "\uF185",    # sun
    "home": "\uF015",          # home
    "music": "\uF001",         # music
    "timer": "\uF017",         # clock
    "temperature": "\uF2C9",   # thermometer-half
    "power": "\uF011",         # power-off
    "light": "\uF0EB",         # lightbulb
    "fan": "\uF863",           # fan
    "lock": "\uF023",          # lock
    "play": "\uF04B",          # play
    "pause": "\uF04C",         # pause
    "stop": "\uF04D",          # stop
    "next": "\uF051",          # step-forward
    "info": "\uF129",          # info
    "warning": "\uF071",       # exclamation-triangle
    "check": "\uF00C",         # check
    "cross": "\uF00D",         # times
}

# Default colors for apps
DEFAULT_COLORS = [
    0xFD5C4C,  # Red-orange
    0x577EFF,  # Blue
    0x03A964,  # Green
    0xEB8429,  # Orange
    0x1AA198,  # Teal
    0x9C27B0,  # Purple
    0xFFB300,  # Amber
    0xE91E63,  # Pink
]

# Namespace C++
dial_menu_ns = cg.esphome_ns.namespace("dial_menu")

# Classes C++
DialMenuController = dial_menu_ns.class_("DialMenuController", cg.Component)
DialApp = dial_menu_ns.class_("DialApp")
SwitchApp = dial_menu_ns.class_("SwitchApp", DialApp)


def app_schema(app_type):
    """Return schema based on app type"""
    base = {
        cv.GenerateID(): cv.declare_id(DialApp),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_ICON_TYPE, default="none"): cv.one_of(*ICON_FONTAWESOME, lower=True),
        cv.Optional(CONF_COLOR): cv.hex_uint32_t,
        cv.Optional(CONF_TYPE, default="generic"): cv.string,
    }
    return cv.Schema(base)


# Schéma pour une app - now with conditional switch_id
APP_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DialApp),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_ICON_TYPE, default="none"): cv.one_of(*ICON_FONTAWESOME, lower=True),
        cv.Optional(CONF_COLOR): cv.hex_uint32_t,
        cv.Optional(CONF_TYPE, default="generic"): cv.string,
        cv.Optional(CONF_SWITCH_ID): cv.use_id(switch.Switch),
    }
)

# Schéma principal du composant
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DialMenuController),
        cv.Required(CONF_DISPLAY_ID): cv.string,
        cv.Optional(CONF_TOUCHSCREEN_ID): cv.string,
        cv.Optional(CONF_ENCODER_ID): cv.string,
        cv.Optional(CONF_BUTTON_ID): cv.string,
        cv.Optional(CONF_APPS, default=[]): cv.ensure_list(APP_SCHEMA),
        cv.Optional(CONF_RADIUS, default=85): cv.int_range(min=50, max=110),
        cv.Optional(CONF_BUTTON_SIZE, default=50): cv.int_range(min=30, max=80),
        cv.Optional(CONF_BUTTON_SIZE_FOCUSED, default=58): cv.int_range(min=30, max=90),
        cv.Optional(CONF_IDLE_TIMEOUT, default="30s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_TIME_ID): cv.use_id(time_component.RealTimeClock),
        cv.Optional(CONF_LANGUAGE, default="en"): cv.one_of(*LANGUAGES, lower=True),
    }
).extend(cv.COMPONENT_SCHEMA)


def calculate_icon_positions(num_apps, radius=85):
    """Calculate x,y positions for icons arranged in a circle"""
    positions = []
    for i in range(num_apps):
        angle = (2 * math.pi * i / num_apps) - (math.pi / 2)  # Start from top
        x = int(radius * math.cos(angle))
        y = int(radius * math.sin(angle))
        positions.append((x, y))
    return positions


async def to_code(config):
    """Generate C++ code for the dial_menu controller"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Get apps configuration
    apps = config.get(CONF_APPS, [])
    radius = config.get(CONF_RADIUS, 85)
    
    # Calculate positions for icons
    num_apps = len(apps)
    positions = calculate_icon_positions(num_apps, radius) if num_apps > 0 else []
    
    # Register apps with controller
    for i, app_conf in enumerate(apps):
        app_type = app_conf.get(CONF_TYPE, "generic")
        app_id = app_conf[CONF_ID]
        
        # Create the right type of app
        if app_type == "switch":
            # SwitchApp for controlling switches - override the type
            app_id.type = SwitchApp
            app_var = cg.new_Pvariable(app_id)
            
            # Set the switch entity if provided
            if CONF_SWITCH_ID in app_conf:
                sw = await cg.get_variable(app_conf[CONF_SWITCH_ID])
                cg.add(app_var.set_switch(sw))
        else:
            # Generic DialApp
            app_var = cg.new_Pvariable(app_id)
        
        cg.add(app_var.set_name(app_conf[CONF_NAME]))
        cg.add(app_var.set_index(i))
        
        # Set color (use default if not specified)
        color = app_conf.get(CONF_COLOR, DEFAULT_COLORS[i % len(DEFAULT_COLORS)])
        cg.add(app_var.set_color(color))
        
        # Set icon type (e.g., "settings", "wifi", etc.) - C++ will map to LVGL symbols
        icon_type = app_conf.get(CONF_ICON_TYPE, "none")
        cg.add(app_var.set_icon(icon_type))
        
        # Set position
        if i < len(positions):
            cg.add(app_var.set_position(positions[i][0], positions[i][1]))
        
        # Add to controller
        cg.add(var.add_app(app_var))
    
    # Set group name
    cg.add(var.set_group_name("dial_menu_group"))
    
    # Store config for LVGL generation
    cg.add(var.set_button_size(config.get(CONF_BUTTON_SIZE, 50)))
    cg.add(var.set_button_size_focused(config.get(CONF_BUTTON_SIZE_FOCUSED, 58)))
    
    # Idle screen configuration
    cg.add(var.set_idle_timeout(config.get(CONF_IDLE_TIMEOUT)))
    
    # Time source for idle screen
    if CONF_TIME_ID in config:
        time_var = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time(time_var))
    
    # Language setting
    lang = config.get(CONF_LANGUAGE, "en")
    cg.add(var.set_language(lang))
