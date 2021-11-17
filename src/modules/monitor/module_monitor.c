#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "../../common.h"
#include "../../events_definitions.h"
#include "module_monitor.h"

int rc;
volatile sig_atomic_t is_running = 1;

TSN_Module *this_module = NULL;

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
    char *event_name = NULL;
    if (data.event_id & EVENT_ERROR) {
        event_name = strdup("EVENT_ERROR");
    } else if (data.event_id & EVENT_STREAM_REQUESTED) {
        event_name = strdup("EVENT_STREAM_REQUESTED");
    } else if (data.event_id & EVENT_STREAM_CONFIGURED) {
        event_name = strdup("EVENT_STREAM_CONFIGURED");
    } else if (data.event_id & EVENT_STREAM_DELETED) {
        event_name = strdup("EVENT_STREAM_DELETED");
    } else if (data.event_id & EVENT_MODULE_ADDED) {
        event_name = strdup("EVENT_MODULE_ADDED");
    } else if (data.event_id & EVENT_MODULE_REGISTERED) {
        event_name = strdup("EVENT_MODULE_REGISTERED");
    } else if (data.event_id & EVENT_MODULE_DATA_UPDATED) {
        event_name = strdup("EVENT_MODULE_DATA_UPDATED");
    } else if (data.event_id & EVENT_MODULE_UNREGISTERED) {
        event_name = strdup("EVENT_MODULE_UNREGISTERED");
    } else if (data.event_id & EVENT_MODULE_DELETED) {
        event_name = strdup("EVENT_MODULE_DELETED");
    } else if (data.event_id & EVENT_TOPOLOGY_DISCOVERY_REQUESTED) {
        event_name = strdup("EVENT_TOPOLOGY_DISCOVERY_REQUESTED");
    } else if (data.event_id & EVENT_TOPOLOGY_DISCOVERED) {
        event_name = strdup("EVENT_TOPOLOGY_DISCOVERED");
    }

    printf("[Monitor][CB] Event '%s'   (%s, %s)\n", event_name, data.entry_id, data.msg);

    return;
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
    signal(SIGTERM, signal_handler);

    // Init this module
    /*
    this_module.name = "Monitor";
    this_module.description = "This module monitors the network including the streams and their latency, synchronization etc.";
    this_module.path = "./MonitorModule";
    this_module.subscribed_events_mask = (
        EVENT_ERROR |
        EVENT_STREAM_REQUESTED | EVENT_STREAM_CONFIGURED | EVENT_STREAM_DELETED | 
        EVENT_MODULE_ADDED | EVENT_MODULE_REGISTERED | EVENT_MODULE_DATA_UPDATED | EVENT_MODULE_UNREGISTERED | EVENT_MODULE_DELETED |
        EVENT_TOPOLOGY_DISCOVERY_REQUESTED | EVENT_TOPOLOGY_DISCOVERED);

    // sub mask = 203535

    this_module.cb_event = _cb_event;
    */

    //rc = module_init(&this_module);

    this_module = malloc(sizeof(TSN_Module));
    rc = module_init("Monitor", &this_module, (EVENT_ERROR | EVENT_STREAM_REQUESTED | EVENT_STREAM_CONFIGURED | EVENT_STREAM_DELETED | 
        EVENT_MODULE_ADDED | EVENT_MODULE_REGISTERED | EVENT_MODULE_DATA_UPDATED | EVENT_MODULE_UNREGISTERED | EVENT_MODULE_DELETED |
        EVENT_TOPOLOGY_DISCOVERY_REQUESTED | EVENT_TOPOLOGY_DISCOVERED), _cb_event);
    if (rc == EXIT_FAILURE) {
        printf("[Monitor] Error initializing module!\n");
        goto cleanup;
    }

    printf("[Monitor] Monitor module successfully started and running\n");

    // Keep running
    while (is_running) {
        sleep(1);
    }

    printf("[Monitor] Stopping the module...\n");
    
cleanup:
    rc = module_shutdown();
    if (rc == EXIT_FAILURE) {
        printf("[Monitor] Error shutting down the module!\n");
    }

    return rc;
}
