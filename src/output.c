#include "output.h"
#include "renderer.h"
#include "input.h"
#include "server.h"
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <stdbool.h>
#include <stdlib.h>
#include <error.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/render/allocator.h>

#include <wlr/util/log.h>
#include <drm_fourcc.h>
#include <inttypes.h>



/*
 * wlroots only makes its EGL context current *inside* its own calls, and
 * restores the previous state (normally: no context) before returning.
 * Anything we draw ourselves has to do the same bracketing, otherwise every
 * gl* call is a silent no-op.
 */
static struct wlr_egl *output_egl(wayterra_server_t *server) {
    return wlr_gles2_renderer_get_egl(server->renderer);
}

static bool output_gl_begin(wayterra_server_t *server, EGLContext *prev_ctx) {
    struct wlr_egl *egl = output_egl(server);

    *prev_ctx = eglGetCurrentContext();
    if (*prev_ctx == wlr_egl_get_context(egl)) {
        return true;
    }

    return eglMakeCurrent(wlr_egl_get_display(egl),
            EGL_NO_SURFACE, EGL_NO_SURFACE, wlr_egl_get_context(egl));
}

static void output_gl_end(wayterra_server_t *server, EGLContext prev_ctx) {
    eglMakeCurrent(wlr_egl_get_display(output_egl(server)),
            EGL_NO_SURFACE, EGL_NO_SURFACE, prev_ctx);
}

/*
 * The backend requests a new state when the resolution changes. On the
 * Wayland backend this happens every time the nested window is resized:
 * the parent compositor sends a configure, and wlroots asks us to commit
 * the new mode. Until we do, wlr_output->width/height keep the old size.
 */
static void output_request_state(struct wl_listener *listener, void *data) {
    wayterra_output_t *output =
        wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;

    if (!wlr_output_commit_state(output->wlr_output, event->state)) {
        wlr_log(WLR_ERROR, "Failed to commit requested output state");
    }
}

static void output_destroy(struct wl_listener *listener, void *data) {
    wayterra_output_t *output =
        wl_container_of(listener, output, destroy);

    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);

    if (output->game_scene_buffer != NULL) {
        wlr_scene_node_destroy(&output->game_scene_buffer->node);
        output->game_scene_buffer = NULL;
    }

    if (output->gameframebuffer != NULL) {
        wlr_buffer_drop(output->gameframebuffer);
        output->gameframebuffer = NULL;
    }

    free(output);
}

static const struct wlr_drm_format *output_pick_format(wayterra_server_t *server) {
    const struct wlr_drm_format_set *formats =
        wlr_renderer_get_texture_formats(
                server->renderer,
                server->allocator->buffer_caps
                );

    if (formats == NULL) {
        wlr_log(WLR_ERROR, "No renderer texture formats");
        return NULL;
    }

    const struct wlr_drm_format *format =
        wlr_drm_format_set_get(formats, DRM_FORMAT_XRGB8888);

    if (format == NULL) {
        wlr_log(WLR_ERROR, "XRGB8888 is not supported");
        return NULL;
    }

    return format;
}

/*
 * Keep the game buffer at the output's current resolution.
 *
 * The output's size is wlr_output->width / wlr_output->height - the same
 * values wlroots uses to size its own swapchain buffers. When they stop
 * matching our buffer (window resized, mode changed), allocate a new one
 * and hand it to the scene node; the scene releases the old one.
 */
static bool output_ensure_game_buffer(wayterra_output_t *output) {
    struct wlr_output *wlr_output = output->wlr_output;

    int width = wlr_output->width;
    int height = wlr_output->height;

    if (width <= 0 || height <= 0) {
        return false;
    }

    if (output->gameframebuffer != NULL &&
            output->gameframebuffer->width == width &&
            output->gameframebuffer->height == height) {
        return true;
    }

    const struct wlr_drm_format *format = output_pick_format(output->server);
    if (format == NULL) {
        return false;
    }

    struct wlr_buffer *new_buffer =
        wlr_allocator_create_buffer(output->server->allocator,
                width, height, format);

    if (new_buffer == NULL) {
        wlr_log(WLR_ERROR, "Failed to allocate %dx%d game buffer",
                width, height);
        return false;
    }

    /* Scene takes a lock on the new buffer and releases the old one
     * together with its cached texture. On the very first call the scene
     * node does not exist yet - server_new_output() creates it after this. */
    if (output->game_scene_buffer != NULL) {
        wlr_scene_buffer_set_buffer(output->game_scene_buffer, new_buffer);
    }

    if (output->gameframebuffer != NULL) {
        wlr_buffer_drop(output->gameframebuffer);
    }
    output->gameframebuffer = new_buffer;

    wlr_log(WLR_INFO, "Game buffer now %dx%d", width, height);
    return true;
}

/* Cover exactly this output, wherever it sits in the layout */
static void output_layout_to_scene(wayterra_output_t *output) {
    if (output->game_scene_buffer == NULL) {
        return;
    }

    struct wlr_box box;
    wlr_output_layout_get_box(output->server->output_layout,
            output->wlr_output, &box);

    if (box.width <= 0 || box.height <= 0) {
        return;
    }

    wlr_scene_node_set_position(&output->game_scene_buffer->node,
            box.x, box.y);
    wlr_scene_buffer_set_dest_size(output->game_scene_buffer,
            box.width, box.height);
}

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

    output->renderer = calloc(1, sizeof(*output->renderer));

    /* Frame listener (for rendering) */
    wl_list_init(&output->frame.link);
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    /* State request listener (the backend asks for a new mode on resize) */
    output->request_state.notify = output_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    /* Destroy listener */
    output->destroy.notify = output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&server->outputs, &output->link);

    /* Add to output layout */
    struct wlr_output_layout_output *l_output =
        wlr_output_layout_add_auto(server->output_layout, wlr_output);

    struct wlr_scene_output *scene_output =
        wlr_scene_output_create(server->scene, wlr_output);

    wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);
   

    // Setup game frame rendering

    const struct wlr_drm_format *format = output_pick_format(server);

    if (format == NULL) {
        return;
    }

    wlr_log(WLR_INFO,
            "Using XRGB8888 with %zu modifiers",
            format->len);

    for (size_t j = 0; j < format->len; j++) {
        wlr_log(WLR_INFO,
                "modifier: 0x%" PRIx64,
                format->modifiers[j]);
    }

    // 1. Allocate a GPU buffer matching the output's current resolution
    if (!output_ensure_game_buffer(output)) {
        wlr_log(WLR_ERROR, "Failed to allocate game buffer");
        return;
    }

    // 2. Put the buffer into the scene
    output->game_scene_buffer =
        wlr_scene_buffer_create(
                &server->scene->tree,
                output->gameframebuffer
                );

    if (output->game_scene_buffer == NULL) {
        wlr_log(WLR_ERROR, "Failed to create scene buffer");
        return;
    }
    
    wlr_scene_node_lower_to_bottom(&output->game_scene_buffer->node);
    output_layout_to_scene(output);

    /* No GL here: the context is not current at this point (and never is
     * outside wlroots' own calls). All drawing happens in output_frame(),
     * where we borrow the context first. The FBO is created lazily and
     * re-fetched every frame by wlr_gles2_renderer_get_buffer_fbo(). */
}

void output_frame(struct wl_listener *listener, void *data) {
    wayterra_output_t *output =
        wl_container_of(listener, output, frame);

    struct wayterra_server *server = output->server;

    /*
     * ------------------------------------------------------------
     * 0. Keep the game buffer at the output's current resolution
     *
     * wlr_output->width / ->height are the pixels the output is
     * actually driven at (the same values wlroots sizes its own
     * swapchain buffers to). They change when the backend requests a
     * new state - on the Wayland backend, when the nested window is
     * resized. This reallocates only when they differ.
     * ------------------------------------------------------------
     */
    output_ensure_game_buffer(output);
    output_layout_to_scene(output);

    /*
     * ------------------------------------------------------------
     * 1. Borrow wlroots' EGL context for our own drawing
     *
     * Outside of wlroots' own calls no context is current, so every
     * gl* call below would be a no-op without this.
     * ------------------------------------------------------------
     */
    EGLContext prev_ctx;
    if (!output_gl_begin(server, &prev_ctx)) {
        wlr_log(WLR_ERROR, "Failed to make EGL context current");
        return;
    }

    struct wlr_buffer *game = output->gameframebuffer;
    struct wayterra_renderer *r = output->renderer;

    if (game != NULL) {
        r->fbo = wlr_gles2_renderer_get_buffer_fbo(server->renderer, game);
    }

    if (game == NULL || r->fbo == 0) {
        wlr_log(WLR_ERROR, "Failed to get game framebuffer FBO");
    } else {
        /* Discard stale errors left behind by wlroots' own GL work */
        while (glGetError() != GL_NO_ERROR) {
            ;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, r->fbo);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            wlr_log(WLR_ERROR, "Game FBO is incomplete: 0x%x", status);
        } else {

            /* Fullscreen: viewport = the whole game buffer */
            renderer_draw_frame(output->renderer, game->width, game->height);
        }

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            wlr_log(WLR_ERROR, "Game rendering failed: 0x%x", err);
        }

        glFlush();
    }

    /*
     * ------------------------------------------------------------
     * 3. Tell the scene that the buffer contents changed
     *
     * wlroots only re-renders damaged regions, and it cannot know we
     * wrote into the buffer behind its back. This damages the whole
     * node and drops the cached texture (so a CPU/shm buffer is
     * re-uploaded from our new pixels). NULL damage = whole buffer.
     * It also schedules the next frame event, which keeps the game
     * loop running.
     * ------------------------------------------------------------
     */
    if (output->game_scene_buffer != NULL && output->gameframebuffer != NULL) {
        wlr_scene_buffer_set_buffer_with_damage(
            output->game_scene_buffer,
            output->gameframebuffer,
            NULL
        );
    }

    /* Hand the context back before wlroots runs its own render pass */
    output_gl_end(server, prev_ctx);

    /*
     * ------------------------------------------------------------
     * 4. Composite the scene and report the frame done
     *
     * This takes our game framebuffer, client surfaces, etc. and
     * renders them into the actual output buffer.
     *
     * This block must not be skipped: without a commit nothing is
     * presented and no new frame events get scheduled, so the screen
     * freezes. Errors are logged instead of bailing out.
     * ------------------------------------------------------------
     */
    struct wlr_scene_output *scene_output =
        wlr_scene_get_scene_output(
            server->scene,
            output->wlr_output
        );

    if (scene_output == NULL) {
        wlr_log(WLR_ERROR, "Failed to get scene output");
        return;
    }

    if (!wlr_scene_output_commit(scene_output, NULL)) {
        wlr_log(WLR_ERROR, "Scene output commit failed");
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    wlr_scene_output_send_frame_done(
        scene_output,
        &now
    );
}
