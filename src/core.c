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

static void
init() 
{
    
}

void main(void)
{
    TSN_Module *this_module = malloc(sizeof(TSN_Module));
    this_module->name = strdup("TestModul");
    this_module->description = strdup("This is a example description");
    this_module->path = strdup("./ThisModule");
    this_module->subscribed_events_mask = 0;

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

    // Start all registered modules (m->registered_modules)
    for (int i=0; i<m->count_registered_modules; ++i) {
        // ...
    }

cleanup:
    return;
}