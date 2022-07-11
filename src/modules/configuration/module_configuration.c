/*
 * Copyright (C) 2022 Linutronix GmbH
 * Author Kurt Kanzenbach <kurt@linutronix.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <ulfius.h>

#include "../../common.h"
#include "../../events_definitions.h"
#include "../../helper/json_serializer.h"
#include "module_configuration.h"

//---------------------------------------
// Configuration Module Utility functions
//---------------------------------------
static volatile sig_atomic_t is_running = 1;

static void signal_handler(int signum)
{
    is_running = 0;
}

#define log(...)                                                        \
    do {                                                                \
        configuration_log(__func__, __VA_ARGS__);                       \
    } while (0)

static void configuration_log(const char * restrict func, const char * restrict fmt, ...)
{
    FILE *out = stdout;
    va_list args;

    va_start(args, fmt);

    fprintf(out, "[%s][%s]: ", "Configuration", func);
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    fflush(out);

    va_end(args);
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void _cb_event(TSN_Event_CB_Data data)
{
    const char *event_name = NULL;

    if (data.event_id & EVENT_CONFIGURATION_DEPLOY)
        event_name = "EVENT_CONFIGURATION_DEPLOY";
    if (data.event_id & EVENT_CONFIGURATION_CHANGED)
        event_name = "EVENT_CONFIGURATION_CHANGED";

    log("Event '%s' (%s, %s)", event_name, data.entry_id, data.msg);
}

// ------------------------------------
// MAIN
// ------------------------------------
int main(void)
{
    TSN_Module *this_module = NULL;
    int rc;

    // Signal handling
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);

    // Init this module
    rc = module_init("Configuration", &this_module,
                     EVENT_CONFIGURATION_DEPLOY |
                     EVENT_CONFIGURATION_CHANGED,
                     _cb_event);
    if (rc == EXIT_FAILURE) {
        log("Error initializing module!");
        goto cleanup;
    }

    // Get saved module data
    TSN_Module_Data *module_data = malloc(sizeof(TSN_Module_Data));
    if (!module_data)
        goto cleanup;

    rc = module_get_data(this_module->id, &module_data);
    if (rc != EXIT_SUCCESS)
        goto cleanup;

    log("Configuration module successfully started and running");

    // Keep running
    while (is_running)
        sleep(1);

    log("Stopping the module...");

cleanup:
    module_shutdown();
    return rc;
}
