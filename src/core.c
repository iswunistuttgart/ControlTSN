#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // fork
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // wait
#include "core.h"
#include "./sysrepo/sysrepo_client.h"

int ret;

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

    /*
    TSN_Module *mod = malloc(sizeof(TSN_Module));
    mod->name = strdup(name);
    mod->description = strdup(description);
    mod->id = modules_id_counter;
    mod->cb_event = cb_event;

    modules_count += 1;
    if (modules_count == 1)
    {
        modules = (TSN_Module *)malloc(1 * sizeof(TSN_Module));
    }
    else
    {
        modules = (TSN_Module *)realloc(modules, modules_count * sizeof(TSN_Module));
    }
    modules[modules_count - 1] = *mod;

    return modules_id_counter;
    */

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
        return ret;
    }

    // Init the callbacks in sysrepo
    sysrepo_init_callbacks(cb_stream_requested,
                           NULL,
                           NULL);

    // Start the subscription for notification from sysrepo
    ret = sysrepo_start_listening();

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
// MAIN
// ----------------------------------------------
void test_cb(int x, TSN_Event_CB_Data data)
{
    printf("CB: %d\n", x);
}

int main(void)
{
    /*
    int m_id = module_register("Hallo", "DescriptionModule", 0, NULL);
    int m_id2 = module_register("Hallo2", "DescriptionModule2", 0, NULL);
    int m_id3 = module_register("Hallo3", "DescriptionModule3", 0, NULL);
    int m_id4 = module_register("Hallo4", "DescriptionModule4", 0, NULL);
    int subscribed_events = EVENT_STREAM_REQUESTED | EVENT_STREAM_CONFIGURED | EVENT_ERROR;
    int m_id5 = module_register("Hallo5", "DescriptionModule5", subscribed_events, test_cb);

    // Print Modules list
    printf("ALL Modules (%d): \n", modules_count);
    for (int i=0; i<modules_count; ++i) {
        printf("%s - %s - %d\n", modules[i].name, modules[i].description, modules[i].id);
    }

    int unreg_id = 4;
    int ret = module_unregister(unreg_id);
    printf("\n");
    printf("Unregister:\n");
    printf("Result from unregister ID %d: %s\n", unreg_id, ret ? "FAILURE" : "SUCCESS");

    // Print Modules list
    printf("ALL Modules (%d): \n", modules_count);
    for (int i=0; i<modules_count; ++i) {
        printf("%s - %s - %d\n", modules[i].name, modules[i].description, modules[i].id);
    }

    // Get Module 
    printf("\n");
    printf("GET:\n");
    TSN_Module *test = malloc(sizeof(TSN_Module));
    test = module_get(2);
    printf("%s - %s - %d\n", test->name, test->description, test->id);

    // Get All
    printf("\n");
    printf("GET ALL:\n");
    int count;
    TSN_Module *list;
    list = module_get_all(&count);
    printf("ALL Modules (%d): \n", count);
    for (int i=0; i<count; ++i) {
        printf("%s - %s - %d\n", list[i].name, list[i].description, list[i].id);
    }
    */

    TSN_Module module_rest;
    module_rest = module_register("Rest Module", "exposes a rest interface", "./Module_REST", 515, NULL);
    print_module(module_rest);

    // Start the REST Module
    int pid = module_start(module_rest);
    if (pid > 0) {
        module_rest.p_id = pid;
        printf("---> PID: %d\n", module_rest.p_id);
    }

    sleep(5);
    ret = module_stop(module_rest);
    printf("Stop module result: %d\n", ret);
}