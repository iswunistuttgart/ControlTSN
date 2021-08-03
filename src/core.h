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

// ----------------------------------------------
// Module organization & orchestration
// ----------------------------------------------
/**
 * @brief Data for the event of a requested stream
 */
typedef struct TSN_Event_Data_Stream_Requested
{
    char *stream_id;
} TSN_Event_Data_Stream_Requested;

/**
 * @brief Data for the event of a configured stream
 */
typedef struct TSN_Event_Data_Stream_Configured
{
    char *stream_id;
} TSN_Event_Data_Stream_Configured;

/**
 * @brief Data for the event of an occured error
 */
typedef struct TSN_Event_Data_Error
{
    int error_code;
    char *error_msg;
} TSN_Event_Data_Error;

/**
 * @brief Union containing the data for the various events
 */
typedef union TSN_Event_CB_Data
{
    TSN_Event_Data_Stream_Requested stream_requested;
    TSN_Event_Data_Stream_Configured stream_configured;
    TSN_Event_Data_Error error;
} TSN_Event_CB_Data;


typedef struct TSN_Module
{
    int id;             // Module ID
    int p_id;           // Module Process ID
    char *path;         // Path to the modules executable       (???)
    char *name;         // Module Name
    char *description;  // Module Description
    int subscribed_events_mask;     // Mask describing the relevant events for this module
    void (*cb_event)(int event_id, TSN_Event_CB_Data data);     // Generic callback method for events
} TSN_Module;

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