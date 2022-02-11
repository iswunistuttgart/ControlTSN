/*
 * Copyright (C) 2022 Linutronix GmbH
 * Author Kurt Kanzenbach <kurt@linutronix.de>
 */

#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "../../common.h"
#include "../../events_definitions.h"
#include "module_container.h"

static volatile sig_atomic_t is_running = 1;

static void signal_handler(int signum)
{
    is_running = 0;
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void _cb_event(TSN_Event_CB_Data data)
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

    if (!event_name)
        return;

    printf("[Container][CB] Event '%s'   (%s, %s)\n", event_name, data.entry_id, data.msg);

    free(event_name);

    return;
}


// ------------------------------------
// MAIN
// ------------------------------------
int main(void)
{
    TSN_Module *this_module = NULL;
    const char *registry_url;
    int rc;

    // Signal handling
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);

    // Init this module
    this_module = malloc(sizeof(TSN_Module));
    if (!this_module)
        return EXIT_FAILURE;

    rc = module_init("Container", &this_module, (EVENT_ERROR | EVENT_STREAM_REQUESTED | EVENT_STREAM_CONFIGURED | EVENT_STREAM_DELETED |
                                                 EVENT_MODULE_ADDED | EVENT_MODULE_REGISTERED | EVENT_MODULE_DATA_UPDATED | EVENT_MODULE_UNREGISTERED | EVENT_MODULE_DELETED |
                                                 EVENT_TOPOLOGY_DISCOVERY_REQUESTED | EVENT_TOPOLOGY_DISCOVERED), _cb_event);
    if (rc == EXIT_FAILURE) {
        printf("[Container] Error initializing module!\n");
        goto cleanup;
    }

    // Get saved module data
    TSN_Module_Data *module_data = malloc(sizeof(TSN_Module_Data));
    if (!module_data)
        goto cleanup;

    rc = module_get_data(this_module->id, &module_data);
    if (rc != EXIT_SUCCESS)
        goto cleanup;

    // Find Container registry url
    TSN_Module_Data_Entry *reg_entry = module_get_data_entry(module_data, MODULE_DATA_IDENTIFIER_REGISTRY);
    if (reg_entry) {
        registry_url = strdup(reg_entry->value.string_val);
        if (!registry_url)
            goto cleanup;
        printf("[Container] Container registry URL: '%s'\n", registry_url);
    }

    printf("[Container] Container module successfully started and running\n");

    // Keep running
    while (is_running)
        sleep(1);

    printf("[Container] Stopping the module...\n");

cleanup:
    module_shutdown();
    free(this_module);
    return rc;
}
