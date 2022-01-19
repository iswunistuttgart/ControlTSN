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
int sysrepo_start_listening();
int sysrepo_stop_listening();
//int sysrepo_save_to_startup();

// -------------------------------------------------------- //
//  Callbacks
// -------------------------------------------------------- //
void sysrepo_init_callback(uint32_t subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data));
int sysrepo_send_notification(uint32_t event_id, char *entry_id, char *msg);


// -------------------------------------------------------- //
//  Module handling
// -------------------------------------------------------- //
/**
 * @brief Adds a new module to the list of available modules in the datastore. 
 * Before that it is checked if the module already exists. 
 * @param module The module to add (the id of the module is determined automatically and ignored if already set)
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_add_module(TSN_Module **module);

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

/**
 * @brief Returns all stored modules (contains the available and the registered).
 * @param modules The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_all_modules(TSN_Modules **modules);

/**
 * @brief Gets a specific module from the list of modules.
 * @param module_id The id of the module to get
 * @param module The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_module(int module_id, TSN_Module **module);

/**
 * @brief Removes a module from the list of available modules in the datastore. The function also unregisters the module. 
 * @param module The module to remove
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_delete_module(int module_id);

/**
 * @brief Sets the PID of a specific module.
 * @param module_id The id of the module
 * @param pid The PID
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_set_module_pid(int module_id, pid_t pid);

/**
 * @brief Gets the stored data of a specific module.
 * @param module_id The id of the module
 * @param data The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1) 
 */
int sysrepo_get_module_data(int module_id, TSN_Module_Data **data);

/**
 * @brief Updates the data of a specific module in the datastore.
 * @param module_id The id of the module
 * @param data The data to write to the sysrepo
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_update_module_data(int module_id, TSN_Module_Data data);

/**
 * @brief Updates the attributes of a specific module in the datastore. 
 * Only the attributes of passed parameters that are not NULL will be updated. 
 * @param module_id The id of the module
 * @param name The new name or NULL
 * @param description The new description or NULL
 * @param path The new path or NULL
 * @param subscribed_events_mask The new subscribed events mask or NULL
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_update_module_attributes(int module_id, const char *name, const char *description, const char *path, const uint32_t subscribed_events_mask);


// -------------------------------------------------------- //
// Stream handling
// -------------------------------------------------------- //
/**
 * @brief Returns all stored streams.
 * @param streams The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_all_streams(TSN_Streams **streams);

/**
 * @brief Writes the request information for a new stream to the datastore.
 * @param request The struct containing the request information
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_write_stream_request(TSN_Request *request);

/**
 * @brief Writes the computed configuration to an existing stream in the datastore.
 * @param stream_id The ID of the stream for which the configuration was calculated
 * @param configuration The computed configuration
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_write_stream_configuration(char *stream_id, TSN_Configuration *configuration);

/**
 * @brief Gets a specific stream from the list of streams.
 * @param stream_id The unique ID of the stream
 * @param stream The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_stream(char *stream_id, TSN_Stream **stream);


// -------------------------------------------------------- //
// Topology handling
// -------------------------------------------------------- //
/**
 * @brief Returns the stored topology containing all devices and the connections.
 * @param topology The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_topology(TSN_Topology **topology);
/**
 * @brief Writes the topology to the datastore.
 * @param topology The struct containing the devices and connections
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_set_topology(TSN_Topology *topology);
/**
 * @brief Returns all stored devices.
 * @param devices The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_all_devices(TSN_Devices **devices);
/**
 * @brief Returns the graph of the topology containing all connections.
 * @param graph The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_topology_graph(TSN_Graph **graph);
/**
 * @brief Triggers the discovery of the topology by setting the corresponding flag in the datastore.
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */

// -------------------------------------------------------- //
// Application handling
// -------------------------------------------------------- //
/**
 * @brief Returns the stored application containing all apps and images.
 * @param applications The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_application(TSN_Application **application);
/**
 * @brief Returns all stored apps.
 * @param apps The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_application_apps(TSN_Apps **apps);
/**
 * @brief Returns all stored images.
 * @param images The struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_application_images(TSN_Images **images);



#endif // __SYSREPO_CLIENT_H__