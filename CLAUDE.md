# ESPHome Dial Library - Project Context

> **Ce fichier sert de référence pour les LLMs travaillant sur ce projet.**

---

## 🎯 Vision

Créer une **librairie ESPHome** pour la gestion d'écrans circulaires avec molette (rotary encoder), utilisant **LVGL** pour un rendu fluide et des animations natives.

---

## 🔧 Hardware : M5Stack Dial

| Composant | Specs | GPIOs |
|-----------|-------|-------|
| **MCU** | ESP32-S3 | - |
| **Display** | GC9A01 240x240 SPI | MOSI=5, CLK=6, CS=7, DC=4, RST=8, BL=9 |
| **Encoder** | Rotatif + bouton | A=40, B=41, BTN=42 |
| **Touch** | FT3267 I2C | SDA=11, SCL=12, INT=14 |

---

## 📁 Structure du Projet

```
esphome-dial/
├── dial-menu.yaml              # Config principale
├── secrets.yaml                # WiFi, etc.
├── components/
│   └── dial_menu/              # Composant ESPHome
│       ├── __init__.py         # Config YAML + codegen
│       ├── dial_menu_controller.h
│       └── dial_menu_controller.cpp
└── M5Dial-UserDemo/            # Firmware de référence M5Stack
```

---

## 🏗️ Architecture

### Composant `dial_menu`

**Role** : Gère la logique métier ET crée l'UI LVGL automatiquement

**Configuration YAML simplifiée** :
```yaml
dial_menu:
  id: menu_controller
  display_id: round_display
  apps:
    - name: "Settings"
      icon_type: settings
      color: 0xFD5C4C
    - name: "WiFi"
      icon_type: wifi
      color: 0x577EFF
```

**C++ Classes** :
- `DialMenuController` : Crée l'UI LVGL, gère navigation et apps
- `DialApp` : Représente une app avec nom, icône, couleur, position, référence LVGL

### Génération automatique LVGL

Le composant `dial_menu` **crée tous les widgets LVGL en C++** :
- Boutons circulaires disposés automatiquement en cercle
- Centre avec label du nom de l'app
- Gestion focus/defocus avec changement de taille
- Événements click, focus, defocus

L'utilisateur n'a **plus besoin d'écrire de YAML LVGL** pour les widgets !

---

## 🎨 UI Générée

### Launcher
- N boutons circulaires disposés en cercle (rayon configurable, défaut 85px)
- Chaque bouton : 50x50 → 58x58 au focus (configurable)
- Première lettre du nom comme icône (fallback, FontAwesome TODO)
- Label central avec nom de l'app sélectionnée
- Focus : bordure blanche + shadow colorée + taille augmentée

### Interactions
| Input | Action |
|-------|--------|
| Rotation encoder | Navigation entre apps |
| Click encoder | Ouvrir app (TODO) |
| Touch | Navigation + sélection (via LVGL) |

---

## 📝 Icônes FontAwesome Disponibles

```python
ICON_FONTAWESOME = {
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
    "lock": "\uF023",          # lock
    "play": "\uF04B",          # play
    "pause": "\uF04C",         # pause
    "info": "\uF129",          # info
}
```

---

## 🚀 Commandes

```bash
# Activer l'environnement
cd /Users/berard/Dev/esphome-dial
source venv/bin/activate

# Compiler
esphome compile dial-menu.yaml

# Flasher
esphome upload dial-menu.yaml --device /dev/cu.usbmodem101

# Logs
esphome logs dial-menu.yaml --device /dev/cu.usbmodem101
```

---

## 🔄 Progress

### ✅ Done
- [x] Structure composant ESPHome dial_menu
- [x] Navigation encoder fonctionnelle
- [x] Touch support via LVGL
- [x] Effet visuel au focus (taille + glow)
- [x] Nettoyage architecture (suppression ancien composant)
- [x] **Génération automatique UI LVGL** (de 532 à 138 lignes YAML!)

### 🚧 TODO
- [ ] Ajouter police FontAwesome pour les icônes
- [ ] Ouvrir une app au click (navigation entre pages)
- [ ] App Brightness (slider circulaire pour backlight)
- [ ] Intégration Home Assistant (API + services)
- [ ] Retour au launcher depuis une app

---

## 📚 Références

- [ESPHome LVGL](https://esphome.io/components/lvgl/index.html)
- [M5Stack Dial Product Page](https://docs.m5stack.com/en/core/M5Dial)

---

*Dernière mise à jour : 2026-02-03*
