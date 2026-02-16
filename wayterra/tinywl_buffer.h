#ifndef TINYWL_BUFFER_H
#define TINYWL_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <wlr/interfaces/wlr_buffer.h>

struct tinywl_solid_buffer {
	struct wlr_buffer base;
	uint32_t format;
	size_t stride;
	uint8_t *data;
};

#endif
