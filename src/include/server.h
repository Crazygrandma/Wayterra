#ifndef SERVER_H
#define SERVER_H


#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_scene.h>

typedef struct wayterra_renderer wayterra_renderer_t;


typedef struct wayterra_output {
    struct wayterra_server *server;
    struct wlr_output *wlr_output;

    struct wl_listener frame;
    struct wl_list link;

    wayterra_renderer_t *renderer;
} wayterra_output_t;

typedef struct wayterra_server {
    struct wl_display *wl_display;
    struct wl_event_loop *event_loop;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;
    

	struct wlr_xdg_shell *xdg_shell;
	struct wl_listener new_xdg_toplevel;
	struct wl_listener new_xdg_popup;
	struct wl_list toplevels;
    
    // INPUT
    struct wlr_seat *seat;
    struct wl_listener new_input;

    struct wl_list keyboards; // wayterra_keyboard_t::link
    

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
 
    struct wl_listener new_output;



} wayterra_server_t;


struct wayterra_toplevel {
	struct wl_list link;
	wayterra_server_t *server;
	struct wlr_xdg_toplevel *xdg_toplevel;
	struct wlr_scene_tree *scene_tree;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;
	struct wl_listener request_move;
	struct wl_listener request_resize;
	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;
};

void setup(wayterra_server_t *server);
void run(wayterra_server_t *server);
void cleanup(wayterra_server_t *server);

#endif
