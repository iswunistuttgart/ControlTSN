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
    TSN_Modules *modules = NULL;

    
    TSN_Module *this_module = malloc(sizeof(TSN_Module));
    this_module->name = strdup("TestModul");
    this_module->description = strdup("This is a example description");
    this_module->path = strdup("./ThisModule");
    this_module->subscribed_events_mask = 0;
    TSN_Module_Data data;
    data.count_entries = 1;
    data.entries = (TSN_Module_Data_Entry *) malloc(sizeof(TSN_Module_Data_Entry) * data.count_entries);
    TSN_Module_Data_Entry entry1;
    entry1.name = strdup("TestValue");
    entry1.type = UINT16;
    TSN_Module_Data_Entry_Value val1;
    val1.uint16_val = 42;
    entry1.value = val1;
    data.entries[0] = entry1;
    this_module->data = data;


    ret = module_init(this_module);
    if (ret) {
        goto cleanup;
    }

    TSN_Modules *m = malloc(sizeof(TSN_Modules));
    ret = module_get_all(&m);
    if (ret) {
        goto cleanup;
    }

    for (int i=0; i<m->count_available_modules; ++i) {
        print_module(m->available_modules[i]);
        //ret = module_delete(m->available_modules[i].id);
    }
    


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

cleanup:
    free(modules);

    return;
}