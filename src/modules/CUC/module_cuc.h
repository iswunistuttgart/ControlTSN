#ifndef __MODULE_CUC_H__
#define __MODULE_CUC_H__


// ------------------------------------
// Module Data Identifiers
// ------------------------------------
const char *MODULE_DATA_IDENTIFIER_CNC = "CNC URL";

// ------------------------------------
// CNC Interface
// ------------------------------------
const char *CNC_INTERFACE_DISCOVER_TOPOLOGY = "/discover_topology";

void cnc_discover_topology();

#endif // __MODULE_CUC_H__