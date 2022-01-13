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
const char *CNC_INTERFACE_COMPUTE_REQUEST   = "/streams/compute_request";
const char *CNC_INTERFACE_COMPUTE_REQUESTS  = "/streams/compute_requests";

void cnc_discover_topology();
void cnc_compute_request(TSN_Stream *stream);
void cnc_compute_requests(TSN_Streams *streams);


// ------------------------------------
// Endpoints Interface
// ------------------------------------
const char *ENDPOINT_INTERFACE_DEPLOY_CONIFUGRATION = "/deploy_configuration";

#endif // __MODULE_CUC_H__