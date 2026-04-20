#include "output.h"
#include "input.h"
#include "renderer.h"
#include "server.h"
#include <stdbool.h>
#include <stdlib.h>
#include <wlr/types/wlr_scene.h>


// TODO use scene graph and render background as scene node?
void server_new_output(struct wl_listener *listener, void *data) {
    /* Event raised by the backend when a new output is available */
    wayterra_server_t *server =
        wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    /* Configure output to use our allocator and renderer */
    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_output_state_set_mode(&state, mode);
    }

    
    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    /* Allocate per-output state */
    wayterra_output_t *output = calloc(1, sizeof(*output));
    output->wlr_output = wlr_output;
    output->server = server;

    /* Allocate renderer for this output */
    output->renderer = calloc(1, sizeof(*output->renderer));

    /* Optionally: initialize vertices array to zero (calloc already does this) */
    output->renderer->shader_initialized = false;

    /* Frame listener (for rendering) */
    wl_list_init(&output->frame.link);
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    // FIXME! FIXME! to allow resizing and unplug to not crash 
    //
    // /* State request listener */
    // output->request_state.notify = output_request_state;
    // wl_signal_add(&wlr_output->events.request_state, &output->request_state);
    //
    // /* Destroy listener */
    // output->destroy.notify = output_destroy;
    // wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&server->outputs, &output->link);

    /* Add to output layout */
    struct wlr_output_layout_output *l_output =
        wlr_output_layout_add_auto(server->output_layout, wlr_output);

    struct wlr_scene_output *scene_output =
        wlr_scene_output_create(server->scene, wlr_output);

    wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);
}





void output_frame(struct wl_listener *listener, void *data) {
	/* This function is called every time an output is ready to display a frame,
	 * generally at the output's refresh rate (e.g. 60Hz). */

    wayterra_output_t *output =
        wl_container_of(listener, output, frame);

	struct wlr_scene *scene = output->server->scene;

	struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(
		scene, output->wlr_output);

    // add background scene node as texture???

	/* Render the scene if needed and commit the output */
	wlr_scene_output_commit(scene_output, NULL);

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output, &now);
}




// void output_frame(struct wl_listener *listener, void *data) {
//     (void)data;
//     wayterra_output_t *output =
//         wl_container_of(listener, output, frame);
//
//     struct wlr_output *wlr_output = output->wlr_output;
//     wayterra_renderer_t *r = output->renderer;
//
//     struct wlr_output_state state;
//     wlr_output_state_init(&state);
//
//     struct wlr_render_pass *pass =
//         wlr_output_begin_render_pass(wlr_output, &state, NULL);
//
//     if (!pass) {
//         wlr_output_state_finish(&state);
//         return;
//     }
//
//     if (!r->shader_initialized) {
//         initialize_renderer(r);
//     }
//
//     int width, height;
//     wlr_output_effective_resolution(wlr_output, &width, &height);
//
//     glViewport(0, 0, width, height);
//
//     // TODO Add update function for physics?
//     renderer_draw_frame(r, width, height);
//
//     wlr_render_pass_submit(pass);
//     wlr_output_commit_state(wlr_output, &state);
//     wlr_output_state_finish(&state);
// }
