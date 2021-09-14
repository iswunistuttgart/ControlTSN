#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>     // fork
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // wait
#include "core.h"
#include "common.h"


int ret;

volatile sig_atomic_t is_running = 1;

static void 
signal_handler(int signum)
{
    is_running = 0;
}


void main(void)
{
    /*
    TSN_Modules *modules = NULL;

    TSN_Module *this_module = malloc(sizeof(TSN_Module));
    this_module->name = strdup("TestModul");
    this_module->description = strdup("This is a example description");
    this_module->path = strdup("./ThisModule");
    this_module->subscribed_events_mask = 0;

    ret = sysrepo_connect();
    ret = module_delete(1);

    // Initializing module 
    ret = module_init(this_module);
    if (ret) {
        printf("ERROR initializing module!\n");
        goto cleanup;
    }

    // Register module 
    ret = module_register(1);
    if (ret) {
        printf("ERROR registering module!\n");
        goto cleanup;
    }

    // Writing module data
    TSN_Module_Data data = {
        .count_entries = 1,
        .entries = (TSN_Module_Data_Entry *) malloc(sizeof(TSN_Module_Data_Entry) * data.count_entries)
    };
    TSN_Module_Data_Entry entry1 = {
        .name = strdup("TestValue"),
        .type = UINT16
    };

    TSN_Module_Data_Entry_Value val1;
    val1.uint16_val = 42;
    entry1.value = val1;
    data.entries[0] = entry1;

    ret = module_update_data(1, data);
    if (ret) {
        printf("ERROR updating module data!\n");
        goto cleanup;
    }

    TSN_Modules *m = malloc(sizeof(TSN_Modules));
    ret = module_get_all(&m);
    if (ret) {
        printf("ERROR getting all modules!\n");
        goto cleanup;
    }
    printf("\n##### AVAILABLE MODULES (%d):\n", m->count_available_modules);
    for (int i=0; i<m->count_available_modules; ++i) {
        print_module(m->available_modules[i]);
    }

    printf("##### REGISTERED MODULES (%d):\n", m->count_registered_modules);
    for (int i=0; i<m->count_registered_modules; ++i) {
        print_module(m->registered_modules[i]);
    }
    
    if (m->count_registered_modules > 0) {
        TSN_Module_Data *data_test = malloc(sizeof(TSN_Module_Data));
        printf("GETTING MODULE DATA FOR ID %d:\n", 1);
        int t_rc = module_get_data(1, &data_test);
        if (t_rc) {
            printf("ERROR getting module data!!\n");
        } else {
            print_module_data(*data_test);
        }
    }

    printf("\n\n");
    data.count_entries = 2;
    data.entries = (TSN_Module_Data_Entry *) malloc(sizeof(TSN_Module_Data_Entry) * data.count_entries);
    TSN_Module_Data_Entry entry2 = {
        .name = strdup("TestValue2"),
        .type = STRING
    };

    TSN_Module_Data_Entry_Value val2;
    val2.string_val = "Test Parameter";
    entry2.value = val2;
    data.entries[0] = entry1;
    data.entries[1] = entry2;

    ret = module_update_data(1, data);
    if (ret) {
        printf("ERROR updating module data!\n");
        goto cleanup;
    }
    TSN_Module_Data *data_test2 = malloc(sizeof(TSN_Module_Data));
    printf("GETTING MODULE DATA FOR ID %d:\n", 1);
    int t_rc = module_get_data(1, &data_test2);
    if (t_rc) {
        printf("ERROR getting module data!!\n");
    } else {
        print_module_data(*data_test2);
    }
    */


    /*
    ret = module_init(NULL);
    if (ret) {
        goto cleanup;
    }

    modules = malloc(sizeof(TSN_Modules));
    ret = module_get_all(&modules);
    if (ret) {
        goto cleanup;
    }

    // Start all registered modules (m->registered_modules)
    for (int i=0; i<modules->count_registered_modules; ++i) {
        // ...
        print_module(modules->registered_modules[i]);
    }
    */

    TSN_Modules *modules = NULL;

    // Init the Sysrepo connection by calling the module initialization 
    // function without a module struct to add
    ret = module_init(NULL);
    if (ret) {
        printf("[MAIN] Error initializing!\n");
        goto cleanup;
    }

    // Getting all stored modules from sysrepo
    modules = malloc(sizeof(TSN_Modules));
    ret = module_get_all(&modules);
    if (ret) {
        printf("[MAIN] Error reading modules from sysrepo!\n");
        goto cleanup;
    }

    // Start all registered modules
    if (modules->count_registered_modules > 0) {
        for (int i=0; i<modules->count_registered_modules; ++i) {
            printf("[MAIN] Starting module '%s' ...\n", modules->registered_modules[i].name);
        }

    } else {
        printf("[MAIN] No registered modules to start.\n");
    }

cleanup:
    free(modules);

    printf("[MAIN] Exiting Main module. The framework is now left to its own... \n");
    return;
}