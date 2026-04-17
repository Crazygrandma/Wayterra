# Wayterra

A wayland compositor that tries to render a terraria like game world as the background and have clients be rendered in the world. 
The player can move around and interact with world objects that can launch other clients

# Motivation

Memorising keyboard shortcuts or nested menus seems the trivial way to interact with a desktop, however being able to spread out applications visually could help and introduces a new design principle that has not been explored to the fullest extend and the more modern hardware.

# Development Progress

At this state the compositor is now only implementing rendering a background texture and a player texture using OpenGL ES 2.0 with a shader.
It also implements input handling with keybinds such as SUPER+Tab which toggles the movement of the player so that you can move the player using WASD.
Closing the compositor is done with SUPER+Esc.
You can also rebind the modifier key as well the player speed by changing the values in the config.h under `include/config.h`

# Next Steps

Reimplememnt client rendering using a scene graph. Since tinywl already show how this is done. It should be fairly easy to integrate. However the rendering logic might need to be reworked.

Additionally some helper functions and more refactoring should be done to logically separate the modules of the compositor.
So that the compositors background rendering works independently from the game logic. 
The goal here is that the renderer provides an interface for rendering tile based worlds but the game logic can be replaced entirely.

Additionally building a basic tilemap editor with SDL should help when designing the world so that artists can help designing the world without needing to manually replace and encode the tilemap textures.

