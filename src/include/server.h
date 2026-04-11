#ifndef SERVER_H
#define SERVER_H


#include <wayland-server-core.h>

typedef struct wayterra_renderer wayterra_renderer_t;

struct wlr_backend;
struct wlr_renderer;
struct wlr_allocator;
struct wlr_output;
struct wlr_output_layout;
struct wlr_scene;
struct wlr_scene_output_layout;


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

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;

    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    struct wl_listener new_output;


    // INPUT
    struct wlr_seat *seat;
    struct wl_listener new_input;

    struct wl_list keyboards; // wayterra_keyboard_t::link

} wayterra_server_t;


void setup(wayterra_server_t *server);
void run(wayterra_server_t *server);
void cleanup(wayterra_server_t *server);

#endif
