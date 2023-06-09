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
module_init(char *module_name, TSN_Module **module, uint32_t adjusted_subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data))
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
            uint32_t mask = (adjusted_subscribed_events_mask > -1) ? adjusted_subscribed_events_mask : (*module)->subscribed_events_mask;
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
