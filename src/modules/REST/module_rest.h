/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#ifndef __MODULE_REST_H__
#define __MODULE_REST_H__


// Module DATA
static const char *MODULE_DATA_EXPORT_REST_API_URL = "export_rest_api";

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

// Root
const char *API_ROOT                   = "/uni";

// Modules
const char *API_MODULES                 = "/modules";
//const char *API_MODULES_REGISTERD_ID    = "/modules/registered/:id";
//const char *API_MODULES_AVAILABLE_ID    = "/modules/available/:id";
const char *API_MODULES_ID              = "/modules/:id";
const char *API_MODULES_ADD             = "/modules/add";
const char *API_MODULES_ID_START        = "/modules/:id/start";
const char *API_MODULES_ID_STOP         = "/modules/:id/stop";
const char *API_MODULES_ID_DELETE       = "/modules/:id/delete";
const char *API_MODULES_ID_REGISTER     = "/modules/:id/register";
const char *API_MODULES_ID_UNREGISTER   = "/modules/:id/unregister";
const char *API_MODULES_ID_DATA         = "/modules/:id/data";
const char *API_MODULES_ID_UPDATE       = "/modules/:id/update";

// Topology
const char *API_TOPOLOGY            = "/topology";
const char *API_TOPOLOGY_DEVICES    = "/topology/devices";
const char *API_TOPOLOGY_GRAPH      = "/topology/graph";
const char *API_TOPOLOGY_DISCOVER   = "/topology/discover";

// Streams
const char *API_STREAMS                     = "/streams";
const char *API_STREAMS_ID_DELETE           = "/streams/:stream-id/delete";
const char *API_STREAMS_REQUEST             = "/streams/request";
const char *API_STREAMS_REQUEST_SIMPLIFIED  = "/streams/request-simplified";
const char *API_STREAMS_COMPUTE             = "/streams/compute";

// Application
const char *API_APPLICATION             = "/application";
const char *API_APPLICATION_DISCOVER	= "/application/discover";
const char *API_APPLICATION_APPS        = "/application/apps";
//const char *API_APPLICATION_APPS_DISTRIBUTE = "/application/apps/distribute";
const char *API_APPLICATION_IMAGES      = "/application/images";
const char *API_APPLICATION_APPS_CREATE = "/application/apps/create";
const char *API_APPLICATION_APPS_DELETE = "/application/apps/:id/delete";
const char *API_APPLICATION_APPS_START  = "/application/apps/:id/start";
const char *API_APPLICATION_APPS_STOP   = "/application/apps/:id/stop";
const char *API_APPLICATION_APPS_UPDATE = "/application/apps/:id/update";

// Configuration
const char *API_CONFIGURATION_DEPLOY = "/configuration/apps/:id/deploy";
const char *API_CONFIGURATION_UPDATE = "/configuration/apps/:id/update";
const char *API_CONFIGURATION_RUN_STATE = "/configuration/apps/:id/state";
const char *API_CONFIGURATION_TOGGLE_TXRX = "/configuration/apps/:id/toggle_txrx";

#endif // __MODULE_REST_H__
