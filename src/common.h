#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include "type_definitions.h"
#include "sysrepo/sysrepo_client.h"


// ----------------------------------------------
//      EVENTS
// ----------------------------------------------
#define EVENT_ERROR                 0x00000001
// Stream related   
#define EVENT_STREAM_REQUESTED      0x00000002
#define EVENT_STREAM_CONFIGURED     0x00000004
//#define ...                       0x00000008
//#define ...                       0x00000010
//#define ...                       0x00000020
//#define ...                       0x00000040
// Enddevice related    
//#define ...                       0x00000080
//#define ...                       0x00000100
//#define ...                       0x00000200
//#define ...                       0x00000400
//#define ...                       0x00000800
//#define ...                       0x00001000
// Modules related  
#define EVENT_MODULE_ADDED          0x00002000
#define EVENT_MODULE_REGISTERED     0x00004000
#define EVENT_MODULE_DATA_UPDATED   0x00008000
#define EVENT_MODULE_UNREGISTERED   0x00010000
#define EVENT_MODULE_DELETED        0x00020000
//#define ...                       0x00040000
// .
// .
// .



// ----------------------------------------------
//      FUNCTIONS - Module-Handling
// ----------------------------------------------

/**
 * @brief Initializes a module by connecting to sysrepo and writing the passed 
 * module information to the datastore under the list of available modules.
 * @param this_module The module information
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_init(TSN_Module *this_module);

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
int module_get_registered(int module_id, TSN_Module **module);

/**
 * @brief Get a specific module based on the id from the list of available modules.
 * @param module_id The id of the module to get
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_available(int module_id, TSN_Module **module);

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
int module_get_id(int module_id, TSN_Modules **modules);


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


// ----------------------------------------------
//      FUNCTIONS - Other/Helpers
// ----------------------------------------------

/**
 * @brief Prints the data of a module to the console.
 * @param module The data of the module to print
 */
void print_module_data(TSN_Module_Data data);

/**
 * @brief Prints a module to the console.
 * @param module The module struct to print
 */
void print_module(TSN_Module module);


#endif // __COMMON_H__