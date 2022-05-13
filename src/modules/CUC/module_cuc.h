#ifndef __MODULE_CUC_H__
#define __MODULE_CUC_H__

#include "../../common.h"


// ------------------------------------
// Module Data Identifiers
// ------------------------------------
const char *MODULE_DATA_IDENTIFIER_CNC = "cnc_url";

// ------------------------------------
// CNC Interface
// ------------------------------------
const char *CNC_INTERFACE_DISCOVER_TOPOLOGY = "/topology/discover_topology";
const char *CNC_INTERFACE_COMPUTE_REQUESTS  = "/streams/compute_requests";

void cnc_discover_topology();
void cnc_compute_requests(TSN_Streams *streams);


// ------------------------------------
// Endpoints Interface
// ------------------------------------
//const char *ENDPOINT_INTERFACE_DEPLOY_CONIFUGRATION = "/deploy_configuration";

const char *APP_PARAMETER_IDENTIFIER_OPCUA_ENDPOINT = "opcua_endpoint";
/**
 * @brief Deploys the calculated stream configuration to a specific application.\n.
 * 
 * At this point, I assume that the stream configuration is sent directly to the individual apps.
 * This means that each app provides a receiver for this. In the ControlTSN project,
 * this corresponds to an OPC UA server on which the configurations are written.
 * 
 * @param app The app the confoiguration belongs to
 * @param stream_configuration The calculated stream configuration from the CNC.
 * 
 */
void deploy_configuration(TSN_App *app, TSN_Configuration *stream_configuration);

#endif // __MODULE_CUC_H__