#include "server.h"
#include "error.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
	/* Wayland requires XDG_RUNTIME_DIR for creating its communications socket */
	if (!getenv("XDG_RUNTIME_DIR"))
		die("XDG_RUNTIME_DIR must be set");

    wayterra_server_t server = {0};
   

    // Setup the structs and listeners
    setup(&server);
    // Run the event loop
    run(&server);
    // Cleanup memory we allocated
    cleanup(&server);
    return 0;
}
