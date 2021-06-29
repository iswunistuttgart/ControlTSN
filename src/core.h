#ifndef __CORE_H__
#define __CORE_H__

#include <stdio.h>
#include "type_definitions.h"

// ----------------------------------------------
// Module organization & orchestration
// ----------------------------------------------
typedef struct TSN_Module
{
    int id;             // Module ID
    char *name;         // Module Name
    char *description;  // Module Description

    void (*cb_stream_requested)(TSN_Stream *);  // Callback for a requested stream
    void (*cb_stream_configured)(TSN_Stream *); // Callback for a configured stream
    // ...
    void (*cb_error)(int)  ;                    // Callback for a occured error
} TSN_Module;

/**
 * @brief Registers a module in the core.
 * @param name The name of the module.
 * @param description The description of the module.
 * @param cb_stream_requested Callback for a requested stream.
 * @param cb_stream_requested Callback for a configured stream.
 * @param cb_stream_requested Callback for a occured error.
 * @return The assigned ID for the registered module.
 */
int module_register(char *name, char *description, 
    void (*cb_stream_requested)(TSN_Stream *),
    void (*cb_stream_configured)(TSN_Stream *),
    // ...
    void (*cb_error)(int));
/**
 * @brief Unregisters a module from the core.
 * @param module_id The id of the module.
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1).
 */
int module_unregister(int module_id);
TSN_Module *module_get(int module_id);
TSN_Module *module_get_all(int *count);


// ----------------------------------------------
// Sysrepo interface
// ----------------------------------------------


#endif // __CORE_H__