/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#ifndef __MODULE_CUC_H__
#define __MODULE_CUC_H__

#include "../../common.h"


// ------------------------------------
// Module Data Identifiers
// ------------------------------------
const char *MODULE_DATA_IDENTIFIER_CNC = "cnc_url";

// ------------------------------------
// CNC Interface
// TODO: Currently implemented as REST but needs to be NETCONF or RESTCONF (802.1Qdj)
// ------------------------------------
const char *CNC_INTERFACE_DISCOVER_TOPOLOGY = "/topology/discover_topology";
const char *CNC_INTERFACE_COMPUTE_REQUESTS  = "/streams/compute_requests";

// UNI uris
const char *CNC_INTERFACE_JOIN_LISTENER = "/streams/join_listener";
const char *CNC_INTERFACE_LEAVE_LISTENER = "/streams/leave_listener";
const char *CNC_INTERFACE_REMOVE_STREAM = "/streams/remove_stream";
const char *CNC_INTERFACE_UPDATE_STREAM = "/streams/update_stream";


void cnc_discover_topology();
void cnc_compute_requests(TSN_Streams *streams);

// UNI methods
void cnc_join_listener(char *stream_id, TSN_Listener *listener);
void cnc_leave_listener(char *stream_id, TSN_StatusListener *listener);
void cnc_remove_stream(char *stream_id);
void cnc_update_stream(char *stream_id, TSN_Stream *stream);

// ------------------------------------
// Endpoints Interface
// ------------------------------------

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
 * @param is_listener Whether the given enddevice is a listener of the stream or not
 * @param listener_nr If the configuration is for a listener, then this is the index of the listener in the array
 * @param stream The calculated stream from the CNC containing the request as well as the configuration
 * @param app_id The ID of the corresponding app to distinguish them in the datastore of the enddevice.
 * @return Success (0) or failure (1)
 */
int deploy_configuration(TSN_Enddevice *enddevice, bool is_listener, uint16_t listener_nr, TSN_Stream *stream, char *app_id);


#endif // __MODULE_CUC_H__