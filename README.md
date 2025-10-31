# **Wayterra**
## **Overview**

**Wayterra** is an experimental **Wayfire compositor plugin** that transforms the traditional desktop environment into a **living, navigable 2D sandbox world** inspired by *Terraria*.
Instead of rendering static wallpapers and floating windows, Wayterra integrates Wayland clients **directly into the world**, turning applications into spatially anchored, interactive elements within a persistent scene.
By blending the flexibility of **Wayfire’s low-level scene graph** with the creativity of a **sandbox game**, Wayterra reimagines the desktop as a space to **explore, build, and play**—while still serving as a fully functional and customizable operating environment.


## **Core Technologies**
- **Wayfire Compositor (C++ / Wayland)** —
Provides the rendering infrastructure and plugin architecture for low-level control over Wayland surfaces.
- **Meson + Ninja Build System** —
Ensures fast, reproducible builds and dependency management.
- **wf-config / wlroots Integration** —
Manages Wayland surface composition, input, and system configuration hooks.
- **OpenGL / GLES Rendering** —
Enables real-time 2D world rendering, lighting, and compositing of Wayland clients within the game world.

## **Key Features**
### 🪐 **World-Integrated Applications**
Applications still act like normal wayland clients and are rendered as floating windows, however the position of these clients is controlled by Wayterra — they’re **anchored entities inside the game world**.
Each Wayland client occupies a fixed position within the environment, moving relative to the player’s viewport and maintaining spatial coherence.
This gives the illusion that applications are **baked into the terrain** — part of the physical world, not merely drawn on top of it. (even though they are)
Wayterra also has the ability to change window decorations to make them fit better into the world. (Vines for a border, overlapping terraria blocks)


### 🎮 **Dual Navigation System**
Wayterra offers two modes of interaction:
- **World Navigation Mode** —
Players move their in-game character through the world to reach **special objects** or **buildings** that represent applications.
For example, entering a tower might open the terminal, while visiting a library launches the browser.
- **Inventory-Based App Launcher** —
A *Terraria-style inventory menu* acts as a **hybrid app launcher**, giving users a more familiar interface similar to **Rofi** or **GNOME’s app grid**.
Applications can be launched directly from collected items, or optionally require players to **discover** those items in the world before they can be used.
This system bridges the gap between **playful exploration** and **traditional desktop usability**, making Wayterra approachable for both gamers and productivity-focused users.

### 🏗️ **Prebuilt World and Sandbox Customization Tools**
The default Wayterra configuration ships with a **handcrafted starting world**, featuring **themed architecture** that houses the essential applications needed for daily use.
This ensures the user can start exploring and working immediately without having to “find” core tools first.
- A **wizard tower** might house the terminal.
- A **library** could represent the web browser.
- A **basement workshop** could contain system settings like display or network configuration.
All key applications are placed within **walking distance**, providing a cohesive, explorable environment right from the first launch.
Beyond the starter world, players can:
- **Place and remove blocks** to build custom structures and layouts.
- **Spawn new app buildings** or decorative elements.
- Enable **flying mode** for creative building and fast customization.


### 🧩 **IPC and Command System**
For advanced users, Wayterra exposes a flexible **IPC-based command system** that allows external scripts and internal “cheat-style” commands to manipulate the world or interact with the compositor.
Commands follow a simple, game-inspired syntax such as:
/give item_name 64
Under the hood, these commands communicate with the **Wayfire compositor** and the **operating system** to:
- Spawn in-game items or blocks.
- Launch applications.
- Run scripts or system actions.
- Generate new in-world app entities automatically from desktop entries (similar to Rofi).
This design keeps the **power-user tools approachable**, while also enabling deep automation and extensibility for users who want to craft a truly personalized environment.


## **Vision**
**Wayterra** is more than a window manager plugin — it’s a reimagining of what a desktop environment can be.
It explores how **spatial interaction**, **playful design**, and **customization** can make digital workspaces more creative, memorable, and personally expressive.
By allowing users to both *live in* and *build* their own computing world, Wayterra blurs the boundary between **operating system**, **sandbox game**, and **artistic medium**. 
