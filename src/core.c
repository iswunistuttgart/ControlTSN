#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "./sysrepo/sysrepo_client.h"

int ret;

// ----------------------------------------------
// Module organization & orchestration
// ----------------------------------------------
static int modules_id_counter;
static int modules_count;
static TSN_Module *modules;

int 
module_register(char *name, char *description, 
    void (*cb_stream_requested)(TSN_Stream *),
    void (*cb_stream_configured)(TSN_Stream *),
    // ...
    void (*cb_error)(int))
{
    modules_id_counter += 1;

    TSN_Module *mod = malloc(sizeof(TSN_Module));
    mod->name = strdup(name);
    mod->description = strdup(description);
    mod->id = modules_id_counter;
    mod->cb_stream_requested = cb_stream_requested;
    mod->cb_stream_configured = cb_stream_configured;
    // ...
    mod->cb_error = cb_error;

    modules_count += 1;
    if (modules_count == 1) {
        modules = (TSN_Module *) malloc(1 * sizeof(TSN_Module));
    }
    else {
        modules = (TSN_Module *) realloc(modules, modules_count * sizeof(TSN_Module));
    }
    modules[modules_count-1] = *mod;

    return modules_id_counter;
}

int
module_unregister(int module_id)
{
    for (int i=0; i<modules_count; ++i) {
        if (modules[i].id == module_id) {
            // Remove element
            modules_count -= 1;
            for (int j=i; j<modules_count; ++j) {
                modules[j] = modules[j+1];
            }
            modules = (TSN_Module *) realloc(modules, modules_count * sizeof(TSN_Module));
            return (modules == NULL) ? EXIT_FAILURE : EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

TSN_Module 
*module_get(int module_id)
{
    for (int i=0; i<modules_count; ++i) {
        if (modules[i].id == module_id) {
            return &modules[i];
        }
    }

    return NULL;
}

TSN_Module 
*module_get_all(int *count)
{
    (*count) = modules_count;
    return modules;
}


// ----------------------------------------------
// Core initialization and shutdown
// ----------------------------------------------
static int 
_core_init()
{
    // Establish Sysrepo Connection
    ret = sysrepo_connect();
    if (ret == EXIT_FAILURE) {
        return ret;
    }

    // Start the subscription for notification from sysrepo
    ret = sysrepo_start_listening();

    return ret;
}

static int
_core_shutdown()
{
    // Stop the sysrepo subscription
    ret = sysrepo_stop_listening();
    if (ret == EXIT_FAILURE) {
        return ret;
    }

    // Disconnect from sysrepo
    ret = sysrepo_disconnect();

    return ret;
}


// ----------------------------------------------
// MAIN
// ----------------------------------------------
void test_cb(int x) {
    printf("CB: %d\n", x);
}

int
main(void)
{
    int m_id = module_register("Hallo", "DescriptionModule", NULL, NULL, NULL);
    int m_id2 = module_register("Hallo2", "DescriptionModule2", NULL, NULL, NULL);
    int m_id3 = module_register("Hallo3", "DescriptionModule3", NULL, NULL, NULL);
    int m_id4 = module_register("Hallo4", "DescriptionModule4", NULL, NULL, NULL);
    int m_id5 = module_register("Hallo5", "DescriptionModule5", NULL, NULL, test_cb);

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
}