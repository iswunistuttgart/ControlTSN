#include <stdlib.h>
#include <string.h>
#include <signal.h>
//#include <unistd.h>     // fork
//#include <sys/types.h>  // pid_t
//#include <sys/wait.h>   // wait

#include "common.h"
#include "./sysrepo/sysrepo_client.h"

int ret;


// ----------------------------------------------
//      FUNCTIONS - Module-Handling
// ----------------------------------------------
int 
module_init(TSN_Module *this_module)
{
    // Establish a connection to sysrepo
    ret = sysrepo_connect();

    // Add module to the available modules
    int rc = sysrepo_add_new_module(*(this_module));
    if (rc != EXIT_SUCCESS) {
        printf("[COMMON] Module already known or error adding to the datastore!\n");
    }

    // Setting the generic callback method
    sysrepo_init_callback(this_module->cb_event);

    return ret;
}

int
module_register(int module_id)
{

}

int
module_unregister(int module_id)
{

}

int
module_get(int module_id, TSN_Module **module)
{

}

int module_get_all(TSN_Modules **modules)
{

}


// ----------------------------------------------
//      FUNCTIONS - Sysrepo
// ----------------------------------------------
int
sysrepo_update_module_data(int module_id, TSN_Module_Data *module_data)
{

}

int
sysrepo_get_module_data(int module_id, TSN_Module_Data **module_data)
{

}


// ----------------------------------------------
//      FUNCTIONS - Other/Helpers
// ----------------------------------------------
void
print_module(TSN_Module module) {
    printf("----- MODULE -----\n");
    printf("Name:       %s\n", module.name);
    printf("Description:    %s\n", module.description);
    printf("ID:             %d\n", module.id);
    printf("P_ID:           %d\n", module.p_id);
    printf("Path:           %s\n", module.path);
    printf("Events Mask:    %d\n", module.subscribed_events_mask);
    printf("\n");
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
