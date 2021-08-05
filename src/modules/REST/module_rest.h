#ifndef __MODULE_REST_H__
#define __MODULE_REST_H__

// 
// Based on Ulfius framework
// https://github.com/babelouest/ulfius
// 

// The port for the API
#define PORT 8080

// The endpoint urls
const char *ENDPOINT_INDEX              = "/";
const char *ENDPOINT_MODULES            = "/modules";
const char *ENDPOINT_MODULES_ID         = "/modules/:id";
const char *ENDPOINT_MODULES_ID_START   = "/modules/:id/start";
const char *ENDPOINT_MODULES_ID_STOP    = "/modules/:id/stop";
const char *ENDPOINT_MODULES_ID_REMOVE  = "/modules/:id/remove";
const char *ENDPOINT_MODULES_ADD        = "/modules/add";

const char *ENDPOINT_TOPOLOGY           = "/topology";
const char *ENDPOINT_TOPOLOGY_SCAN      = "/topology/scan";

const char *ENDPOINT_STREAMS            = "/streams";
const char *ENDPOINT_STREAMS_ID         = "/streams/:id";
const char *ENDPOINT_STREAMS_ID_MODIFY  = "/streams/:id/modify";
const char *ENDPOINT_STREAMS_ID_REMOVE  = "/streams/:id/remove";
const char *ENDPOINT_STREAMS_ADD        = "/streams/add";


#endif // __MODULE_REST_H__