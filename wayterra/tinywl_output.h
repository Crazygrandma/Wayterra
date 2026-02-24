#ifndef TINYWL_OUTPUT_H
#define TINYWL_OUTPUT_H

#include <stdbool.h>
#include <GLES2/gl2.h>
#include <wayland-util.h>

#include <wlr/render/swapchain.h>

struct tinywl_server;
struct wlr_output;
struct wlr_scene_buffer;

struct tinywl_output {
	struct wl_list link;
	struct tinywl_server *server;
	struct wlr_output *wlr_output;
	struct wlr_scene_buffer *blue_background;
	struct wlr_swapchain *background_swapchain;
	int background_width;
	int background_height;
	bool background_gl_ready;
	GLuint background_program;
	GLuint background_vbo;
	GLint background_pos_loc;
	GLint background_time_loc;
	struct wl_listener frame;
	struct wl_listener request_state;
	struct wl_listener destroy;
};

#endif
