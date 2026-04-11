#include "server.h"
#include "error.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
	/* Wayland requires XDG_RUNTIME_DIR for creating its communications socket */
	if (!getenv("XDG_RUNTIME_DIR"))
		die("XDG_RUNTIME_DIR must be set");

    wayterra_server_t server = {0};
    
    setup(&server);
    run(&server);
    cleanup(&server);
    return 0;
}
