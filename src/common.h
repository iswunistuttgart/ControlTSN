#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include "type_definitions.h"
#include "sysrepo/sysrepo_client.h"


// ----------------------------------------------
//      EVENTS
// ----------------------------------------------
#define EVENT_STREAM_REQUESTED  0x0001
#define EVENT_STREAM_CONFIGURED 0x0002
// .
// .
// .
#define EVENT_ERROR             0x0200


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
 * @brief Unregisters a module from the core.
 * @param module_id The id of the module to unregister
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_unregister(int module_id);

/**
 * @brief Get a specific module based on the id.
 * @param module_id The id of the module to get
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get(int module_id, TSN_Module **module);

/**
 * @brief Get all modules stored in the datastore
 * @param modules The TSN_Modules struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_all(TSN_Modules **modules);


// ----------------------------------------------
//      FUNCTIONS - Sysrepo
// ----------------------------------------------
/**
 * @brief Writes the module specific data to the corresponding place in the sysrepo.
 * @param module_id The id of the module for which the data is to be updated
 * @param module_data The module specific data
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_update_module_data(int module_id, TSN_Module_Data *module_data);

/**
 * @brief Get the module specific data of a module.
 * @param module_id The id of the module
 * @param module_data A pointer where the data will be written
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int sysrepo_get_module_data(int module_id, TSN_Module_Data **module_data);


// ----------------------------------------------
//      FUNCTIONS - Other/Helpers
// ----------------------------------------------
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