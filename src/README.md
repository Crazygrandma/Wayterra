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

#### Arch

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
ninja -C build
```


## Keybinds
You can change the keybinds in the `include/config.h` file
Documenation and parser will come later.
For now you have to rebuild everytime you change the keybind
- Alt+Escape  : Terminate the compositor
- Alt+F1      : Cycle between windows
- Alt+Return  : Spawn alacritty terminal window (alacritty required)
- Alt+Tab     : Toggle WASD movement
