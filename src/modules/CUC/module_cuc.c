#include <unistd.h>
#include <signal.h>

#include "../../common.h"
#include "../../events_definitions.h"
#include "module_cuc.h"

int rc;
volatile sig_atomic_t is_running = 1;

TSN_Module this_module;
TSN_Module_Data *this_module_data = NULL;

static void 
signal_handler(int signum)
{
    is_running = 0;
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void
_cb_event(TSN_Event_CB_Data data)
{
    printf("[CUC] Triggered callback for event ID %d\n", data.event_id);
    
    if (data.event_id == EVENT_ERROR) {
        printf("[REST][CB] ERROR (%d): %s\n", data.entry_id, data.msg);
    }
    else if (data.event_id == EVENT_TOPOLOGY_DISCOVERY_REQUESTED) {
        printf("[CUC][CB] Topology discovery requested!\n");
    }

    return;
}

void
_init_cuc()
{
    this_module_data = malloc(sizeof(TSN_Module_Data));
}

// ------------------------------------
// MAIN
// ------------------------------------
int
main(void)
{
    // Signal handling
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);

    // Init this module
    this_module.name = "CUC";
    this_module.description = "Represents the Central User Controller in the network based on the central configuration approach of TSN";
    this_module.path ="./CUCModule";
    this_module.subscribed_events_mask = (EVENT_ERROR | EVENT_TOPOLOGY_DISCOVERY_REQUESTED);
    this_module.cb_event = _cb_event;

    rc = module_init(&this_module);
    if (rc == EXIT_FAILURE) {
        printf("[CUC] Error initializing module!\n");
        goto cleanup;
    }

    printf("[CUC] CUC module successfully started and running\n");

    // Keep running
    while (is_running) {
        sleep(1);
    }

    printf("[CUC] Stopping the module...\n");
    
cleanup:
    rc = module_shutdown();
    if (rc == EXIT_FAILURE) {
        printf("[REST] Error shutting down the module!\n");
    }

    return rc;
}
