#include "output.h"
#include "input.h"
#include "renderer.h"
#include "server.h"
#include <wlr/render/egl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wlr/types/wlr_scene.h>

void output_init_fbo(wayterra_output_t *output, int width, int height) {
    output->width = width;
    output->height = height;

    // 1. Create texture
    glGenTextures(1, &output->color_tex);
    glBindTexture(GL_TEXTURE_2D, output->color_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // (optional but good)
    glBindTexture(GL_TEXTURE_2D, 0);

    // 2. Create framebuffer
    glGenFramebuffers(1, &output->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, output->fbo);

    // 3. Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           output->color_tex,
                           0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FBO error: 0x%x\n", status);
    }

    // 5. Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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

void output_render_to_fbo(wayterra_output_t *output) {
    // 1. Bind FBO (VERY IMPORTANT: first step)
    glBindFramebuffer(GL_FRAMEBUFFER, output->fbo);

    // 2. Set viewport to match texture
    glViewport(0, 0, output->width, output->height);

    // 3. (optional but recommended)
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // 4. Draw your shader
    renderer_draw_frame(output->renderer,
                        output->width,
                        output->height);

    // 5. Unbind (restore default framebuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void output_frame(struct wl_listener *listener, void *data) {
    wayterra_output_t *output =
        wl_container_of(listener, output, frame);

    struct wlr_scene *scene = output->server->scene;

    struct wlr_scene_output *scene_output =
        wlr_scene_get_scene_output(scene, output->wlr_output);

    int width, height;
    wlr_output_effective_resolution(output->wlr_output, &width, &height);

    /* 1. Lazy init (IMPORTANT: GL context is valid here) */
    if (!output->renderer->shader_initialized) {
        initialize_renderer(output->renderer);
    }

    if (!output->fbo_initialized) {
        output_init_fbo(output, width, height);
        output->fbo_initialized = true;
    }


    // TODO Figure out egl context
    // When do i have gl context
    printf("GL context = %p\n", wlr_egl_get_context());
    printf("GL display = %p\n", wlr_egl_gdisplaylay());
    /* 2. Render into FBO */
    output_render_to_fbo(output);

    /* 3. Scene graph still renders whatever is already there */
    wlr_scene_output_commit(scene_output, NULL);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}
