#ifndef __SYSREPO_CLIENT_H__
#define __SYSREPO_CLIENT_H__

#include <sysrepo.h>
#include "../type_definitions.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// -------------------------------------------------------- //
//  Initialization and connection stuff
// -------------------------------------------------------- //
int sysrepo_connect();
int sysrepo_disconnect();
//int sysrepo_start_listening();
//int sysrepo_stop_listening();
//int sysrepo_save_to_startup();

// -------------------------------------------------------- //
//  Callbacks
// -------------------------------------------------------- //
void sysrepo_init_callback(void (*cb_event)(TSN_Event_CB_Data));
/*
void sysrepo_init_callbacks(
    void (*cb_stream_requested)(TSN_Stream *),
    void (*cb_stream_configured)(TSN_Stream *),
    // ...
    void (*cb_error)()
);
*/

// -------------------------------------------------------- //
//  Module handling
// -------------------------------------------------------- //
/**
 * @brief Adds a new module to the list of available modules in the datastore. 
 * Before that it is checked if the module already exists.
 * @param module The module to add (the id of the module is determined automatically and ignored if already set)
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_add_new_module(TSN_Module module);

/**
 * @brief Registers a module by adding it to the list of registered modules in the datastore. 
 * That means on the next ramp up it will be automatically started. 
 * Before that the function checks if the module is already registered.
 * @param module_id The id of the module to register (use the id stored under all-modules)
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_register_module(int module_id);

/**
 * @brief Unregisters a module by removing it from the list of registered modules in the datastore.
 * @param module_id The id of the module to unregister
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_unregister_module(int module_id);

int sysrepo_get_module_from_registered(int module_id, TSN_Module **module);

int sysrepo_get_module_from_all(int module_id, TSN_Module **module);

int sysrepo_get_all_modules(TSN_Modules **modules);

/**
 * @brief Removes a module from the list of available modules in the datastore. 
 * @param module The module to remove
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_delete_module(int module_id);

// -------------------------------------------------------- //
//  CRUD methods
// -------------------------------------------------------- //
// The yang module root containing the stream-list
//int sysrepo_get_root(TSN_Uni **root);

// Stream
// int sysrepo_write_stream(TSN_Stream stream);
// int sysrepo_read_stream(char *stream_id, TSN_Stream **stream);
// int sysrepo_delete_stream(char *stream_id);

// Modules
//int sysrepo_register_module(TSN_Module module);
//int sysrepo_add_available_module(TSN_Module module);
//int sysrepo_get_modules(TSN_Modules **modules);

// Topology


#endif // __SYSREPO_CLIENT_H__