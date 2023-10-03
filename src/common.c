/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>     // fork
#include <sys/types.h>  // pid_t
//#include <sys/wait.h>   // wait

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include "../src_generated/tsndep_nodeids.h"
#include "../src_generated/pcmsubscriber_nodeids.h"

#include "common.h"
#include "logger.h"

int ret;


// ----------------------------------------------
//      FUNCTIONS - Module-Handling
// ----------------------------------------------

int
module_connect()
{
    // Establish a connection to sysrepo
    ret = sysrepo_connect();

    return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
//module_init(char *module_name, TSN_Module **module, uint32_t adjusted_subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data))
module_init(char *module_name, TSN_Module **module, uint64_t adjusted_subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data))
{
    // Connect (if not done yet)
    ret = module_connect();

    // Get the registered modules
    TSN_Modules *all_modules = malloc(sizeof(TSN_Modules));
    ret = sysrepo_get_all_modules(&all_modules);

    // Search for the desired module
    for (int i=0; i<all_modules->count_modules; ++i) {
        if (strcmp(module_name, all_modules->modules[i].name) == 0) {
            // Found the module
            (*module) = &all_modules->modules[i];

            // Setting the generic callback method
            //uint32_t mask = (adjusted_subscribed_events_mask > -1) ? adjusted_subscribed_events_mask : (*module)->subscribed_events_mask;
            uint64_t mask = (adjusted_subscribed_events_mask > -1) ? adjusted_subscribed_events_mask : (*module)->subscribed_events_mask;
            sysrepo_init_callback(mask, cb_event);

            // Start listening to notifications
            ret = sysrepo_start_listening();

            // Write the PID to sysrepo
            pid_t module_pid = getpid();
            (*module)->p_id = module_pid;
            ret = sysrepo_set_module_pid((*module)->id, module_pid);
            if (ret) {
                printf("[COMMON] Error writing PID (%d) to module '%s' with ID %d!\n", module_pid, (*module)->name, (*module)->id);
                ret = EXIT_FAILURE;
            } else {
                printf("[COMMON] Successfully started module '%s' with PID %d\n", (*module)->name, module_pid);
            }


            goto cleanup;
        }
    }
    ret = EXIT_FAILURE;

cleanup:
    return ret;
}

int
module_shutdown(int module_id)
{
    // Reset pid in datastore
    ret = sysrepo_set_module_pid(module_id, 0);
    if (ret) {
        printf("[COMMON] Error resetting pid of module with ID %d in sysrepo!\n", module_id);
        return EXIT_FAILURE;
    }

    // Disconnect from sysrepo connection and stop listening
    int rc = sysrepo_disconnect();

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
module_register(int module_id)
{
    ret = sysrepo_register_module(module_id);
    return ret;
}

int
module_unregister(int module_id)
{
    ret = sysrepo_unregister_module(module_id);
    return ret;
}


int
module_get_id(int module_id, TSN_Module **module)
{
    ret = sysrepo_get_module(module_id, module);
    return ret;
}

int
module_get_all(TSN_Modules **modules)
{
    ret = sysrepo_get_all_modules(modules);
    return ret;
}

int
module_delete(int module_id)
{
    ret = sysrepo_delete_module(module_id);
    return ret;
}

int
module_start(int module_id)
{
    // Get the module from sysrepo
    TSN_Module *module = NULL;
    module = malloc(sizeof(TSN_Module));
    //ret = sysrepo_get_module_from_registered(module_id, &module);
    ret = sysrepo_get_module(module_id, &module);
    if (ret) {
        printf("[COMMON] Error reading module with ID %d from sysrepo!\n", module_id);
        return EXIT_FAILURE;
    }

    // Fork the current process
    pid_t pid = fork();
    if (pid == 0) {
        // Parameters for the call
        char *arg = malloc(strlen(module->name) + strlen("[ControlTSN Module] ") + 1);
        sprintf(arg, "[ControlTSN Module] %s", module->name);
        char *argv[] = {arg, NULL};
        // Start the module
        if (execv(module->path, argv) == -1) {
            printf("[COMMON] Error starting module '%s'!\n", module->name);
            return EXIT_FAILURE;
        }

    } else if (pid < 0) {
        printf("[COMMON] Error creating a process for module '%s'\n", module->name);
        return EXIT_FAILURE;
    }

    /*
    // Write the PID to sysrepo
    ret = sysrepo_set_module_pid(module->id, pid);
    if (ret) {
        printf("[COMMON] Error writing PID (%d) to module '%s' with ID %d!\n", pid, module->name, module->id);
        return EXIT_FAILURE;
    }
    printf("[COMMON] Successfully started module '%s' with PID %d\n", module->name, pid);
    */


    return EXIT_SUCCESS;
}

int
module_stop(int module_id)
{
    // Get the module from sysrepo
    TSN_Module *module = NULL;
    module = malloc(sizeof(TSN_Module));
    //ret = sysrepo_get_module_from_registered(module_id, &module);
    ret = sysrepo_get_module(module_id, &module);
    if (ret) {
        printf("[COMMON] Error reading module with ID %d from sysrepo!\n", module_id);
        return EXIT_FAILURE;
    }

    // Kill the process
    if (!(module->p_id > 0)) {
        printf("[COMMON] Error stopping process of module '%s'. Could not read the PID!\n", module->name);
        return EXIT_FAILURE;
    }

    if (kill(module->p_id, SIGTERM) != 0) {
        printf("[COMMON] Error stopping process of module '%s' with PID %d!\n", module->name, module->p_id);
        return EXIT_FAILURE;
    }

    // Reset pid in datastore
    ret = sysrepo_set_module_pid(module_id, 0);
    if (ret) {
        printf("[COMMON] Error resetting pid of module with ID %d in sysrepo!\n", module_id);
        return EXIT_FAILURE;
    }

    printf("[COMMON] Successfully stopped module '%s' with PID %d\n", module->name, module->p_id);

    return EXIT_SUCCESS;
}

int
module_update_data(int module_id, TSN_Module_Data module_data)
{
    ret = sysrepo_update_module_data(module_id, module_data);
    return ret;
}

int
module_get_data(int module_id, TSN_Module_Data **module_data)
{
    ret = sysrepo_get_module_data(module_id, module_data);
    return ret;
}

TSN_Module_Data_Entry *
module_get_data_entry(TSN_Module_Data *module_data, const char *entry_name)
{
    TSN_Module_Data_Entry *entry = NULL;

    for (int i=0; i<module_data->count_entries; ++i) {
        if (strcmp(module_data->entries[i].name, entry_name) == 0) {
            return &(module_data->entries[i]);
        }
    }

    return entry;
}


// ----------------------------------------------
//      FUNCTIONS - Stream
// ----------------------------------------------
int
streams_get_all(TSN_Streams **streams, uint8_t without_configured_ones)
{
    ret = sysrepo_get_all_streams(streams, without_configured_ones);
    return ret;
}

int 
streams_get(char *stream_id, TSN_Stream **stream)
{
    ret = sysrepo_get_stream(stream_id, stream);
    return ret;
}

TSN_Request
create_stream_request(TSN_Enddevice *talker_device,
                      uint16_t count_listeners,
                      TSN_Enddevice *listener_devices,
                      IEEE_TrafficSpecification *traffic_spec,
                      IEEE_UserToNetworkRequirements *qos_talker,
                      IEEE_UserToNetworkRequirements *qos_listeners)
{
    TSN_Request request;
    TSN_Talker talker;
    TSN_Listener *listener_list;

    // Talker
    // End station interfaces
    IEEE_InterfaceId talker_if;
    talker_if.interface_name = "";
    talker_if.mac_address = strdup(talker_device->mac);
    talker.count_end_station_interfaces = 1;
    talker.end_station_interfaces = malloc(sizeof(IEEE_InterfaceId) * talker.count_end_station_interfaces);
    talker.end_station_interfaces[0] = talker_if;

    // Traffic specification
    talker.traffic_specification = *traffic_spec;

    // Dataframe specification
    talker.count_data_frame_specifications = 1; // Why is this necessary? The YANG module description says this list is optional but it also defines min-elements: 1
    IEEE_DataFrameSpecification dfspec;
    dfspec.field_type = DATA_FRAME_SPECIFICATION_VLAN_TAG;
    IEEE_VlanTag *v_tag = malloc(sizeof(IEEE_VlanTag));
    v_tag->vlan_id = 0;
    v_tag->priority_code_point = 6;
    dfspec.ieee802_vlan_tag = v_tag;
    talker.data_frame_specification = malloc(sizeof(IEEE_DataFrameSpecification) * talker.count_data_frame_specifications);
    talker.data_frame_specification[0] = dfspec;

    // User to network requirements
    talker.user_to_network_requirements = *qos_talker;

    // Interface capabilities
    talker.interface_capabilities.count_cb_stream_iden_types = 0;
    talker.interface_capabilities.count_cb_sequence_types = 0;


    // Listeners
    listener_list = malloc(sizeof(TSN_Listener) * count_listeners);
    for (int i=0; i<count_listeners; ++i) {
        TSN_Listener listener;

        // Index
        listener.index = i;

        // End station interfaces
        IEEE_InterfaceId listener_if;
        listener_if.interface_name = "";
        listener_if.mac_address = strdup(listener_devices[i].mac);
        listener.count_end_station_interfaces = 1;
        listener.end_station_interfaces = malloc(sizeof(IEEE_InterfaceId) * listener.count_end_station_interfaces);
        listener.end_station_interfaces[0] = listener_if;

        // User to network requirements
        listener.user_to_network_requirements = qos_listeners[i];

        // Interface capabilities
        listener.interface_capabilities.count_cb_stream_iden_types = 0;
        listener.interface_capabilities.count_cb_sequence_types = 0;

        listener_list[i] = listener;
    }

    request.talker = talker;
    request.count_listeners = count_listeners;
    request.listener_list = listener_list;

    return request;
}

int
stream_request(TSN_Request *request, char **generated_stream_id)
{
    ret = sysrepo_write_stream_request(request, generated_stream_id);
    return ret;
}

int
stream_set_computed(char *stream_id, TSN_Configuration *configuration)
{
    ret = sysrepo_write_stream_configuration(stream_id, configuration);
    return ret;
}

int
streams_delete(char *stream_id)
{
    ret = sysrepo_delete_stream(stream_id);
    return ret;
}

// ----------------------------------------------
//      FUNCTIONS - Topology
// ----------------------------------------------
int
topology_get(TSN_Topology **topology)
{
    ret = sysrepo_get_topology(topology);
    return ret;
}

int
topology_get_devices(TSN_Devices **devices)
{
    ret = sysrepo_get_all_devices(devices);
    return ret;
}

void
topology_put_devices(TSN_Devices *devices)
{
    int i;

    if (!devices)
        return;

    for (i = 0; i < devices->count_enddevices; ++i) {
        free(devices->enddevices[i].mac);
        //free(devices->enddevices[i].app_ref);
        int j;
        for (j = 0; j < devices->enddevices[i].count_apps; ++j) {
            free(devices->enddevices[i].apps[j].app_ref);
        }
    }

    for (i = 0; i < devices->count_switches; ++i)
        free(devices->switches[i].mac);

    free(devices->enddevices);
    free(devices->switches);
    free(devices);
}

int
topology_get_graph(TSN_Graph **graph)
{
    ret = sysrepo_get_topology_graph(graph);
    return ret;
}


// ----------------------------------------------
//      FUNCTIONS - Applications
// ----------------------------------------------
int
application_get(TSN_Application **application)
{
    ret = sysrepo_get_application(application);
    return ret;
}

int
application_get_apps(TSN_Apps **apps)
{
    ret = sysrepo_get_application_apps(apps);
    return ret;
}

static void _application_app_put(TSN_App *app)
{
    int i;

    if (!app)
        return;

    free(app->id);
    free(app->name);
    free(app->description);
    free(app->version);
    free(app->image_ref);

    for (i = 0; i < app->count_parameters; ++i)
        free(app->parameters[i].name);
    free(app->parameters);
}

void application_put_apps(TSN_Apps *apps)
{
    int i;

    if (!apps)
        return;

    for (i = 0; i < apps->count_apps; ++i)
        _application_app_put(&apps->apps[i]);
    free(apps->apps);

    free(apps);
}

int
application_get_images(TSN_Images **images)
{
    ret = sysrepo_get_application_images(images);
    return ret;
}

void application_put_images(TSN_Images *images)
{
    int i;

    if (!images)
        return;

    for (i = 0; i < images->count_images; ++i) {
        free(images->images[i].id);
        free(images->images[i].name);
        free(images->images[i].description);
        free(images->images[i].version);
    }
    free(images->images);
    free(images);
}

void application_app_put(TSN_App *app)
{
    _application_app_put(app);
    free(app);
}

const TSN_Enddevice *
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

static int
_write_stream_sendreceive_flag(const TSN_Enddevice *enddevice, bool enable)
{
    int rc = EXIT_FAILURE;
    UA_NodeId nodeId;
    UA_StatusCode ret;
    UA_Client *client;
    UA_Variant *variant;

    variant = UA_Variant_new();

    if (!enddevice->interface_uri) {
        printf("[COMMON][OPCUA][ERROR] No configuration interface specified for enddevice %s!\n", enddevice->name);
        goto cleanup;
    }

    // Connect to the server
    client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(config);
    
    ret = UA_Client_connect(client, enddevice->interface_uri);
    if (ret != UA_STATUSCODE_GOOD) {
        printf("[COMMON][OPCUA][ERROR] Could not connect to OPC UA Server '%s'\n", enddevice->interface_uri);
        goto cleanup;
    }

    nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_SENDRECEIVEENABLED);
    UA_Boolean sendReceiveEnabled = (UA_Boolean) enable;
    UA_Variant_setScalarCopy(variant, &sendReceiveEnabled, &UA_TYPES[UA_TYPES_BOOLEAN]);
    ret = UA_Client_writeValueAttribute(client, nodeId, variant);
    if (ret != UA_STATUSCODE_GOOD) {
        printf("[COMMON][OPCUA][ERROR] Could not write SendReceiveEnabled to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        goto cleanup;
    }

    rc = EXIT_SUCCESS;

cleanup:
    UA_Variant_delete(variant);
    UA_Client_delete(client);

    return rc;
}

static bool
_read_stream_sendreceive_flag(const TSN_Enddevice *enddevice)
{
    int rc = EXIT_FAILURE;
    UA_NodeId nodeId;
    UA_StatusCode ret;
    UA_Client *client;
    UA_Variant *value;
    value = UA_Variant_new();

    if (!enddevice->interface_uri) {
        printf("[COMMON][OPCUA][ERROR] No configuration interface specified for enddevice %s!\n", enddevice->name);
        goto cleanup;
    }

    // Connect to the server
    client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(config);
    
    ret = UA_Client_connect(client, enddevice->interface_uri);
    if (ret != UA_STATUSCODE_GOOD) {
        printf("[COMMON][OPCUA][ERROR] Could not connect to OPC UA Server '%s'\n", enddevice->interface_uri);
        goto cleanup;
    }

    nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_SENDRECEIVEENABLED);
    ret = UA_Client_readValueAttribute(client, nodeId, value);
    if (ret != UA_STATUSCODE_GOOD || !UA_Variant_isScalar(value) || value->type != &UA_TYPES[UA_TYPES_BOOLEAN]) {
        printf("[COMMON][OPCUA][ERROR] Could not read SendReceiveEnabled to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        goto cleanup;
    }

    UA_Boolean enabled = *(UA_Boolean *)value->data;

cleanup:
    UA_Variant_delete(value);
    UA_Client_delete(client);

    return enabled;
}

int configuration_stream_set_sendreceive(char *stream_id, bool enable)
{
    // --------------------------------------------------------------
    // TODO/Info: 
    // In the future the send/receive enabled flag will be per communication flow.
    // There will be one OPC UA Server receiving the configurations ('Endpoint Daemon').
    // The datamodel will be splitted into communication, application and deployment.
    // Under each of them we have the parameters specific for each application on the enddevice.
    // e.g.:

    // - ROOT
    // --- Communication
    // ------ App_1
    // --------- CycleTime
    // --------- Interface
    // --------- SocketPrio
    // --------- SendReceiveEnabled     <<<<<<<<< Here ???
    // --------- ...
    // ------ App_N
    // --------- ...
    // --- Application
    // ------ App_1
    // --------- CurrentState
    // --------- SendReceiveEnabled     <<<<<<<<< OR here ???
    // --------- ...
    // --- Deployment
    // ------ App_1
    // --------- ThreadPriority
    // --------- ...
    //
    // -------------------------------------------------------------- 
    int rc;
    TSN_Apps *apps = malloc(sizeof(TSN_Apps));
    TSN_Devices *devices = malloc(sizeof(TSN_Devices));
    TSN_App **listenerApps = NULL;
    const TSN_Enddevice **listenerDevices = NULL;

    // First we make sure to find all participating apps and their enddevices 
    // before setting the flag for one of them

    // Get the talker and listener apps
    rc = sysrepo_get_application_apps(&apps);
    if (rc != EXIT_SUCCESS) {
        printf("[COMMON][ERROR] Error reading applications from datastore!\n");
        goto cleanup;
    }
    // Find the talker
    TSN_App *talkerApp;
    for (int i=0; i<apps->count_apps; ++i) {
        for (int j=0; j<apps->apps[i].stream_mapping.count_egress; j++) {
            if (strcmp(apps->apps[i].stream_mapping.egress[j], stream_id) == 0) {
                talkerApp = &apps->apps[i];
            }
        }
    }
    // Find the listener(s)
    listenerApps = (TSN_App **)malloc(apps->count_apps * sizeof(TSN_App *));
    int count_listeners = 0;
    for (int i=0; i<apps->count_apps; ++i) {
        for (int j=0; j<apps->apps[i].stream_mapping.count_ingress; j++) {
            if (strcmp(apps->apps[i].stream_mapping.ingress[j], stream_id) == 0) {
                listenerApps[count_listeners] = &apps->apps[i];
                count_listeners++;
            }
        }
    }
    // Find the enddevices
    rc = sysrepo_get_all_devices(&devices);
    if (rc != EXIT_SUCCESS) {
        printf("[COMMON][ERROR] Error reading devices from datastore!\n");
        goto cleanup;
    }
    const TSN_Enddevice *talkerDevice = configuration_find_enddevice_of_app(talkerApp->id, devices->enddevices, devices->count_enddevices);
    if (!talkerDevice) {
        printf("[COMMON][ERROR] Error finding enddevice for app id %s!\n", talkerApp->id);
        goto cleanup;
    }

    listenerDevices = (const TSN_Enddevice **)malloc(count_listeners * sizeof(TSN_Enddevice *));
    for (int i=0; i<count_listeners; ++i) {
        listenerDevices[i] = configuration_find_enddevice_of_app(listenerApps[i]->id, devices->enddevices, devices->count_enddevices);
        if (!listenerDevices[i]) {
            printf("[COMMON][ERROR] Error finding enddevice for app id %s!\n", listenerApps[i]->id);
            goto cleanup;
        }
    }

    // Now that we have all enddevices we can start setting the stream flag
    // First the listeners to prevent missing data from the talker
    int success_counter = 0;
    for (int i=0; i<count_listeners; ++i) {
        rc = _write_stream_sendreceive_flag(listenerDevices[i], enable);
        if (rc == EXIT_SUCCESS) {
            success_counter++;
        }
    }
    // Finally the talker
    rc = _write_stream_sendreceive_flag(talkerDevice, enable);
    if (rc == EXIT_SUCCESS) {
        success_counter++;
    }

cleanup:
    free(apps);
    free(devices);
    free(listenerApps);
    free(listenerDevices);
    // Indicate a success if all listeners and the talker are set to enabled
    return success_counter == (count_listeners + 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool configuration_stream_get_sendreceive(char *stream_id)
{
    int rc;
    TSN_Apps *apps = malloc(sizeof(TSN_Apps));
    TSN_Devices *devices = malloc(sizeof(TSN_Devices));
    TSN_App **listenerApps = NULL;
    const TSN_Enddevice **listenerDevices = NULL;

    bool is_enabled = true;

    // First we make sure to find all participating apps and their enddevices 
    // before setting the flag for one of them

    // Get the talker and listener apps
    rc = sysrepo_get_application_apps(&apps);
    if (rc != EXIT_SUCCESS) {
        printf("[COMMON][ERROR] Error reading applications from datastore!\n");
        goto cleanup;
    }
    // Find the talker
    TSN_App *talkerApp;
    for (int i=0; i<apps->count_apps; ++i) {
        for (int j=0; j<apps->apps[i].stream_mapping.count_egress; j++) {
            if (strcmp(apps->apps[i].stream_mapping.egress[j], stream_id) == 0) {
                talkerApp = &apps->apps[i];
            }
        }
    }

    // Find the listener(s)
    listenerApps = (TSN_App **)malloc(apps->count_apps * sizeof(TSN_App *));
    int count_listeners = 0;
    for (int i=0; i<apps->count_apps; ++i) {
        for (int j=0; j<apps->apps[i].stream_mapping.count_ingress; j++) {
            if (strcmp(apps->apps[i].stream_mapping.ingress[j], stream_id) == 0) {
                listenerApps[count_listeners] = &apps->apps[i];
                count_listeners++;
            }
        }
    }

    // Find the enddevices
    rc = sysrepo_get_all_devices(&devices);
    if (rc != EXIT_SUCCESS) {
        printf("[COMMON][ERROR] Error reading devices from datastore!\n");
        goto cleanup;
    }
    const TSN_Enddevice *talkerDevice = configuration_find_enddevice_of_app(talkerApp->id, devices->enddevices, devices->count_enddevices);
    if (!talkerDevice) {
        printf("[COMMON][ERROR] Error finding enddevice for app id %s!\n", talkerApp->id);
        goto cleanup;
    }

    listenerDevices = (const TSN_Enddevice **)malloc(count_listeners * sizeof(TSN_Enddevice *));
    for (int i=0; i<count_listeners; ++i) {
        listenerDevices[i] = configuration_find_enddevice_of_app(listenerApps[i]->id, devices->enddevices, devices->count_enddevices);
        if (!listenerDevices[i]) {
            printf("[COMMON][ERROR] Error finding enddevice for app id %s!\n", listenerApps[i]->id);
            goto cleanup;
        }
    }

    // Now that we have all enddevices we can start getting the stream flag

    // For indicating a enabled stream all apps must have the flag set to true
    bool e;
    for (int i=0; i<count_listeners; ++i) {
        e = _read_stream_sendreceive_flag(listenerDevices[i]);
        is_enabled = is_enabled && e;
    }
    // Finally the talker
    e = _read_stream_sendreceive_flag(talkerDevice);
    is_enabled = is_enabled && e;
    

cleanup:
    free(apps);
    free(devices);
    free(listenerApps);
    free(listenerDevices);
    // Indicate a success if all listeners and the talker are set to enabled
    return is_enabled;
}

static void
_fill_opcua_variant_with_app_parameter(UA_Variant *variant, TSN_App_Parameter *param)
{
    if (param->type == BINARY) {
        UA_String val = UA_String_fromChars(param->value.binary_val);
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_STRING]);

    } else if (param->type == BOOLEAN) {
        UA_Boolean val = param->value.boolean_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);

    } else if (param->type == DECIMAL64) {
        UA_Double val = param->value.decimal64_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_DOUBLE]);
        
    } else if (param->type == INSTANCE_IDENTIFIER) {
        UA_String val = UA_String_fromChars(param->value.instance_identifier_val);
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_STRING]);
        
    } else if (param->type == INT8) {
        UA_SByte val = param->value.int8_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_SBYTE]);
        
    } else if (param->type == INT16) {
        UA_Int16 val = param->value.int16_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_INT16]);
        
    } else if (param->type == INT32) {
        UA_Int32 val = param->value.int32_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_INT32]);
        
    } else if (param->type == INT64) {
        UA_Int64 val = param->value.int64_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_INT64]);
        
    } else if (param->type == STRING) {
        UA_String val = UA_String_fromChars(param->value.string_val);
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_STRING]);
        
    } else if (param->type == UINT8) {
        UA_Byte val = param->value.uint8_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_BYTE]);
        
    } else if (param->type == UINT16) {
        UA_UInt16 val = param->value.uint16_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_UINT16]);
        
    } else if (param->type == UINT32) {
        UA_UInt32 val = param->value.uint32_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_UINT32]);
        
    } else if (param->type == UINT64) {
        UA_UInt64 val = param->value.uint64_val;
        UA_Variant_setScalarCopy(variant, &val, &UA_TYPES[UA_TYPES_UINT64]);
    }
}

static int 
_write_app_parameters(const TSN_Enddevice *enddevice, TSN_App *app)
{
    int rc = EXIT_FAILURE;
    UA_NodeId paramsRootNodeId;
    UA_StatusCode ret;
    UA_Client *client;
    UA_Variant *variant;

    variant = UA_Variant_new();

    if (!enddevice->interface_uri) {
        printf("[COMMON][OPCUA][ERROR] No configuration interface specified for enddevice %s!\n", enddevice->name);
        goto cleanup;
    }

    // Connect to the server
    client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(config);
    
    ret = UA_Client_connect(client, enddevice->interface_uri);
    if (ret != UA_STATUSCODE_GOOD) {
        printf("[COMMON][OPCUA][ERROR] Could not connect to OPC UA Server '%s'\n", enddevice->interface_uri);
        goto cleanup;
    }

    #define APP_PARAMS_ROOT_FOLDER_NAMESPACE_INDEX 8
    paramsRootNodeId = UA_NODEID_NUMERIC(APP_PARAMS_ROOT_FOLDER_NAMESPACE_INDEX, UA_PCMSUBSCRIBER_ID_PCMSUBSCRIBERENGINEERING);
    
    UA_TranslateBrowsePathsToNodeIdsRequest request;
    UA_TranslateBrowsePathsToNodeIdsRequest_init(&request);

    request.browsePathsSize = app->count_parameters;
    request.browsePaths = (UA_BrowsePath *) UA_Array_new(app->count_parameters, &UA_TYPES[UA_TYPES_BROWSEPATH]);

    // Iterate over the app parameters to find the Node for each of them
    for (int i=0; i<app->count_parameters; ++i) {
        UA_BrowsePath browsePath;
        UA_BrowsePath_init(&browsePath);
        browsePath.startingNode = paramsRootNodeId;
        // Assuming the params are stored as direct child of the params root folder!
        #define BROWSE_PATHS_SIZE 1
        browsePath.relativePath.elements = (UA_RelativePathElement *) UA_Array_new(BROWSE_PATHS_SIZE, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
        browsePath.relativePath.elementsSize = BROWSE_PATHS_SIZE;
        UA_RelativePathElement *elem = &browsePath.relativePath.elements[0];
        elem->referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
        elem->targetName = UA_QUALIFIEDNAME_ALLOC(APP_PARAMS_ROOT_FOLDER_NAMESPACE_INDEX, app->parameters[i].name);
       
        request.browsePaths[i] = browsePath;
    }

    uint16_t successCounter = 0;
    UA_TranslateBrowsePathsToNodeIdsResponse response = UA_Client_Service_translateBrowsePathsToNodeIds(client, request);
    if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        for (int i=0; i<response.resultsSize; i++) {
            if (response.results[i].statusCode == UA_STATUSCODE_GOOD) {
                UA_NodeId nodeId = response.results[i].targets[0].targetId.nodeId;
                printf("Response #%d:  %d\n", i, nodeId.identifier.numeric);

                // Write param value to node
                // Convert param type to OPC UA Type...
                _fill_opcua_variant_with_app_parameter(variant, &(app->parameters[i]));
                ret = UA_Client_writeValueAttribute(client, nodeId, variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    printf("[COMMON][OPCUA][ERROR] Could not write '%s' to Node ns=%d;i=%d!\n", app->parameters[i].name, nodeId.namespaceIndex, nodeId.identifier.numeric);
                    goto cleanup;
                }
                successCounter++;
            }
        }
    }

cleanup:
    UA_Variant_delete(variant);
    UA_Client_delete(client);

    return successCounter == app->count_parameters ? EXIT_SUCCESS : EXIT_FAILURE;
}

int configuration_app_deploy_parameters(char *app_id)
{
    int rc;
    TSN_App *app = malloc(sizeof(TSN_App));
    TSN_Devices *devices = malloc(sizeof(TSN_Devices));

    // Read the application from the datastore
    rc = sysrepo_get_application_app(app_id, &app);
    if (rc != EXIT_SUCCESS) {
        printf("[COMMON][ERROR] Error reading application with ID '%s' from datastore!\n", app_id);
        goto cleanup;
    }

    // Find the enddevice
    rc = sysrepo_get_all_devices(&devices);
    if (rc != EXIT_SUCCESS) {
        printf("[COMMON][ERROR] Error reading devices from datastore!\n");
        goto cleanup;
    }
    const TSN_Enddevice *appDevice = configuration_find_enddevice_of_app(app->id, devices->enddevices, devices->count_enddevices);
    if (!appDevice) {
        printf("[COMMON][ERROR] Error finding enddevice for app id %s!\n", app->id);
        goto cleanup;
    }

    // Write the application parameters to the engineering interface of the corresponding enddevice
    rc = _write_app_parameters(appDevice, app);

cleanup:
    free(app);
    free(devices);

    return rc;
}
