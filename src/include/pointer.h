#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_cursor.h>

typedef struct wayterra_server wayterra_server_t;

void server_new_pointer(wayterra_server_t *server, struct wlr_input_device *device);


void server_cursor_button(struct wl_listener *listener, void *data);
void server_cursor_motion_absolute(struct wl_listener *listener, void *data);
void server_cursor_motion(struct wl_listener *listener, void *data);


void process_cursor_motion(wayterra_server_t *server, uint32_t time);
void process_cursor_resize(wayterra_server_t *server);
