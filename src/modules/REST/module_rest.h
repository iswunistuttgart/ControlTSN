#ifndef __MODULE_REST_H__
#define __MODULE_REST_H__

// 
// Based on Ulfius framework
// https://github.com/babelouest/ulfius
// 

// The port for the API
#define PORT 8080
#define API_PREFIX ""

// ----------------------------------------------
//      The API endpoint urls
// ----------------------------------------------
const char *API_INDEX                  = "/";

// Modules
const char *API_MODULES                 = "/modules";
const char *API_MODULES_REGISTERD_ID    = "/modules/registered/:id";
const char *API_MODULES_AVAILABLE_ID    = "/modules/available/:id";
const char *API_MODULES_ID_START        = "/modules/:id/start";
const char *API_MODULES_ID_STOP         = "/modules/:id/stop";
const char *API_MODULES_ID_DELETE       = "/modules/:id/delete";
const char *API_MODULES_ID_REGISTER     = "/modules/:id/register";
const char *API_MODULES_ID_UNREGISTER   = "/modules/:id/unregister";
const char *API_MODULES_ID_DATA         = "/modules/:id/data";

// Topology
const char *API_TOPOLOGY            = "/topology";
const char *API_TOPOLOGY_DISCOVER   = "/topology/discover";

// Streams
const char *API_STREAMS         = "/streams";
const char *API_STREAMS_REQUEST = "/streams/request";

// Application
const char *API_APPLICATIONS                        = "/applications";
const char *API_APPLICATIONS_IMAGES                 = "/applications/images";
const char *API_APPLICATIONS_IMAGES_DISTRIBUTION    = "/applications/images/distribution";
const char *API_APPLICATIONS_START                  = "/applications/:id/start";
const char *API_APPLICATIONS_STOP                   = "/applications/:id/stop";


#endif // __MODULE_REST_H__