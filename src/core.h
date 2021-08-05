#ifndef __CORE_H__
#define __CORE_H__

#include <stdio.h>
#include "type_definitions.h"

// ----------------------------------------------
// Events
// ----------------------------------------------
#define EVENT_STREAM_REQUESTED  0x0001
#define EVENT_STREAM_CONFIGURED 0x0002
/// ...
#define EVENT_ERROR             0x0200

/**
 * @brief Registers a module in the core.
 * @param name The name of the module.
 * @param description The description of the module.
 * @param path The path to the executable
 * @param subscribed_events_mask The mask describing subscribed events
 * @param cb_event Generic callback method
 * @return The created struct of the module.
 */
TSN_Module module_register(char *name, 
    char *description, 
    char *path,
    int subscribed_events_mask,
    void (*cb_event)(int event_id, TSN_Event_CB_Data event_data));

/**
 * @brief Unregisters a module from the core.
 * @param module_id The id of the module.
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1).
 */
int module_unregister(int module_id);
TSN_Module *module_get(int module_id);
TSN_Module *module_get_all(int *count);

/**
 * @brief Method to start a module by executing it in a process.
 * @return The process id on success, otherwise -1;
 */
int module_start(TSN_Module module);
/**
 * @brief Method to stop a module.
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1).
 */
int module_stop(TSN_Module module);


// ----------------------------------------------
// Helpers
// ----------------------------------------------
void print_module(TSN_Module mod);

// ----------------------------------------------
// Sysrepo interface
// ----------------------------------------------


#endif // __CORE_H__