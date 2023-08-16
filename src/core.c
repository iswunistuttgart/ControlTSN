/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>     // fork
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // wait
#include "core.h"
#include "common.h"


static int ret;

volatile sig_atomic_t is_running = 1;

static void 
signal_handler(int signum)
{
    is_running = 0;
}


int main(void)
{

    // Signal handling
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);


    TSN_Modules *modules = NULL;
    TSN_Modules *modules_after = NULL;

    ret = module_connect();
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

    uint16_t count_registered_modules = 0;
    for (int i=0; i<modules->count_modules; ++i) {
        if (modules->modules[i].registered) {
            count_registered_modules += 1;
            printf("[MAIN] Starting module '%s' ...\n", modules->modules[i].name);
            ret = module_start(modules->modules[i].id);
            if (ret) {
                printf("[MAIN] Could not start module '%s' ...\n", modules->modules[i].name);
            }
        }
    }
    if (count_registered_modules == 0) {
        printf("[MAIN] No registered modules to start.\n");
    }


    // Keep running
    while (is_running) {
        sleep(1);
    }


    // Stop all modules when shutting down this main module
    printf("-------------------------------------------------------------\n");
    
    // Retrieve the stored modules again in case a module was added during execution
    modules_after = malloc(sizeof(TSN_Modules));
    ret = module_get_all(&modules_after);
    if (ret) {
        printf("[MAIN] Error reading modules from sysrepo (after execution)!\n");
        goto cleanup;
    }
    for (int i=0; i<modules_after->count_modules; ++i) {
        if (modules_after->modules[i].registered) {
            printf("[MAIN] Stopping module '%s' ...\n", modules_after->modules[i].name);
            ret = module_stop(modules_after->modules[i].id);
            if (ret) {
                printf("[MAIN] Could not stop module '%s' ...\n", modules_after->modules[i].name);
            }
        }
    }

    // Stop the plugin
    char buf[512];
    FILE *cmd_pipe = popen("pidof sysrepo-plugind", "r");
    fgets(buf, 512, cmd_pipe);
    pid_t plugin_pid = strtoul(buf, NULL, 10);
    pclose(cmd_pipe);
    if (plugin_pid > 0) {
        if (kill(plugin_pid, SIGTERM) != 0) {
            printf("[MAIN] Error stopping the Sysrepo Plugin with PID '%d'!\n", plugin_pid);
        } else {
            printf("[MAIN] Successfully stopped the Sysrepo Plugin with PID '%d'!\n", plugin_pid);
        }
    }

cleanup:
    free(modules);
    free(modules_after);

    return EXIT_SUCCESS;
}
