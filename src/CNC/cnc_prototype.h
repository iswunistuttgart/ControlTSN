#ifndef __CNC_PROTOTYPE_H__
#define __CNC_PROTOTYPE_H__


#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


#define PORT 11067
#define API_PREFIX ""

const char *API_TOPOLOGY_DISCOVER_TOPOLOGY  = "/topology/discover_topology";
const char *API_STREAMS_COMPUTE_REQUEST     = "/streams/compute_request";
const char *API_STREAMS_COMPUTE_REQUESTS    = "/streams/compute_requests";


#endif // __CNC_PROTOTYPE_H__