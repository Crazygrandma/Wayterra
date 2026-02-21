# Wayterra

A wayland compositor that tries to render a terraria like game world as the background and have clients be rendered in the world. 
The player can move around and interact with world objects that can launch other clients

# Motivation

Memorising keyboard shortcuts or nested menus seems the trivial way to interact with a desktop, however being able to spread out applications visually could help and introduces a new design principle that has not been explored to the fullest extend and the more modern hardware.

# Development Progress

Before actually developing the main aspects of the compositor, learning the fundamentals of the wayland protocol and being able to interact with wlrootsapi using C is necessary. 

As I have never build anything with C, this is a very time consuming and hard project to accomplish.

The status of the wayterra compositor is currently just some additional keybinds and movement logic of the wayland clients on top of tinywl.
Additionally the background is being rendered with a predefined background image.

This already allows the user to have an infinite canvas (technically not really) where the user can navigate around using the WASD keys.
The user can also spawn an alacritty window using Alt+Return.

With Alt+Tab the user can toggle the movement mode, so that interacting with a terminal works as usual, so that pressing the movement keys dont accidentally move the clients.

Since its being built on top of tinywl, all features tinywl provides are also possible. Such as resizing, moving and obviously input handling and changing focus of the window when clicking with the mouse.

At this state wayterra doesnt allow some interfaces that would allow clients like, waybar or rofi to function. 
Additionally applications that dont work natively on wayland would require xwayland which is not implemented as well.


# Next Steps

Next up is obviously learn the entire wayland pipeline and how rendering works since the goal is to be able to 
render a terraria like game in the background.

My goal is to build the project in such a way that the underlying game can be exchanged easily. 
That way Wayterra could provide a template for other game implemenations.



