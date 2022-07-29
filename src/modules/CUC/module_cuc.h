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

/*
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
//void deploy_configuration(TSN_App *app, TSN_Configuration *stream_configuration);


/**
 * @brief Deploys the calculated stream configuration to the endpoint where the corresponding application is deployed.
 * 
 * We assume that the end device has an interface (OPC UA Server) to receive the stream configurations. 
 * From this interface, the data is written to a sysrepo and the end device is configured accordingly. 
 * The applications can retrieve the desired information via an interface to the data storage.
 * 
 * Note: The same interface (OPC UA Server) is also used for transmitting the 
 * execution parameters (cycle_time, wcet, priorities, etc.) as well as for 
 * the application-specific parameters (e.g. axes count for a vPLC) running on the end device.
 * 
 * @param enddevice The enddevice the configuration will be deployed to
 * @param stream_configuration The calculated stream configuration from the CNC
 * @param app_id The ID of the corresponding app to distinguish them in the datastore of the enddevice.
 * @return Success (0) or failure (1)
 */
int deploy_configuration(TSN_Enddevice *enddevice, TSN_Configuration *stream_configuration, char *app_id);


#endif // __MODULE_CUC_H__