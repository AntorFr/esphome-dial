"""
dial_menu component for ESPHome - LVGL Backend
Generates a circular app launcher UI with LVGL.

The user declares apps in YAML, and the component creates the LVGL UI automatically.
Supports multiple languages and custom fonts for accented characters.

Example:
  dial_menu:
    display_id: my_display
    language: fr  # French
    font_14: my_french_font  # Optional: for accented characters
    apps:
      - name: "Lumières"
        type: switch
        icon_type: light
        switches:
          - switch_id: my_switch
            name: "Salon"

For full examples, see:
  - dial-menu.yaml (complete)
  - dial-menu-simple.yaml (with packages)
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
from esphome.components import cover
from esphome.components import time as time_component
from esphome.components import font

CODEOWNERS = ["@antorfr"]
DEPENDENCIES = ["lvgl"]
AUTO_LOAD = []

# Reference to homeassistant_addon components (cover, climate, media_player)
# These are local components, so we reference them by namespace
homeassistant_addon_ns = cg.esphome_ns.namespace("homeassistant_addon")
HomeassistantCover = homeassistant_addon_ns.class_("HomeassistantCover", cg.Component)
HomeassistantClimate = homeassistant_addon_ns.class_("HomeassistantClimate", cg.Component)
HomeassistantMediaPlayer = homeassistant_addon_ns.class_("HomeassistantMediaPlayer", cg.Component)

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
CONF_SWITCHES = "switches"
CONF_COVER_ID = "cover_id"
CONF_COVERS = "covers"
CONF_CLIMATE_ID = "climate_id"
CONF_TEMPERATURE_STEP = "temperature_step"
CONF_MEDIA_PLAYER_ID = "media_player_id"
CONF_VOLUME_STEP = "volume_step"
CONF_IDLE_TIMEOUT = "idle_timeout"
CONF_TIME_ID = "time_id"
CONF_LANGUAGE = "language"
CONF_FONT_14 = "font_14"
CONF_FONT_18 = "font_18"

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
    "gate": "\uF52B",          # door-open (for gates/portals)
    "garage": "\uF52B",        # door-open (alias for garage doors)
    "blinds": "\uF8A0",        # blinds
    "window": "\uF8A0",        # blinds (alias for windows)
    "thermostat": "\uF2C9",    # thermometer (for climate/thermostat)
    "hvac": "\uF2C9",          # thermometer (alias for HVAC)
    "media_player": "\uF001",  # music (for media player)
    "speaker": "\uF028",       # volume-up (for speakers)
    "tv": "\uF26C",            # tv
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
CoverApp = dial_menu_ns.class_("CoverApp", DialApp)
ClimateApp = dial_menu_ns.class_("ClimateApp", DialApp)
MediaPlayerApp = dial_menu_ns.class_("MediaPlayerApp", DialApp)


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


# Schéma pour un switch individuel dans la liste
SWITCH_ITEM_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SWITCH_ID): cv.use_id(switch.Switch),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_COLOR): cv.hex_uint32_t,
    }
)

# Schéma pour un cover individuel dans la liste
COVER_ITEM_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_COVER_ID): cv.use_id(cover.Cover),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_COLOR): cv.hex_uint32_t,
    }
)

# Schéma pour une app - supports single switch_id or list of switches
APP_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DialApp),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_ICON_TYPE, default="none"): cv.one_of(*ICON_FONTAWESOME, lower=True),
        cv.Optional(CONF_COLOR): cv.hex_uint32_t,
        cv.Optional(CONF_TYPE, default="generic"): cv.string,
        cv.Optional(CONF_SWITCH_ID): cv.use_id(switch.Switch),
        cv.Optional(CONF_SWITCHES): cv.ensure_list(SWITCH_ITEM_SCHEMA),
        cv.Optional(CONF_COVER_ID): cv.use_id(cover.Cover),
        cv.Optional(CONF_COVERS): cv.ensure_list(COVER_ITEM_SCHEMA),
        cv.Optional(CONF_CLIMATE_ID): cv.use_id(HomeassistantClimate),
        cv.Optional(CONF_TEMPERATURE_STEP, default=0.5): cv.float_range(min=0.1, max=2.0),
        cv.Optional(CONF_MEDIA_PLAYER_ID): cv.use_id(HomeassistantMediaPlayer),
        cv.Optional(CONF_VOLUME_STEP, default=0.05): cv.float_range(min=0.01, max=0.2),
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
        cv.Optional(CONF_FONT_14): cv.use_id(font.Font),
        cv.Optional(CONF_FONT_18): cv.use_id(font.Font),
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
    
    # Get optional custom fonts
    font_14_var = None
    font_18_var = None
    if CONF_FONT_14 in config:
        font_14_var = await cg.get_variable(config[CONF_FONT_14])
    if CONF_FONT_18 in config:
        font_18_var = await cg.get_variable(config[CONF_FONT_18])
    
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
            
            # Pass custom font to SwitchApp
            if font_14_var is not None:
                cg.add(app_var.set_font_14(font_14_var))
            
            # Check for multiple switches first
            if CONF_SWITCHES in app_conf:
                for sw_conf in app_conf[CONF_SWITCHES]:
                    sw = await cg.get_variable(sw_conf[CONF_SWITCH_ID])
                    sw_name = sw_conf[CONF_NAME]
                    sw_color = sw_conf.get(CONF_COLOR, DEFAULT_COLORS[i % len(DEFAULT_COLORS)])
                    cg.add(app_var.add_switch(sw, sw_name, sw_color))
            # Fallback to single switch_id
            elif CONF_SWITCH_ID in app_conf:
                sw = await cg.get_variable(app_conf[CONF_SWITCH_ID])
                color = app_conf.get(CONF_COLOR, DEFAULT_COLORS[i % len(DEFAULT_COLORS)])
                cg.add(app_var.add_switch(sw, app_conf[CONF_NAME], color))
        elif app_type == "cover":
            # CoverApp for controlling covers (gates, blinds, etc.)
            cg.add_define("USE_DIAL_MENU_COVER")
            app_id.type = CoverApp
            app_var = cg.new_Pvariable(app_id)
            
            # Pass custom font to CoverApp
            if font_14_var is not None:
                cg.add(app_var.set_font_14(font_14_var))
            
            # Check for multiple covers first
            if CONF_COVERS in app_conf:
                for cv_conf in app_conf[CONF_COVERS]:
                    cv_entity = await cg.get_variable(cv_conf[CONF_COVER_ID])
                    cv_name = cv_conf[CONF_NAME]
                    cv_color = cv_conf.get(CONF_COLOR, DEFAULT_COLORS[i % len(DEFAULT_COLORS)])
                    cg.add(app_var.add_cover(cv_entity, cv_name, cv_color))
            # Fallback to single cover_id
            elif CONF_COVER_ID in app_conf:
                cv_entity = await cg.get_variable(app_conf[CONF_COVER_ID])
                color = app_conf.get(CONF_COLOR, DEFAULT_COLORS[i % len(DEFAULT_COLORS)])
                cg.add(app_var.add_cover(cv_entity, app_conf[CONF_NAME], color))
        elif app_type == "climate":
            # ClimateApp for controlling a single thermostat
            cg.add_define("USE_DIAL_MENU_CLIMATE")
            app_id.type = ClimateApp
            app_var = cg.new_Pvariable(app_id)
            
            # Pass custom fonts to ClimateApp
            if font_14_var is not None:
                cg.add(app_var.set_font_14(font_14_var))
            if font_18_var is not None:
                cg.add(app_var.set_font_18(font_18_var))
            
            # Set climate entity
            if CONF_CLIMATE_ID in app_conf:
                climate_entity = await cg.get_variable(app_conf[CONF_CLIMATE_ID])
                cg.add(app_var.set_climate(climate_entity))
            
            # Set temperature step
            temp_step = app_conf.get(CONF_TEMPERATURE_STEP, 0.5)
            cg.add(app_var.set_temperature_step(temp_step))
        elif app_type == "media_player":
            # MediaPlayerApp for controlling a single media player
            cg.add_define("USE_DIAL_MENU_MEDIA_PLAYER")
            app_id.type = MediaPlayerApp
            app_var = cg.new_Pvariable(app_id)
            
            # Set controller reference for language support
            cg.add(app_var.set_controller(var))
            
            # Pass custom fonts to MediaPlayerApp
            if font_14_var is not None:
                cg.add(app_var.set_font_14(font_14_var))
            if font_18_var is not None:
                cg.add(app_var.set_font_18(font_18_var))
            
            # Set media player entity
            if CONF_MEDIA_PLAYER_ID in app_conf:
                mp_entity = await cg.get_variable(app_conf[CONF_MEDIA_PLAYER_ID])
                cg.add(app_var.set_media_player(mp_entity))
            
            # Set volume step
            vol_step = app_conf.get(CONF_VOLUME_STEP, 0.05)
            cg.add(app_var.set_volume_step(vol_step))
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

    # Custom fonts for French accents support
    if CONF_FONT_14 in config:
        font_14_var = await cg.get_variable(config[CONF_FONT_14])
        cg.add(var.set_font_14(font_14_var))
    
    if CONF_FONT_18 in config:
        font_18_var = await cg.get_variable(config[CONF_FONT_18])
        cg.add(var.set_font_18(font_18_var))
