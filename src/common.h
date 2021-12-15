#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include "type_definitions.h"
#include "sysrepo/sysrepo_client.h"


// ----------------------------------------------
//      FUNCTIONS - Module-Handling
// ----------------------------------------------
/**
 * @brief Connects the module to sysrepo
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_connect();

/**
 * @brief Initializes a module by connecting to sysrepo and writing the passed 
 * module information to the datastore under the list of available modules.
 * @param this_module The module information
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_init_BACKUP(TSN_Module *this_module);

/**
 * @brief Initializes a module by getting the stored module information from the registered modules
 * and writing the information to the passed struct.
 * @param module_name The name of the module to search for in the list of registered modules
 * @param module The struct for writing the gathered information
 * @param adjusted_subscribed_events_mask If > -1 this mask will be used instead of the stored one from the list of available modules
 * @param cb_event The events callback function of the module
 */
int module_init(char *module_name, TSN_Module **module, uint32_t adjusted_subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data));

/**
 * @brief Shutdown the module by disconnecting from sysrepo and stopping the subscription of notification.
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_shutdown();

/**
 * @brief Registers a module in the core.
 * @param module_id The id of the module to register
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_register(int module_id);

/**
 * @brief Unregisters a module from the core. Before that the function stops the module if necessary.
 * @param module_id The id of the module to unregister
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_unregister(int module_id);

/**
 * @brief Get a specific module based on the id from the list of registered modules.
 * @param module_id The id of the module to get
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
//int module_get_registered(int module_id, TSN_Module **module);

/**
 * @brief Get a specific module based on the id from the list of available modules.
 * @param module_id The id of the module to get
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
//int module_get_available(int module_id, TSN_Module **module);

/**
 * @brief Get all modules stored in the datastore
 * @param modules The TSN_Modules struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_all(TSN_Modules **modules);

/**
 * @brief Get a specific module stored in the datastore
 * @param module_id The id of the desired module
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_id(int module_id, TSN_Module **module);


/**
 * @brief Deletes a specific module from the list of available modules. 
 * Before that, the module will be unregistered
 * @param module_id The id of the module to delete
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_delete(int module_id);

/**
 * @brief Starts a specific module by calling the executable defined by the path.
 * @param module_id The ID of the module to start
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_start(int module_id);

/**
 * @brief Stops the execution of a specific module by terminating the process defined by the PID.
 * The function uses 'kill' with the SIGUSR1 signal.
 * @param module_id The id of the module to start
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_stop(int module_id);


/**
 * @brief Writes the module specific data to the corresponding place in the sysrepo.
 * @param module_id The id of the module for which the data is to be updated
 * @param module_data The module specific data
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_update_data(int module_id, TSN_Module_Data module_data);

/**
 * @brief Get the module specific data of a module.
 * @param module_id The id of the module
 * @param module_data A pointer where the data will be written
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_data(int module_id, TSN_Module_Data **module_data);

/**
 * @brief Find and return a specific entry from the module data.
 * @param module_data The whole module data struct
 * @param entry_name The name of the entry to search for
 * @return The specific entry or NULL
 */
TSN_Module_Data_Entry *module_get_data_entry(TSN_Module_Data *module_data, const char *entry_name);

// ----------------------------------------------
//      FUNCTIONS - Stream-Handling
// ----------------------------------------------
/**
 * @brief Get all streams stored in the datastore
 * @param streams The TSN_Streams struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int streams_get_all(TSN_Streams **streams);


// ----------------------------------------------
//      FUNCTIONS - Topology-Handling
// ----------------------------------------------
/**
 * @brief Get the complete topology information containing the devices and the connections.
 * @param topology The TSN_Topology struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int topology_get(TSN_Topology **topology);
/**
 * @brief Get all devices from the topology.
 * @param devices The TSN_Devices struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int topology_get_devices(TSN_Devices **devices);
/**
 * @brief Get the graph of the topology containing all connections.
 * @param graph The TSN_Graph struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int topology_get_graph(TSN_Graph **graph);
/**
 * @brief Triggers the discovery of the topology. The result can be captured by subscribing the corresponding event.
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int topology_trigger_discovery();

// ----------------------------------------------
//      FUNCTIONS - Application-Handling
// ----------------------------------------------
/**
 * @brief Get the application information containing all apps and images.
 * @param applications The TSN_Applications struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int application_get(TSN_Application **application);
/**
 * @brief Get the stored apps.
 * @param apps The TSN_Apps struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int application_get_apps(TSN_Apps **apps);
/**
 * @brief Get the stored images.
 * @param apps The TSN_Images struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int application_get_images(TSN_Images **images);

#endif // __COMMON_H__