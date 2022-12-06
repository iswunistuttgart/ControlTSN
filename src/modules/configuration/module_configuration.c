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

#include "../../../src_generated/types_endpoint_generated.h"
#include "../../../src_generated/types_endpoint_generated_handling.h"
#include "../../../src_generated/endpoint_nodeids.h"

#include "module_configuration.h"

//---------------------------------------
// Configuration Module Utility functions
//---------------------------------------
static volatile sig_atomic_t is_running = 1;

static UA_DataTypeArray customTypesArray = { NULL, UA_TYPES_ENDPOINT_COUNT, UA_TYPES_ENDPOINT, UA_FALSE };

static void signal_handler(int signum)
{
    (void)signum;
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

static char *x_strdup(const char *str)
{
    char *p = strdup(str);
    if (!p) {
        log("Error allocating memory!");
        exit(EXIT_FAILURE);
    }

    return p;
}

static void *x_calloc(size_t nmemb, size_t size)
{
    void *p;

    p = calloc(nmemb, size);
    if (!p) {
        log("Error allocating memory!");
        exit(EXIT_FAILURE);
    }

    return p;
}

static void *x_realloc(void *ptr, size_t size)
{
    void *p;

    p = realloc(ptr, size);
    if (!p) {
        log("Error allocating memory!");
        exit(EXIT_FAILURE);
    }

    return p;
}

static const TSN_Enddevice *
configuration_find_enddevice_of_app(char *app_id, const TSN_Enddevice *enddevices,
                                    uint16_t count_enddevices)
{
    int i, j;

    // Search through the enddevices and find the one where the given app is running on
    for (i = 0; i < count_enddevices; ++i) {
        if (enddevices[i].has_app) {
            for (j = 0; j < enddevices[i].count_apps; ++j) {
                if (!strcmp(enddevices[i].apps[j].app_ref, app_id))
                    return &enddevices[i];
            }
        }
    }

    return NULL;
}

static void configuration_fill_app_param(struct configuration_parameter *parameter,
                                         const TSN_App *app, const TSN_Enddevice *device)
{
    int num_app_parameters = app->count_parameters;
    int i, j;

    parameter->opcua_configuration_uri = x_strdup(device->interface_uri);
    parameter->app_id = x_strdup(app->id);

    //
    // Fetch and store engineering parameters.
    //
    for (i = 0; i < app->count_parameters; ++i) {
        TSN_App_Parameter *par = &app->parameters[i];

        if (!strcmp(par->name, "current_status")) {
            parameter->eng.current_status = parameter_data_value_to_string(par);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "commanded_status")) {
            parameter->eng.commanded_status = parameter_data_value_to_string(par);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "destination_mac")) {
            parameter->eng.destination_mac = parameter_data_value_to_string(par);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "subscribed_mac")) {
            parameter->eng.subscribed_mac = parameter_data_value_to_string(par);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "interface")) {
            parameter->eng.interface = parameter_data_value_to_string(par);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "sendreceive_enabled")) {
            char *txrx;
            int tmp;

            txrx = parameter_data_value_to_string(par);
            tmp = atoi(txrx);
            parameter->eng.sendreceive_enabled = tmp ? true : false;
            if (tmp <= 0)
                parameter->eng.sendreceive_enabled = CONFIGURATION_DEFAULT_SENDRECEIVE_ENABLED;
            free(txrx);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "cycle_time")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->eng.cycle_time = strtod(time, &endptr);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->eng.cycle_time = CONFIGURATION_DEFAULT_CYCLE_TIME;
            free(time);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "base_time")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->eng.base_time = strtoull(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->eng.base_time = CONFIGURATION_DEFAULT_BASE_TIME_NS;
            free(time);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "qbv_offset")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->eng.qbv_offset = strtoll(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->eng.qbv_offset = CONFIGURATION_DEFAULT_QBV_OFFSET_NS;
            free(time);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "wakeup_latency")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->eng.wakeup_latency = strtoull(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->eng.wakeup_latency = CONFIGURATION_DEFAULT_WAKEUP_LATENCY_NS;
            free(time);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "wcet")) {
            char *time, *endptr;

            time = parameter_data_value_to_string(par);
            parameter->eng.wcet = strtoull(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->eng.wcet = CONFIGURATION_DEFAULT_WCET;
            free(time);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "scheduling_priority")) {
            char *prio;

            prio = parameter_data_value_to_string(par);
            parameter->eng.scheduling_priority = atoi(prio);
            if (parameter->eng.scheduling_priority <= 0)
                parameter->eng.scheduling_priority = CONFIGURATION_DEFAULT_SCHEDULING_PRIORITY;
            free(prio);
            num_app_parameters--;
        }

        if (!strcmp(par->name, "socket_priority")) {
            char *prio;

            prio = parameter_data_value_to_string(par);
            parameter->eng.socket_priority = atoi(prio);
            if (parameter->eng.socket_priority < 0)
                parameter->eng.scheduling_priority = CONFIGURATION_DEFAULT_SOCKET_PRIORITY;
            free(prio);
            num_app_parameters--;
        }
    }

    //
    // Skip container module parameters.
    //
    for (i = 0; i < app->count_parameters; ++i) {
        TSN_App_Parameter *par = &app->parameters[i];

        if (!strcmp(par->name, "command") || !strcmp(par->name, "command_line") ||
            !strcmp(par->name, "capabilities") || !strcmp(par->name, "cpus") ||
            !strcmp(par->name, "ram"))
            num_app_parameters--;
    }

    parameter->app.num_parameters = num_app_parameters;
    parameter->app.names = x_calloc(num_app_parameters, sizeof(char *));
    parameter->app.values = x_calloc(num_app_parameters, sizeof(char *));
    parameter->app.types = x_calloc(num_app_parameters, sizeof(char *));

    //
    // All parameters which not belong to engineering or container are
    // application specific.
    //
    for (i = 0, j = 0; i < app->count_parameters; ++i) {
        TSN_App_Parameter *par = &app->parameters[i];

        // Skip engineering parameter
        if (!strcmp(par->name, "current_status") || !strcmp(par->name, "commanded_status") ||
            !strcmp(par->name, "destination_mac") || !strcmp(par->name, "subscribed_mac") ||
            !strcmp(par->name, "interface") || !strcmp(par->name, "sendreceive_enabled") ||
            !strcmp(par->name, "cycle_time") || !strcmp(par->name, "base_time") ||
            !strcmp(par->name, "qbv_offset") || !strcmp(par->name, "wakeup_latency") ||
            !strcmp(par->name, "wcet") || !strcmp(par->name, "scheduling_priority") ||
            !strcmp(par->name, "socket_priority"))
            continue;

        // Skip container parameter
        if (!strcmp(par->name, "command") || !strcmp(par->name, "command_line") ||
            !strcmp(par->name, "capabilities") || !strcmp(par->name, "cpus") ||
            !strcmp(par->name, "ram"))
            continue;

        parameter->app.names[j] = x_strdup(par->name);
        parameter->app.values[j] = parameter_data_value_to_string(par);
        parameter->app.types[j] = x_strdup(data_type_to_string(par->type));
        j++;
    }
}

static void configuration_free_app_param(struct configuration_parameter *parameter)
{
    size_t i;

    if (!parameter)
        return;

    free((void *)parameter->opcua_configuration_uri);
    free((void *)parameter->app_id);
    free((void *)parameter->app_path);
    free((void *)parameter->eng.current_status);
    free((void *)parameter->eng.commanded_status);
    free((void *)parameter->eng.destination_mac);
    free((void *)parameter->eng.subscribed_mac);
    free((void *)parameter->eng.interface);

    for (i = 0; i < parameter->app.num_parameters; ++i) {
        free((void *)parameter->app.names[i]);
        free((void *)parameter->app.values[i]);
        free((void *)parameter->app.types[i]);
    }

    free(parameter->app.names);
    free(parameter->app.values);
    free(parameter->app.types);
}

// ------------------------------------
// Deploy OPC/UA configuration
// ------------------------------------
static void configuration_deploy_app_par(const struct configuration_parameter *parameter,
                                         const TSN_Enddevice *enddevice)
{
    UA_NodeId app_node;
    UA_StatusCode ret;
    UA_Client *client;
    UA_Variant out;

    // If no URI provided, nothing works
    if (!parameter || !enddevice->interface_uri || !parameter->app_id) {
        log("Missing parameters for deploying configuration to Application!");
        return;
    }

    // Connect to Application Configuration Endpoint
    client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);

    UA_ClientConfig_setDefault(cc);
    cc->customDataTypes = &customTypesArray;

    ret = UA_Client_connect(client, enddevice->interface_uri);
    if (ret != UA_STATUSCODE_GOOD) {
        log("Failed to connect to endpoint service!");
        UA_Client_delete(client);
        return;
    }

    //
    // Retrieve all TSN applications stored on endpoint service.
    //
    UA_Variant_init(&out);
    app_node = UA_NODEID_NUMERIC(2, UA_ENDPOINTID_ENDPOINT_APPLICATION);

    ret = UA_Client_readValueAttribute(client, app_node, &out);
    if (ret != UA_STATUSCODE_GOOD) {
        log("Failed to read TSN applications!");
        goto out;
    }

    //
    // Check whether the application already exists.
    //
    UA_ExtensionObject *obj = out.data;
    size_t num_apps = out.arrayLength;
    UA_TSNApplication *apps, *app = NULL;
    apps = x_calloc(num_apps, sizeof(*apps));

    for (size_t i = 0; i < num_apps; ++i) {
        UA_TSNApplication *a = obj[i].content.decoded.data;

        UA_TSNApplication_init(&apps[i]);
        UA_TSNApplication_copy(a, &apps[i]);

        char id[a->iD.length + 1];
        memcpy(id, a->iD.data, a->iD.length);
        id[a->iD.length] = '\0';

        if (!strcmp(parameter->app_id, id))
            app = &apps[i];
    }

    //
    // If not found, increase the apps array by one.
    //
    if (!app) {
        num_apps++;
        apps = x_realloc(apps, num_apps * sizeof(*apps));
        app = &apps[num_apps - 1];
        UA_TSNApplication_init(app);
    }

    //
    // Set all application parameter.
    //
    app->iD = UA_STRING_ALLOC((char *)parameter->app_id);
    app->path = UA_STRING_ALLOC((char *)parameter->app_path);
    app->engineeringParameters.basetime = parameter->eng.base_time;
    app->engineeringParameters.commandedStatus = UA_STRING_ALLOC(parameter->eng.commanded_status);
    app->engineeringParameters.currentStatus = UA_STRING_ALLOC(parameter->eng.current_status);
    app->engineeringParameters.cycleTime = parameter->eng.cycle_time;
    app->engineeringParameters.destinationMAC = UA_STRING_ALLOC(parameter->eng.destination_mac);
    app->engineeringParameters.interface = UA_STRING_ALLOC(parameter->eng.interface);
    app->engineeringParameters.qbvOffset = parameter->eng.qbv_offset;
    app->engineeringParameters.sendReceiveEnabled = parameter->eng.sendreceive_enabled;
    app->engineeringParameters.socketPriority = parameter->eng.socket_priority;
    app->engineeringParameters.subscribedMAC = UA_STRING_ALLOC(parameter->eng.subscribed_mac);
    app->engineeringParameters.wCET = parameter->eng.wakeup_latency;

    app->applicationParameters = x_calloc(parameter->app.num_parameters, sizeof(UA_TSNApplicationEngineeringParameters));
    app->applicationParametersSize = parameter->app.num_parameters;

    for (size_t i = 0; i < app->applicationParametersSize; ++i) {
        app->applicationParameters[i].name = UA_STRING_ALLOC(parameter->app.names[i]);
        app->applicationParameters[i].type = UA_STRING_ALLOC(parameter->app.types[i]);
        app->applicationParameters[i].value = UA_STRING_ALLOC(parameter->app.values[i]);
    }

    //
    // Write array of TSN application back to TSN endpoint.
    //
    UA_ExtensionObject *new_obj = x_calloc(num_apps, sizeof(*new_obj));

    for (size_t i = 0; i < num_apps; ++i) {
        UA_ExtensionObject *o = &new_obj[i];

        UA_ExtensionObject_init(o);

        o->encoding = UA_EXTENSIONOBJECT_DECODED;
        o->content.decoded.data = &apps[i];
        o->content.decoded.type = &UA_TYPES_ENDPOINT[UA_TYPES_ENDPOINT_TSNAPPLICATION];
    }

    UA_Variant v;
    UA_Variant_init(&v);

    UA_Variant_setArrayCopy(&v, new_obj, num_apps, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
    ret = UA_Client_writeValueAttribute(client, app_node, &v);
    if (ret != UA_STATUSCODE_GOOD)
        log("Failed to write TSN applications!\n");

    for (size_t i = 0; i < num_apps; ++i)
        UA_TSNApplication_clear(&apps[i]);
    free(apps);
    free(new_obj);

    UA_Variant_clear(&v);

out:
    UA_Variant_clear(&out);
    UA_Client_delete(client);
}

static void
configuration_request_app_run_state(const char *app_id,
                                    const TSN_Enddevice *enddevice)
{
    UA_NodeId app_node;
    UA_StatusCode ret;
    UA_Client *client;
    UA_Variant out;

    // If no URI provided, nothing works
    if (!enddevice || !app_id || !enddevice->interface_uri) {
        log("Missing parameters for requesting run state!");
        return;
    }

    // Connect to Application Configuration Endpoint
    client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);

    UA_ClientConfig_setDefault(cc);
    cc->customDataTypes = &customTypesArray;

    ret = UA_Client_connect(client, enddevice->interface_uri);
    if (ret != UA_STATUSCODE_GOOD) {
        log("Failed to connect to endpoint service!");
        UA_Client_delete(client);
        return;
    }

    //
    // Retrieve all TSN applications stored on endpoint service.
    //
    UA_Variant_init(&out);
    app_node = UA_NODEID_NUMERIC(2, UA_ENDPOINTID_ENDPOINT_APPLICATION);

    ret = UA_Client_readValueAttribute(client, app_node, &out);
    if (ret != UA_STATUSCODE_GOOD) {
        log("Failed to read TSN applications!");
        goto out;
    }

    //
    // Check whether the application already exists.
    //
    UA_ExtensionObject *obj = out.data;
    size_t num_apps = out.arrayLength;
    UA_TSNApplication *apps, *app = NULL;
    apps = x_calloc(num_apps, sizeof(*apps));

    for (size_t i = 0; i < num_apps; ++i) {
        UA_TSNApplication *a = obj[i].content.decoded.data;

        UA_TSNApplication_init(&apps[i]);
        UA_TSNApplication_copy(a, &apps[i]);

        char id[a->iD.length + 1];
        memcpy(id, a->iD.data, a->iD.length);
        id[a->iD.length] = '\0';

        if (!strcmp(app_id, id))
            app = &apps[i];
    }

    //
    // To request the run state the application should exist.
    //
    if (!app) {
        log("Failed to request run state for app '%s'", app_id);
        goto out;
    }

    //
    // Set AppState to "CONFIG"
    // --> On the endpoint side, this will trigger the app to apply the saved parameters
    // and use the communication configuration (socket prio, qbv offset, basetime, etc.) to create a PubSub connection
    //
    app->engineeringParameters.commandedStatus = UA_STRING_ALLOC("CONFIG");
    app->engineeringParameters.currentStatus = UA_STRING_ALLOC("CONFIG");

    //
    // Write array of TSN application back to TSN endpoint.
    //
    UA_ExtensionObject *new_obj = x_calloc(num_apps, sizeof(*new_obj));

    for (size_t i = 0; i < num_apps; ++i) {
        UA_ExtensionObject *o = &new_obj[i];

        UA_ExtensionObject_init(o);

        o->encoding = UA_EXTENSIONOBJECT_DECODED;
        o->content.decoded.data = &apps[i];
        o->content.decoded.type = &UA_TYPES_ENDPOINT[UA_TYPES_ENDPOINT_TSNAPPLICATION];
    }

    UA_Variant v;
    UA_Variant_init(&v);

    UA_Variant_setArrayCopy(&v, new_obj, num_apps, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
    ret = UA_Client_writeValueAttribute(client, app_node, &v);
    if (ret != UA_STATUSCODE_GOOD)
        log("Failed to write TSN applications!\n");

    for (size_t i = 0; i < num_apps; ++i)
        UA_TSNApplication_clear(&apps[i]);
    free(apps);
    free(new_obj);

    UA_Variant_clear(&v);

out:
    UA_Variant_clear(&out);
    UA_Client_delete(client);
}

static void
configuration_toggle_app_send_receive(const char *app_id,
                                      const TSN_Enddevice *enddevice)
{
    UA_NodeId app_node;
    UA_StatusCode ret;
    UA_Client *client;
    UA_Variant out;

    // If no URI provided, nothing works
    if (!enddevice || !app_id || !enddevice->interface_uri) {
        log("Missing parameters for requesting run state!");
        return;
    }

    // Connect to Application Configuration Endpoint
    client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);

    UA_ClientConfig_setDefault(cc);
    cc->customDataTypes = &customTypesArray;

    ret = UA_Client_connect(client, enddevice->interface_uri);
    if (ret != UA_STATUSCODE_GOOD) {
        log("Failed to connect to endpoint service!");
        UA_Client_delete(client);
        return;
    }

    //
    // Retrieve all TSN applications stored on endpoint service.
    //
    UA_Variant_init(&out);
    app_node = UA_NODEID_NUMERIC(2, UA_ENDPOINTID_ENDPOINT_APPLICATION);

    ret = UA_Client_readValueAttribute(client, app_node, &out);
    if (ret != UA_STATUSCODE_GOOD) {
        log("Failed to read TSN applications!");
        goto out;
    }

    //
    // Check whether the application already exists.
    //
    UA_ExtensionObject *obj = out.data;
    size_t num_apps = out.arrayLength;
    UA_TSNApplication *apps, *app = NULL;
    apps = x_calloc(num_apps, sizeof(*apps));

    for (size_t i = 0; i < num_apps; ++i) {
        UA_TSNApplication *a = obj[i].content.decoded.data;

        UA_TSNApplication_init(&apps[i]);
        UA_TSNApplication_copy(a, &apps[i]);

        char id[a->iD.length + 1];
        memcpy(id, a->iD.data, a->iD.length);
        id[a->iD.length] = '\0';

        if (!strcmp(app_id, id))
            app = &apps[i];
    }

    //
    // To request the run state the application should exist.
    //
    if (!app) {
        log("Failed to request send/receive enable for app '%s'", app_id);
        goto out;
    }

    //
    // Toggle send/receive enable flag.
    //
    app->engineeringParameters.sendReceiveEnabled = !app->engineeringParameters.sendReceiveEnabled;

    //
    // Write array of TSN application back to TSN endpoint.
    //
    UA_ExtensionObject *new_obj = x_calloc(num_apps, sizeof(*new_obj));

    for (size_t i = 0; i < num_apps; ++i) {
        UA_ExtensionObject *o = &new_obj[i];

        UA_ExtensionObject_init(o);

        o->encoding = UA_EXTENSIONOBJECT_DECODED;
        o->content.decoded.data = &apps[i];
        o->content.decoded.type = &UA_TYPES_ENDPOINT[UA_TYPES_ENDPOINT_TSNAPPLICATION];
    }

    UA_Variant v;
    UA_Variant_init(&v);

    UA_Variant_setArrayCopy(&v, new_obj, num_apps, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
    ret = UA_Client_writeValueAttribute(client, app_node, &v);
    if (ret != UA_STATUSCODE_GOOD)
        log("Failed to write TSN applications!\n");

    for (size_t i = 0; i < num_apps; ++i)
        UA_TSNApplication_clear(&apps[i]);
    free(apps);
    free(new_obj);

    UA_Variant_clear(&v);

out:
    UA_Variant_clear(&out);
    UA_Client_delete(client);
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void _cb_event(TSN_Event_CB_Data data)
{
    struct configuration_parameter param = { 0 };
    const char *event_name = NULL;
    TSN_App *app = NULL;
    int ret;
    TSN_Devices *devices = malloc(sizeof(TSN_Devices));

    // Initial parameter deployment
    if (data.event_id & EVENT_CONFIGURATION_DEPLOY) {
        event_name = "EVENT_CONFIGURATION_DEPLOY";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        const TSN_Enddevice *enddev = configuration_find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (!enddev)
            goto out;

        // Enrich app configuration parameters from sysrepo data
        configuration_fill_app_param(&param, app, enddev);

        // Configure parameters via OPC/UA!
        configuration_deploy_app_par(&param, enddev);
    }

    // Update run time parameters
    if (data.event_id & EVENT_CONFIGURATION_CHANGED) {
        event_name = "EVENT_CONFIGURATION_CHANGED";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        const TSN_Enddevice *enddev = configuration_find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (!enddev)
            goto out;

        // Enrich app configuration parameters from sysrepo data
        configuration_fill_app_param(&param, app, enddev);

        // Configure parameters via OPC/UA!
        configuration_deploy_app_par(&param, enddev);
    }

    // Request App Run mode
    if (data.event_id & EVENT_CONFIGURATION_REQUEST_RUN_STATE) {
        event_name = "EVENT_CONFIGURATION_REQUEST_RUN_STATE";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        const TSN_Enddevice *enddev = configuration_find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (!enddev)
            goto out;

        // Set app state to Config in order to trigger tghe app to apply the configuration and create the PubSub connection
        configuration_request_app_run_state(data.entry_id, enddev);
    }

    // Toggle app send & receive flag
    if (data.event_id & EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE) {
        event_name = "EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto out;

        // Get the corresponding enddevice this app is deployed to
        ret = sysrepo_get_all_devices(&devices);
        const TSN_Enddevice *enddev = configuration_find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
        if (!enddev)
            goto out;

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
                     (EVENT_CONFIGURATION_DEPLOY |
                     EVENT_CONFIGURATION_CHANGED |
                     EVENT_CONFIGURATION_REQUEST_RUN_STATE |
                     EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE),
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
    module_shutdown(this_module->id);
    return rc;
}
