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
 * @brief Deletes a specific module from the list of available modules. 
 * Before that, the module will be unregistered
 * @param module_id The id of the module to delete
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_delete(int module_id);

/**
 * @brief Starts a specific module by calling the executable defined by the path.
 * @param module_id The id of the module to start
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_start(int module_id);

/**
 * @brief Stops the execution of a specific module by terminating the process defined by the PID.
 * @param module_id The id of the module to start
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_stop(int module_id);

// ----------------------------------------------
//      FUNCTIONS - Sysrepo
// ----------------------------------------------
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





// ----------------------------------------------
// BACKUP
// ----------------------------------------------
/**
 * @brief Method to start a module by executing it in a process.
 * @return The process id on success, otherwise -1;
 */
// int module_start(TSN_Module module);
/**
 * @brief Method to stop a module.
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1).
 */
// int module_stop(TSN_Module module);

//int get_modules_from_sysrepo(TSN_Modules **modules);


#endif // __COMMON_H__