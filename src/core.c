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

int main(void)
{
    printf("[CORE] Started CORE\n");
    TSN_Modules *m = malloc(sizeof(TSN_Modules));
    
    ret = sysrepo_connect();
    ret = sysrepo_get_modules(&m);
    printf("[CORE]: %d\n", m->count_all_modules);

    // Start all registered modules (m->registered_modules)
    // ...
}