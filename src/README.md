# Wayterra

Wayterra is an extension of tinywl built on top of wlroots.

---

## Building Wayterra

Wayterra depends on the following:

- wlroots (0.20)
- Wayland (wayland-server)
- libxkbcommon
- libdrm
- meson
- ninja

### Install Dependencies

#### Debian / Ubuntu

```bash
sudo pacman -S meson ninja \
  wlroots \
  wayland \
  libxkbcommon \
  libdrm
```

---

## Build
```bash
meson setup build
meson compile -C build
```


## Keybinds

- Alt+Escape  : Terminate the compositor
- Alt+F1      : Cycle between windows
- Alt+Return  : Spawn alacritty terminal window (alacritty required)
- Alt+Tab     : Toggle WASD movement