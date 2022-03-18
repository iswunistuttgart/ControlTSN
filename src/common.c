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

    // Get the reegistered modules
    TSN_Modules *all_modules = malloc(sizeof(TSN_Modules));
    ret = sysrepo_get_all_modules(&all_modules);

    // Search for the desired module
    for (int i=0; i<all_modules->count_modules; ++i) {
        if (strcmp(module_name, all_modules->modules[i].name) == 0) {
            // Found the module
            (*module) = &all_modules->modules[i];

            // Setting the generic callback method
            uint32_t mask = (adjusted_subscribed_events_mask == -1) ? adjusted_subscribed_events_mask : (*module)->subscribed_events_mask;
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
module_shutdown()
{
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
streams_get_all(TSN_Streams **streams)
{
    ret = sysrepo_get_all_streams(streams);
    return ret;
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
        free(devices->enddevices[i].app_ref);
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

void application_put_apps(TSN_Apps *apps)
{
    int i;

    if (!apps)
        return;

    for (i = 0; i < apps->count_apps; ++i)
        application_app_put(&apps->apps[i]);

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
    free(app);
}
