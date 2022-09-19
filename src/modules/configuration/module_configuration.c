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

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>

#include "../../common.h"
#include "../../events_definitions.h"
#include "../../helper/json_serializer.h"
#include "../../logger.h"

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

static TSN_Enddevice *
_find_enddevice_of_app( char *app_id, TSN_Enddevice *enddevices, uint16_t count_enddevices)
{
    // Search through the enddevices and find the one where the given app is running on
    for (int i=0; i<count_enddevices; ++i) {
        if (enddevices[i].has_app) {
            for (int j=0; j<enddevices[i].count_apps; j++) {
                if (strcmp(enddevices[i].apps[j].app_ref, app_id) == 0) {
                    return &enddevices[i];
                }
            }
        }
    }

    return NULL;
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
// Deploy initial configuration
// ------------------------------------
static void configuration_deploy_app_par(const struct configuration_parameter *parameter, TSN_Enddevice *enddevice)
{
    UA_Variant *my_variant;
    UA_StatusCode retval;
    UA_Client *client;

    /* If no URI provided, nothing works */
    //if (!parameter || !parameter->opcua_configuration_uri)
    //    return;
    if (!parameter || !enddevice->interface_uri)
        return;

    /* Connect to Application Configuration Endpoint */
    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    //retval = UA_Client_connect(client, parameter->opcua_configuration_uri);
    retval = UA_Client_connect(client, enddevice->interface_uri);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return;
    }

    /* Write vPLC parameters */
    UA_Int32 int_value = parameter->vplc.axes;
    my_variant = UA_Variant_new();
    UA_Variant_setScalarCopy(my_variant, &int_value, &UA_TYPES[UA_TYPES_INT32]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "axes"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %d to %s!", int_value, "vPLC.axes");
        goto out;
    }

    UA_Double double_value = parameter->vplc.length;
    UA_Variant_setScalarCopy(my_variant, &double_value, &UA_TYPES[UA_TYPES_DOUBLE]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "length"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %lf to %s!", double_value, "vPLC.length");
        goto out;
    }

    double_value = parameter->vplc.speed;
    UA_Variant_setScalarCopy(my_variant, &double_value, &UA_TYPES[UA_TYPES_DOUBLE]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "speed"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %lf to %s!", double_value, "vPLC.speed");
        goto out;
    }

    UA_String string_value = UA_STRING((char *)parameter->vplc.current_status);
    UA_Variant_setScalarCopy(my_variant, &string_value, &UA_TYPES[UA_TYPES_STRING]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "currentstatus"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %s to %s!", "INIT", "vPLC.currentstatus");
        goto out;
    }

    string_value = UA_STRING((char *)parameter->vplc.commanded_status);
    UA_Variant_setScalarCopy(my_variant, &string_value, &UA_TYPES[UA_TYPES_STRING]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "commandedstatus"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %s to %s!", "INIT", "vPLC.commandedstatus");
        goto out;
    }

    /* Write Execution parameters */
    UA_UInt64 uint_value = parameter->exec.cycle_time;
    UA_Variant_setScalarCopy(my_variant, &uint_value, &UA_TYPES[UA_TYPES_UINT64]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "cycletime"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %lu to %s!", uint_value, "Execution.cycletime");
        goto out;
    }

    uint_value = parameter->exec.base_time;
    UA_Variant_setScalarCopy(my_variant, &uint_value, &UA_TYPES[UA_TYPES_UINT64]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "basetime"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %lu to %s!", uint_value, "Execution.basetime");
        goto out;
    }

    uint_value = parameter->exec.wakeup_latency;
    UA_Variant_setScalarCopy(my_variant, &uint_value, &UA_TYPES[UA_TYPES_UINT64]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "wakeuplatency"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %lu to %s!", uint_value, "Execution.wakeuplatency");
        goto out;
    }

    int_value = parameter->exec.scheduling_priority;
    UA_Variant_setScalarCopy(my_variant, &int_value, &UA_TYPES[UA_TYPES_INT32]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "schedulingpriority"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %lu to %s!", uint_value, "Execution.schedulingpriority");
        goto out;
    }

    int_value = parameter->exec.socket_priority;
    UA_Variant_setScalarCopy(my_variant, &int_value, &UA_TYPES[UA_TYPES_INT32]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "socketpriority"), my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write %lu to %s!", uint_value, "Execution.socketpriority");
        goto out;
    }

out:
    UA_Variant_delete(my_variant);
    UA_Client_delete(client);
}

static void configuration_request_app_run_state(const char *app_id, TSN_Enddevice *enddevice) {
    UA_Variant *my_variant;
    UA_NodeId nodeID;
    UA_StatusCode retval;
    UA_Client *client;

    if (!enddevice || !enddevice->interface_uri) {
        return;
    }

    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    retval = UA_Client_connect(client, enddevice->interface_uri);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return;
    }

    // Set AppState to "CONFIG" 
    // --> On the endpoint side, this will trigger the app to apply the saved parameters 
    // and use the communication ocnfiguration (socket prio, qbv offset, basetime, etc.) to create a PubSub connection
    TSN_APPSTATE state;
    state = CONFIG;
    my_variant = UA_Variant_new();
    UA_NodeId appStateNodeID = UA_NODEID_STRING(1, "AppState");
    UA_Variant_setScalarCopy(my_variant, &state, &UA_TYPES[UA_TYPES_INT32]);
    retval = UA_Client_writeValueAttribute(client, appStateNodeID, my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write AppState 'CONFIG' to NodeID %s on OPC UA Server %s!", appStateNodeID.identifier.string, enddevice->interface_uri);
        goto out;
    }

out:
    UA_Variant_delete(my_variant);
    UA_Client_delete(client);
}

static void configuration_toggle_app_send_receive(const char *app_id, TSN_Enddevice *enddevice) {
     UA_Variant *my_variant;
    UA_NodeId nodeID;
    UA_StatusCode retval;
    UA_Client *client;

    if (!enddevice || !enddevice->interface_uri) {
        return;
    }

    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    retval = UA_Client_connect(client, enddevice->interface_uri);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return;
    }

    // Read current send & receive enabled flag ('PubsubEnabled')
    my_variant = UA_Variant_new();
    nodeID = UA_NODEID_STRING(1, "PubSubEnabled");
    retval = UA_Client_readValueAttribute(client, nodeID, my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to read App send & receive flag from NodeID %s on OPC UA Server %s!", nodeID.identifier.string, enddevice->interface_uri);
        goto out;
    }
    UA_Boolean sendReceiveEnabledFlag = *(UA_Boolean *) my_variant->data;
    sendReceiveEnabledFlag = !sendReceiveEnabledFlag;
    
    UA_Variant_setScalarCopy(my_variant, &sendReceiveEnabledFlag, &UA_TYPES[UA_TYPES_BOOLEAN]);
    retval = UA_Client_writeValueAttribute(client, nodeID, my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        log("Failed to write App send & receive flag to NodeID %s on OPC UA Server %s!", nodeID.identifier.string, enddevice->interface_uri);
        goto out;
    }

out:
    UA_Variant_delete(my_variant);
    UA_Client_delete(client);
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
    TSN_Devices *devices = malloc(sizeof(TSN_Devices));

    // Initial parameter deployment
    if (data.event_id & EVENT_CONFIGURATION_DEPLOY) {
        event_name = "EVENT_CONFIGURATION_DEPLOY";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.entry_id, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        TSN_Enddevice *enddev = _find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (enddev == NULL) {
            goto out;
        }

        // Enrich app configuration parameters from sysrepo data
        configuration_fill_app_param(&param, app);

        // Configure parameters via OPC/UA!
        configuration_deploy_app_par(&param, enddev);
    }

    // Update run time parameters
    if (data.event_id & EVENT_CONFIGURATION_CHANGED) {
        event_name = "EVENT_CONFIGURATION_CHANGED";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.entry_id, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        TSN_Enddevice *enddev = _find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (enddev == NULL) {
            goto out;
        }

        // Enrich app configuration parameters from sysrepo data
        configuration_fill_app_param(&param, app);

        // Configure parameters via OPC/UA!
        configuration_deploy_app_par(&param, enddev);
    }

    // Request App Run mode
    if (data.event_id & EVENT_CONFIGURATION_REQUEST_RUN_STATE) {
        event_name = "EVENT_CONFIGURATION_REQUEST_RUN_STATE";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.entry_id, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        TSN_Enddevice *enddev = _find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (enddev == NULL) {
            goto out;
        }

        // Set app state to Config in order to trigger tghe app to apply the configuration and create the PubSub connection
        configuration_request_app_run_state(data.entry_id, enddev);
    }

    // Toggle app send & receive flag
    if (data.event_id & EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE) {
        event_name = "EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.entry_id, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        TSN_Enddevice *enddev = _find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (enddev == NULL) {
            goto out;
        }

        // Toggle the flag
        configuration_toggle_app_send_receive(data.entry_id, enddev);
    }

out:
    log("Event '%s' (%s, %s)", event_name, data.entry_id, data.msg);
    configuration_free_app_param(&param);
    application_app_put(app);
    free(devices);
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
    rc = module_init("AppConfiguration", &this_module,
                     -1,
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
