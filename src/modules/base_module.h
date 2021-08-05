#ifndef __BASE_MODULE_H__
#define __BASE_MODULE_H__

#include "../core.h"
#include "../sysrepo/sysrepo_client.h"

// ----------------------------------------------
// Module initialization
// ----------------------------------------------
TSN_Module this_module;

// ----------------------------------------------
// Module data handling
// ----------------------------------------------
/**
 * @brief Get the data of this module. Called from the core.
 * @return Pointer of the data of unknown type.
 */
void *get_module_data();

/**
 * @brief Set data of this module. Called from the core.
 * @param data A pointer to the data (of unknown type)
 */
void set_module_data(void *data);

#endif // __BASE_MODULE_H__