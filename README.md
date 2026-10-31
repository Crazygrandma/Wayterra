# Wayterra

A wayland compositor that tries to render a terraria like game world as the background and have clients be rendered in the world. 
The player can move around and interact with world objects that can launch other clients

Built with C, wlroots 0.20 and OpenGL ES.

## Dependencies

Wayterra requires the following packages on Arch Linux / CachyOS:

```bash
sudo pacman -S --needed \
    base-devel \
    meson \
    ninja \
    pkgconf \
    wlroots0.20 \
    wayland \
    libxkbcommon \
    libdrm \
    libglvnd \
    mesa \
    cglm \
    alacritty \
    firefox \
    obs-studio
```

The following applications are used by Wayterra's default keybindings:

- Alacritty – Terminal
- Firefox – Web browser
- OBS Studio – Screen recording

Make sure all three applications are installed before using their respective keybindings.

## Building

Clone the repository:

```bash
git clone git@github.com:Crazygrandma/Wayterra.git
cd wayterra
```

Initialize the build directory:

```bash
make setup
```

Build Wayterra:

```bash
make build
```

Alternatively, build and run directly:

```bash
make run
```

## Installation

Install Wayterra as a system-wide Wayland session:

```bash
sudo make install
```

This installs the executable to `/usr/local/bin/wayterra` and the desktop session entry to `/usr/local/share/wayland-sessions/wayterra.desktop`.

You can then select Wayterra from your display manager.

To uninstall:

```bash
sudo make uninstall
```

## Development

Run Wayterra directly from the project:

```bash
make run
```

Capture debug output:

```bash
./build/wayterra 2>&1 | tee ~/wayterra.log
```

Clean the build directory:

```bash
make clean
```

**Note:** Shader and asset paths currently use relative paths. Run Wayterra from the project root when developing.

## Keybindings

| Keybinding | Action |
|---|---|
| `Alt + Enter` | Spawn Alacritty terminal |
| `Alt + B` | Spawn Firefox |
| `Alt + R` | Spawn OBS Studio |
| `Alt + Q` | Kill a focused client (make sure to click on it first) |
| `Alt + Esc` | Quit wayterra |

If no client is focused you can move around with WASD
