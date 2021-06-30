#ifndef __BASE_MODULE_H__
#define __BASE_MODULE_H__

#include "core.h"

// ----------------------------------------------
// Module initialization
// ----------------------------------------------
TSN_Module tsn_module;

// ----------------------------------------------
// Module data handling
// ----------------------------------------------
/**
 * @brief Get the data of this module. Called from the core.
 * @return Pointer of the data of unknown type.
 */
void *get_module_data();

#endif // __BASE_MODULE_H__