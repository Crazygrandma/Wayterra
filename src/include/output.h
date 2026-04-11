
#ifndef OUTPUT_H
#define OUTPUT_H

#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

struct wl_listener;

void server_new_output(struct wl_listener *listener, void *data);
void output_frame(struct wl_listener *listener, void *data);
#endif
