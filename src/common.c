#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>     // fork
#include <sys/types.h>  // pid_t
//#include <sys/wait.h>   // wait

#include "common.h"

int ret;


// ----------------------------------------------
//      FUNCTIONS - Module-Handling
// ----------------------------------------------
/*
int 
module_init_BACKUP(TSN_Module *this_module)
{
    // Establish a connection to sysrepo
    ret = sysrepo_connect();

    if (this_module) {
        // Add module to the available modules
        // TODO!! This step is not necessary and should be done (e.g.) by the engineering tool
        //int rc = sysrepo_add_or_get_module(&this_module);
        //if (rc != EXIT_SUCCESS) {
        //    printf("[COMMON] Module already known or error adding to the datastore!\n");
        //}

        // Setting the generic callback method
        sysrepo_init_callback(this_module->subscribed_events_mask, this_module->cb_event);

        // Start listening to notifications
        ret = sysrepo_start_listening();
    }

    return ret;
}
*/

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
    /*
    for (int i=0; i<all_modules->count_registered_modules; ++i) {
        if (strcmp(module_name, all_modules->registered_modules[i].name) == 0) {
            // Found the module
            (*module) = &all_modules->registered_modules[i];

            // Setting the generic callback method
            uint32_t mask = (adjusted_subscribed_events_mask == -1) ? adjusted_subscribed_events_mask : (*module)->subscribed_events_mask;
            sysrepo_init_callback(mask, cb_event);

            // Start listening to notifications
            ret = sysrepo_start_listening();

            goto cleanup;
        }
    }
    */
    for (int i=0; i<all_modules->count_modules; ++i) {
        if (strcmp(module_name, all_modules->modules[i].name) == 0) {
            // Found the module
            (*module) = &all_modules->modules[i];

            // Setting the generic callback method
            uint32_t mask = (adjusted_subscribed_events_mask == -1) ? adjusted_subscribed_events_mask : (*module)->subscribed_events_mask;
            sysrepo_init_callback(mask, cb_event);

            // Start listening to notifications
            ret = sysrepo_start_listening();

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

/*
int
module_get_registered(int module_id, TSN_Module **module)
{
    ret = sysrepo_get_module_from_registered(module_id, module);
    return ret;
}

int
module_get_available(int module_id, TSN_Module **module)
{
    ret = sysrepo_get_module_from_available(module_id, module);
    return ret;
}
*/

int 
module_get_id(int module_id, TSN_Modules **module)
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

    // Write the PID to sysrepo
    ret = sysrepo_set_module_pid(module->id, pid);
    if (ret) {
        printf("[COMMON] Error writing PID (%d) to module '%s' with ID %d!\n", pid, module->name, module->id);
        return EXIT_FAILURE;
    }
    printf("[COMMON] Successfully started module '%s' with PID %d\n", module->name, pid);
    
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


// ----------------------------------------------
//      FUNCTIONS - Stream
// ----------------------------------------------
int 
streams_get_all(TSN_Streams **streams)
{
    ret = sysrepo_get_all_streams(streams);
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

int
topology_get_graph(TSN_Graph **graph)
{
    ret = sysrepo_get_topology_graph(graph);
    return ret;
}

int
topology_trigger_discovery()
{
    
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

int
application_get_images(TSN_Images **images)
{
    ret = sysrepo_get_application_images(images);
    return ret;
}



/*
BACKUP

// ----------------------------------------------
// Module organization & orchestration
// ----------------------------------------------
static int modules_id_counter;
static int modules_count;
static TSN_Module *modules;

TSN_Module 
module_register(char *name,
                           char *description,
                           char *path,
                           int subscribed_events_mask,
                           void (*cb_event)(int event_id, TSN_Event_CB_Data event_data))
{
    modules_id_counter += 1;

    
    // TSN_Module *mod = malloc(sizeof(TSN_Module));
    // mod->name = strdup(name);
    // mod->description = strdup(description);
    // mod->id = modules_id_counter;
    // mod->cb_event = cb_event;

    // modules_count += 1;
    // if (modules_count == 1)
    // {
    //     modules = (TSN_Module *)malloc(1 * sizeof(TSN_Module));
    // }
    // else
    // {
    //     modules = (TSN_Module *)realloc(modules, modules_count * sizeof(TSN_Module));
    // }
    // modules[modules_count - 1] = *mod;

    // return modules_id_counter;
    

    TSN_Module mod;
    mod.name = strdup(name);
    mod.description = strdup(description);
    mod.path = strdup(path);
    mod.id = modules_id_counter;
    mod.subscribed_events_mask = subscribed_events_mask;
    mod.cb_event = cb_event;
    mod.p_id = -1;

    modules_count += 1;
    if (modules_count == 1)
    {
        modules = (TSN_Module *)malloc(1 * sizeof(TSN_Module));
    }
    else
    {
        modules = (TSN_Module *)realloc(modules, modules_count * sizeof(TSN_Module));
    }
    modules[modules_count - 1] = mod;

    return mod;
}

int 
module_unregister(int module_id)
{
    for (int i = 0; i < modules_count; ++i)
    {
        if (modules[i].id == module_id)
        {
            // Remove element
            modules_count -= 1;
            for (int j = i; j < modules_count; ++j)
            {
                modules[j] = modules[j + 1];
            }
            modules = (TSN_Module *)realloc(modules, modules_count * sizeof(TSN_Module));
            return (modules == NULL) ? EXIT_FAILURE : EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}
*/

/*
TSN_Module *
module_get(int module_id)
{
    for (int i = 0; i < modules_count; ++i)
    {
        if (modules[i].id == module_id)
        {
            return &modules[i];
        }
    }

    return NULL;
}

TSN_Module *
module_get_all(int *count)
{
    (*count) = modules_count;
    return modules;
}
*/

/*
int 
module_start(TSN_Module module)
{
    pid_t pid = fork();
    if (pid == 0) {
        //static char *argv[] = {"echo", "Foo is my name.", NULL};
        //execv("/bin/echo", argv);
        static char *argv[] = {};
        execv(module.path, argv);
        
    } else {
        //waitpid(pid, 0, 0);
    }

    return pid;
}

int 
module_stop(TSN_Module module)
{
    int res;
    if (module.p_id > 0) {
        res = kill(module.p_id, SIGKILL);
    }

    return res == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

// ----------------------------------------------
// Core callback methods
// ----------------------------------------------
static void
cb_stream_requested(TSN_Stream *stream)
{
    printf("[CORE][CALLBACK] Stream requested (%s)\n", stream->stream_id);
}

// ----------------------------------------------
// Core initialization and shutdown
// ----------------------------------------------
static int
_core_init()
{
    // Establish Sysrepo Connection
    ret = sysrepo_connect();
    if (ret == EXIT_FAILURE)
    {
        printf("[CORE] Failure while connecting to Sysrepo!\n");
        return ret;
    }
    printf("[CORE] Connected to Sysrepo!\n");

    // Init the callbacks in sysrepo
    sysrepo_init_callbacks(cb_stream_requested,
                           NULL,
                           NULL);

    // Start the subscription for notification from sysrepo
    // TODO uncomment below
    //ret = sysrepo_start_listening();

    return ret;
}

static int
_core_shutdown()
{
    // Stop the sysrepo subscription
    ret = sysrepo_stop_listening();
    if (ret == EXIT_FAILURE)
    {
        return ret;
    }

    // Disconnect from sysrepo
    ret = sysrepo_disconnect();

    return ret;
}

// ----------------------------------------------
// Helpers
// ----------------------------------------------
void print_module(TSN_Module mod)
{
    printf("----- MODULE -----\n");
    printf("Name:        %s\n", mod.name);
    printf("Description: %s\n", mod.description);
    printf("ID:          %d\n", mod.id);
    printf("P_ID:        %d\n", mod.p_id);
    printf("Path:        %s\n", mod.path);
    printf("Events Mask: %d\n", mod.subscribed_events_mask);
    printf("\n");
}

// ----------------------------------------------
// Sysrepo interface
// ----------------------------------------------
// MODULES

int 
get_modules_from_sysrepo(TSN_Modules **modules)
{
    
}
*/
