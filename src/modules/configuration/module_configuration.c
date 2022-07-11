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

static void configuration_fill_app_param(struct configuration_parameter *parameter,
                                         const TSN_App *app)
{
    int i;

    for (i = 0; i < app->count_parameters; ++i) {
        TSN_App_Parameter *par = &app->parameters[i];

        /* OPC/UA Configuation URI */
        if (!strcmp(par->name, "opcua_configuration_uri"))
            parameter->opcua_configuration_uri = parameter_data_value_to_string(par);

        /* vPLC parameter */
        if (!strcmp(par->name, "vplc_current_status"))
            parameter->vplc.current_status = parameter_data_value_to_string(par);

        if (!strcmp(par->name, "vplc_commanded_status"))
            parameter->vplc.commanded_status = parameter_data_value_to_string(par);

        if (!strcmp(par->name, "vplc_axes")) {
            char *axes;

            axes = parameter_data_value_to_string(par);
            parameter->vplc.axes = atoi(axes);
            if (parameter->vplc.axes < 0)
                parameter->vplc.axes = 0;
            free(axes);
        }

        if (!strcmp(par->name, "vplc_speed")) {
            char *speed;

            speed = parameter_data_value_to_string(par);
            parameter->vplc.speed = atof(speed);
            free(speed);
        }

        if (!strcmp(par->name, "vplc_length")) {
            char *length;

            length = parameter_data_value_to_string(par);
            parameter->vplc.length = atof(length);
            free(length);
        }

        /* Execution parameter */
        if (!strcmp(par->name, "exec_cycle_time")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->exec.cycle_time = strtoull(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->exec.cycle_time = CONFIGURATION_DEFAULT_CYCLE_TIME_NS;
            free(time);
        }

        if (!strcmp(par->name, "exec_base_time")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->exec.base_time = strtoull(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->exec.base_time = CONFIGURATION_DEFAULT_BASE_TIME_NS;
            free(time);
        }

        if (!strcmp(par->name, "exec_wakeup_latency")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->exec.wakeup_latency = strtoull(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->exec.wakeup_latency = CONFIGURATION_DEFAULT_WAKEUP_LATENCY_NS;
            free(time);
        }

        if (!strcmp(par->name, "exec_scheduling_priority")) {
            char *prio;

            prio = parameter_data_value_to_string(par);
            parameter->exec.scheduling_priority = atoi(prio);
            if (parameter->exec.scheduling_priority <= 0)
                parameter->exec.scheduling_priority = CONFIGURATION_DEFAULT_SCHEDULING_PRIORITY;
            free(prio);
        }

        if (!strcmp(par->name, "exec_socket_priority")) {
            char *prio;

            prio = parameter_data_value_to_string(par);
            parameter->exec.socket_priority = atoi(prio);
            if (parameter->exec.socket_priority < 0)
                parameter->exec.scheduling_priority = CONFIGURATION_DEFAULT_SOCKET_PRIORITY;
            free(prio);
        }
    }
}

static void configuration_free_app_param(struct configuration_parameter *parameter)
{
    if (!parameter)
        return;

    free((void *)parameter->opcua_configuration_uri);
    free((void *)parameter->vplc.current_status);
    free((void *)parameter->vplc.commanded_status);
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void _cb_event(TSN_Event_CB_Data data)
{
    struct configuration_parameter param = { };
    const char *event_name = NULL;
    TSN_App *app = NULL;
    int ret;

    // Initial parameter deployment
    if (data.event_id & EVENT_CONFIGURATION_DEPLOY) {
        event_name = "EVENT_CONFIGURATION_DEPLOY";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Enrich app configuration parameters from sysrepo data
        configuration_fill_app_param(&param, app);

        // FIXME: Configure parameters via OPC/UA!
    }

    // Update run time parameters
    if (data.event_id & EVENT_CONFIGURATION_CHANGED) {
        event_name = "EVENT_CONFIGURATION_CHANGED";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Enrich app configuration parameters from sysrepo data
        configuration_fill_app_param(&param, app);

        // FIXME: Configure parameters via OPC/UA!
    }

out:
    log("Event '%s' (%s, %s)", event_name, data.entry_id, data.msg);
    configuration_free_app_param(&param);
    application_app_put(app);
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
