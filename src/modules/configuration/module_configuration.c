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
            parameter->eng.cycle_time = strtoull(time, &endptr, 10);
            if (errno != 0 || endptr == time || *endptr != '\0')
                parameter->eng.cycle_time = CONFIGURATION_DEFAULT_CYCLE_TIME_NS;
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

    parameter->app.num_parameters = num_app_parameters;
    parameter->app.names = x_calloc(num_app_parameters, sizeof(char *));
    parameter->app.values = x_calloc(num_app_parameters, sizeof(char *));
    parameter->app.types = x_calloc(num_app_parameters, sizeof(char *));

    for (i = 0, j = 0; i < app->count_parameters; ++i) {
        TSN_App_Parameter *par = &app->parameters[i];

        if (!strcmp(par->name, "current_status") || !strcmp(par->name, "commanded_status") ||
            !strcmp(par->name, "destination_mac") || !strcmp(par->name, "subscribed_mac") ||
            !strcmp(par->name, "interface") || !strcmp(par->name, "sendreceive_enabled") ||
            !strcmp(par->name, "cycle_time") || !strcmp(par->name, "base_time") ||
            !strcmp(par->name, "qbv_offset") || !strcmp(par->name, "wakeup_latency") ||
            !strcmp(par->name, "wcet") || !strcmp(par->name, "scheduling_priority") ||
            !strcmp(par->name, "socket_priority"))
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

static bool
configuration_find_app_node(UA_Client *client,
                            const struct configuration_parameter *parameter,
                            UA_NodeId *app_node_id)
{
    UA_BrowseRequest browse_request;
    bool found = false;

    UA_BrowseRequest_init(&browse_request);

    browse_request.requestedMaxReferencesPerNode = 0;
    browse_request.nodesToBrowse = UA_BrowseDescription_new();
    browse_request.nodesToBrowseSize = 1;
    browse_request.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    browse_request.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    UA_BrowseResponse browse_response = UA_Client_Service_browse(client, browse_request);
    for (size_t i = 0; i < browse_response.resultsSize; ++i) {
        for (size_t j = 0; j < browse_response.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &browse_response.results[i].references[j];
            char browse_name[1024] = { 0 };

            if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC ||
                ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                size_t len = ref->browseName.name.length >= sizeof(browse_name) ? sizeof(browse_name) - 1 :
                    ref->browseName.name.length;

                memcpy(browse_name, ref->browseName.name.data, len);
            }

            if (!strcmp(browse_name, parameter->app_id)) {
                found = true;
                *app_node_id = ref->nodeId.nodeId;
                break;
            }
        }
    }
    UA_BrowseRequest_clear(&browse_request);
    UA_BrowseResponse_clear(&browse_response);

    return found;
}

static UA_StatusCode
configuration_set_app_parameter(UA_Client *client,
                                const struct configuration_parameter *parameter,
                                UA_NodeId parameter_node_id)
{
    UA_StatusCode ret = UA_STATUSCODE_GOOD;
    UA_BrowseRequest browse_request;

    UA_BrowseRequest_init(&browse_request);

    browse_request.requestedMaxReferencesPerNode = 0;
    browse_request.nodesToBrowse = UA_BrowseDescription_new();
    browse_request.nodesToBrowseSize = 1;
    browse_request.nodesToBrowse[0].nodeId = parameter_node_id;
    browse_request.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    UA_BrowseResponse browse_response = UA_Client_Service_browse(client, browse_request);
    for (size_t i = 0; i < browse_response.resultsSize; ++i) {
        for (size_t j = 0; j < browse_response.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &browse_response.results[i].references[j];
            char browse_name[1024] = { 0 };

            if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC ||
                ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                size_t len = ref->browseName.name.length >= sizeof(browse_name) ? sizeof(browse_name) - 1 :
                    ref->browseName.name.length;

                memcpy(browse_name, ref->browseName.name.data, len);
            }

            // ParameterType::Names
            if (!strcmp(browse_name, "Names")) {
                UA_String names[parameter->app.num_parameters];
                UA_UInt32 array_dims = 0;
                UA_Variant array;

                for (size_t i = 0; i < parameter->app.num_parameters; ++i)
                    names[i] = UA_STRING((char *)parameter->app.names[i]);

                UA_Variant_setArrayCopy(&array, names, parameter->app.num_parameters, &UA_TYPES[UA_TYPES_STRING]);
                array.arrayDimensions = &array_dims;
                array.arrayDimensionsSize = 1;
                array.arrayDimensions[0] = parameter->app.num_parameters;

                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, &array);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write ParameterType::Names!");
                    continue;
                }
            }

            // ParameterType::Values
            if (!strcmp(browse_name, "Values")) {
                UA_String values[parameter->app.num_parameters];
                UA_UInt32 array_dims = 0;
                UA_Variant array;

                for (size_t i = 0; i < parameter->app.num_parameters; ++i)
                    values[i] = UA_STRING((char *)parameter->app.values[i]);

                UA_Variant_setArrayCopy(&array, values, parameter->app.num_parameters, &UA_TYPES[UA_TYPES_STRING]);
                array.arrayDimensions = &array_dims;
                array.arrayDimensionsSize = 1;
                array.arrayDimensions[0] = parameter->app.num_parameters;

                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, &array);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write ParameterType::Values!");
                    continue;
                }
            }
        }
    }

    UA_BrowseRequest_clear(&browse_request);
    UA_BrowseResponse_clear(&browse_response);

    return ret;
}

// Keep in sync with [endpoint]src/app_config/app_model.c
static UA_NodeId app_create_node_id = {1, UA_NODEIDTYPE_NUMERIC, {1010}};

static UA_StatusCode
configuration_create_app_node(UA_Client *client,
                              const struct configuration_parameter *parameter)
{
    UA_Variant input, *output;
    size_t output_size;
    UA_StatusCode ret;

    UA_Variant_init(&input);
    UA_String app_id = UA_String_fromChars(parameter->app_id);
    UA_Variant_setScalar(&input, &app_id, &UA_TYPES[UA_TYPES_STRING]);

    ret = UA_Client_call(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                         app_create_node_id, 1, &input, &output_size, &output);
    UA_String_clear(&app_id);

    return ret;
}

// ------------------------------------
// Deploy initial configuration
// ------------------------------------
static void configuration_deploy_app_par(const struct configuration_parameter *parameter,
                                         const TSN_Enddevice *enddevice)
{
    UA_BrowseResponse browse_response;
    UA_BrowseRequest browse_request;
    UA_Variant *my_variant;
    UA_NodeId app_node_id;
    UA_StatusCode ret;
    UA_Client *client;

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
        UA_Client_delete(client);
        return;
    }

    my_variant = UA_Variant_new();

    //
    // Check for application node: The exported UPC/UA data model should contain
    // a node for the application to configure all parameters. If it's not
    // there, a new application instance has to be created.
    //
    if (!configuration_find_app_node(client, parameter, &app_node_id)) {
        ret = configuration_create_app_node(client, parameter);
        if (ret != UA_STATUSCODE_GOOD) {
            log("Failed to create Application instance on Endpoint Service!");
            goto out;
        }

        // Find app node again.
        if (!configuration_find_app_node(client, parameter, &app_node_id)) {
            log("Failed to find Application instance on Endpoint Service!");
            goto out;
        }
    }

    // Application instance found. Configure parameter.
    browse_request.requestedMaxReferencesPerNode = 0;
    browse_request.nodesToBrowse = UA_BrowseDescription_new();
    browse_request.nodesToBrowseSize = 1;
    browse_request.nodesToBrowse[0].nodeId = app_node_id;
    browse_request.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    browse_response = UA_Client_Service_browse(client, browse_request);

    for (size_t i = 0; i < browse_response.resultsSize; ++i) {
        for (size_t j = 0; j < browse_response.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &browse_response.results[i].references[j];
            char browse_name[1024] = { 0 };

            if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC ||
                ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                size_t len = ref->browseName.name.length >= sizeof(browse_name) ? sizeof(browse_name) - 1 :
                    ref->browseName.name.length;

                memcpy(browse_name, ref->browseName.name.data, len);
            }

            // AppType::CurrentStatus
            if (!strcmp(browse_name, "CurrentStatus")) {
                UA_String string_value = UA_STRING((char *)parameter->eng.current_status);
                UA_Variant_setScalarCopy(my_variant, &string_value, &UA_TYPES[UA_TYPES_STRING]);
                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, my_variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %s to %s!", parameter->eng.current_status, "AppType::CurrentStatus");
                    continue;
                }
            }

            // AppType::CommandedStatus
            if (!strcmp(browse_name, "CommandedStatus")) {
                UA_String string_value = UA_STRING((char *)parameter->eng.commanded_status);
                UA_Variant_setScalarCopy(my_variant, &string_value, &UA_TYPES[UA_TYPES_STRING]);
                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, my_variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %s to %s!", parameter->eng.commanded_status, "AppType::CommandedStatus");
                    continue;
                }
            }

            // AppType::CycleTime
            if (!strcmp(browse_name, "CycleTime")) {
                UA_UInt64 uint_value = parameter->eng.cycle_time;
                UA_Variant_setScalarCopy(my_variant, &uint_value, &UA_TYPES[UA_TYPES_UINT64]);
                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, my_variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %lu to %s!", parameter->eng.cycle_time, "AppType::CycleTime");
                    continue;
                }
            }

            // AppType::BaseTime
            if (!strcmp(browse_name, "BaseTime")) {
                UA_UInt64 uint_value = parameter->eng.base_time;
                UA_Variant_setScalarCopy(my_variant, &uint_value, &UA_TYPES[UA_TYPES_UINT64]);
                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, my_variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %lu to %s!", parameter->eng.base_time, "AppType::BaseTime");
                    continue;
                }
            }

            // AppType::WakeupLatency
            if (!strcmp(browse_name, "WakeupLatency")) {
                UA_UInt64 uint_value = parameter->eng.wakeup_latency;
                UA_Variant_setScalarCopy(my_variant, &uint_value, &UA_TYPES[UA_TYPES_UINT64]);
                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, my_variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %lu to %s!", parameter->eng.wakeup_latency, "AppType::WakeupLatency");
                    continue;
                }
            }

            // AppType::SchedulingPriority
            if (!strcmp(browse_name, "SchedulingPriority")) {
                UA_Int32 int_value = parameter->eng.scheduling_priority;
                UA_Variant_setScalarCopy(my_variant, &int_value, &UA_TYPES[UA_TYPES_INT32]);
                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, my_variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %d to %s!", parameter->eng.socket_priority, "AppType::SchedulingPriority");
                    continue;
                }
            }

            // AppType::SocketPriority
            if (!strcmp(browse_name, "SocketPriority")) {
                UA_Int32 int_value = parameter->eng.socket_priority;
                UA_Variant_setScalarCopy(my_variant, &int_value, &UA_TYPES[UA_TYPES_INT32]);
                ret = UA_Client_writeValueAttribute(client, ref->nodeId.nodeId, my_variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %d to %s!", parameter->eng.socket_priority, "AppType::SocketPriority");
                    continue;
                }
            }

            // AppType::ApplicationParameter
            if (!strcmp(browse_name, "ApplicationParameter")) {
                ret = configuration_set_app_parameter(client, parameter, ref->nodeId.nodeId);
                if (ret != UA_STATUSCODE_GOOD) {
                    log("Failed to write %d to %s!", parameter->eng.socket_priority, "AppType::SocketPriority");
                    continue;
                }
            }
        }
    }

    UA_BrowseRequest_clear(&browse_request);
    UA_BrowseResponse_clear(&browse_response);

out:
    UA_Variant_delete(my_variant);
    UA_Client_delete(client);
}

static void
configuration_request_app_run_state(const char *app_id,
                                    const TSN_Enddevice *enddevice)
{
    UA_Variant *my_variant;
    UA_StatusCode retval;
    UA_Client *client;

    (void)app_id;

    if (!enddevice || !enddevice->interface_uri)
        return;

    client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);

    UA_ClientConfig_setDefault(cc);
    cc->customDataTypes = &customTypesArray;

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

static void
configuration_toggle_app_send_receive(const char *app_id,
                                      const TSN_Enddevice *enddevice)
{
    UA_Variant *my_variant;
    UA_NodeId nodeID;
    UA_StatusCode retval;
    UA_Client *client;

    (void)app_id;

    if (!enddevice || !enddevice->interface_uri)
        return;

    client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);

    UA_ClientConfig_setDefault(cc);
    cc->customDataTypes = &customTypesArray;

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
    struct configuration_parameter param = { 0 };
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
        ret = sysrepo_get_application_app(data.entry_id, &app);
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
        ret = sysrepo_get_application_app(data.entry_id, &app);
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
        ret = sysrepo_get_application_app(data.entry_id, &app);
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
