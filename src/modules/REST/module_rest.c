/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#include <stdio.h>
#include <ulfius.h>
#include <signal.h>
#include <string.h>

#include "../../common.h"
#include "../../logger.h"
#include "../../events_definitions.h"
#include "module_rest.h"
#include "../../helper/json_serializer.h"

static int rc;
volatile sig_atomic_t is_running = 1;

TSN_Module *this_module;
// Module Data
static const char *export_rest_api;

struct _u_instance server_instance;

// WebSocket test
struct _websocket_manager *_websocket_manager = NULL;


static void
signal_handler(int signum)
{
    is_running = 0;
}


// ------------------------------------
// Callback handler
// ------------------------------------
static void
_cb_event(TSN_Event_CB_Data data)
{
    // Sending events in the websocket if it's open
    if (_websocket_manager) {
        // TODO: Newer version of ulfius supports sending json message 'ulfius_websocket_send_json_message'
        json_t *serialized_data = serialize_event_cb_data(&data);
        char *event_dump = json_dumps(serialized_data, 0);

        Websocket_Message ws_msg = {
            .timestamp = time(NULL),
            .msg = event_dump
        };
        char *msg_dump = json_dumps(serialize_websocket_message(&ws_msg), 0);

        if (ulfius_websocket_send_message(_websocket_manager, U_WEBSOCKET_OPCODE_TEXT, strlen(msg_dump), msg_dump) != U_OK) {
            printf("[REST][WS]: Error sending websocket message\n");
        }
    }

    if (data.event_id == EVENT_ERROR) {
        //printf("[REST][CB] ERROR: Code %d - '%s'\n", data.error.error_code, data.error.error_msg);
        printf("[REST][CB] ERROR (%s): %s\n", data.entry_id, data.msg);
    }
    //else if (data.event_id == EVENT_TOPOLOGY_DISCOVERY_REQUESTED) {
    //    printf("[REST][CB] Topology discovery requested!\n");
    //}

    else if (data.event_id == EVENT_MODULE_EXPORT_DATA) {
        printf("[REST][CB] Export of data requested!\n");
        // Get Sysrepo data and send them to the URL that is set in the module data ('export_rest_api')
        TSN_Uni *uni = malloc(sizeof(TSN_Uni));
        rc = sysrepo_get_uni(&uni);
        if (rc != EXIT_SUCCESS) {
            printf("[REST][CB] Error reading stored data!\n");
            goto cleanup_export;
        } 

        // Make request to send the exported json data
        if (!export_rest_api) {
            printf("[REST][CB][ERROR]: No API for export specified!\n");
            goto cleanup_export;
        }

        
        struct _u_response response;
        struct _u_request request;
        ulfius_init_request(&request);
        ulfius_init_response(&response);
        request.http_url = strdup(export_rest_api);
        request.http_verb = strdup("POST");
        json_t *json_body = NULL;
        json_body = serialize_uni(uni);
        ulfius_set_json_body_request(&request, json_body);
        int ret = ulfius_send_http_request(&request, &response);
        if (ret == U_OK && response.status == 200) {
            printf("[REST][CB] Successfully exported data to '%s'!\n", request.http_url);
        } else {
            printf("[REST][CB][ERROR]: Failure sending data to API '%s'!\n", request.http_url);
        }
        ulfius_clean_response(&response);
        ulfius_clean_request(&request);

cleanup_export:
        free(uni);
    }

    return;
}


// ------------------------------------
// API Endpoint Callback Functions
// ------------------------------------
// INDEX
static int
_api_index_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *resp = "<style>td {padding-right: 20px; } th {text-align: left;} tr td:first-child {width: 350px;}</style><html><b>This is the ControlTSN REST Server.</b></br>" \
                       "Following endpoints are provided:</br></br>" \
                       "<table>" \
                       "<tr><th>URI</th><th>Type</th><th>Description</th><th>POST Body</th></tr>" \
                       // Index
                       "<tr><th>Index</th></tr>" \
                       "<tr><td><a href='/'>/</a></td><td>GET</td><td>This site</td></tr>" \
                       // Root
                       "<tr><th>Root</th></tr>" \
                       "<tr><td><a href='/uni'>/uni</a></td><td>GET</td><td>Get the root element of the stored data</td></tr>" \
                       // Modules
                       "<tr><th>Modules</th></tr>" \
                       "<tr><td><a href='/modules'>/modules</a></td><td>GET</td><td>Get all available and registered modules</td></tr>" \
                       //"<tr><td><a href='/modules/registered/1'>/modules/registered/:id</a></td><td>GET</td><td>Get a specific registered module based on the id</td></tr>"
                       //"<tr><td><a href='/modules/available/1'>/modules/available/:id</a></td><td>GET</td><td>Get a specific available module based on the id</td></tr>"
                       "<tr><td><a href='/modules/1'>/modules/:id</a></td><td>GET</td><td>Get a specific module based on the id</td></tr>" \
                       "<tr><td><a href='/modules/add'>/modules/add</a></td><td>POST</td><td>Add a new module to the list of available modules</td><td>name (string), description (string), path (string), subscribed_events_mask (int)</td></tr>" \
                       "<tr><td><a href='/modules/1/start'>/modules/:id/start</a></td><td>POST</td><td>Start a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/stop'>/modules/:id/stop</a></td><td>POST</td><td>Stop a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/delete'>/modules/:id/delete</a></td><td>POST</td><td>Delete a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/register'>/modules/:id/register</a></td><td>POST</td><td>Register a specific module (use 'mask' to adjust the subscribed events)</td><td>mask (int)</td></tr>" \
                       "<tr><td><a href='/modules/1/unregister'>/modules/:id/unregister</a></td><td>POST</td><td>Unregister a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/data'>/modules/:id/data</a></td><td>GET</td><td>Get the data of a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/data'>/modules/:id/data</a></td><td>POST</td><td>Sets the data of a specific module</td><td>count_entries (int), {name (string), type (TSN_Module_Data_Entry_Type), value (TSN_Module_Data_Entry_Value)}[]</td></tr>" \
                       "<tr><td><a href='/modules/1/update'>/modules/:id/update</a></td><td>POST</td><td>Update the attributes of a specific module</td><td>name (string), description (string), path (string), subscribed_events_mask (int)</td></tr>" \
                       // Topology
                       "<tr><th>Topology</th></tr>" \
                       "<tr><td><a href='/topology'>/topology</a></td><td>GET</td><td>Get the stored topology data</td></tr>" \
                       "<tr><td><a href='/topology/discover'>/topology/discover</a></td><td>POST</td><td>Trigger the topology discovery</td></tr>" \
                       "<tr><td><a href='/topology/devices'>/topology/devices</a></td><td>GET</td><td>Get the stored devices</td></tr>" \
                       "<tr><td><a href='/topology/devices/00-00-00-00-00-00/update'>/topology/devices/:mac/update</a></td><td>POST</td><td>Update the attributes of a specific enddevice</td><td>name (string), interface-uri (string)</td></tr>" \
                       "<tr><td><a href='/topology/graph'>/topology/graph</a></td><td>GET</td><td>Get the topology graph containing all connections</td></tr>" \
                       "<tr><td><a href='/topology/graph/update'>/topology/graph/update</a></td><td>POST</td><td>Update the topology graph with the modeled connections</td></tr>" \
                       // Streams
                       "<tr><th>Streams</th></tr>" \
                       "<tr><td><a href='/streams'>/streams</a></td><td>GET</td><td>Get all streams</td></tr>" \
                       "<tr><td><a href='/streams/00-00-00-00-00-00:00-01'>/streams/:stream-id</a></td><td>GET</td><td>Get a specific stream</td></tr>" \
                       "<tr><td><a href='/streams/00-00-00-00-00-00:00-01/delete'>/streams/:stream-id/delete</a></td><td>POST</td><td>Delete a specific stream</td></tr>" \
                       "<tr><td><a href='/streams/00-00-00-00-00-00:00-01/deploy'>/streams/:stream-id/deploy</a></td><td>POST</td><td>Deploy a specific stream configuration to the enddevices</td></tr>" \
                       "<tr><td><a href='/streams/request'>/streams/request</a></td><td>POST</td><td>Request a new stream</td><td>request (TSN_Request)</td></tr>" \
                       "<tr><td><a href='/streams/compute'>/streams/compute</a></td><td>POST</td><td>Trigger the computation of all stream requests</td><td></td></tr>" \
                       // Applications
                       "<tr><th>Applications</th></tr>" \
                       "<tr><td><a href='/application/discover'>/application/discover</a></td><td>POST</td><td>Request discovery of images and applications</td></tr>" \
                       "<tr><td><a href='/application'>/application</a></td><td>GET</td><td>Get the application containing all apps and images</td></tr>" \
                       "<tr><td><a href='/application/apps'>/application/apps</a></td><td>GET</td><td>Get all stored apps</td></tr>" \
                       "<tr><td><a href='/application/images'>/application/images</a></td><td>GET</td><td>Get all stored images</td></tr>" \
                       "<tr><td><a href='/application/apps/create'>/application/apps/create</a></td><td>POST</td><td>Create a specific app</td></tr>" \
                       "<tr><td><a href='/application/apps/:id/delete'>/application/apps/:id/delete</a></td><td>POST</td><td>Delete a specific app</td></tr>" \
                       "<tr><td><a href='/application/apps/:id/start'>/application/apps/:id/start</a></td><td>POST</td><td>Start a specific app</td></tr>" \
                       "<tr><td><a href='/application/apps/:id/stop'>/application/apps/:id/stop</a></td><td>POST</td><td>Stop a specific app</td></tr>" \
                       "<tr><td><a href='/application/apps/:id/update'>/application/apps/:id/update</a></td><td>POST</td><td>Update a specific app</td></tr>" \
                       // Configuration
                       "<tr><td><a href='/configuration/apps/:id/deploy'>/configuration/apps/:id/deploy</a></td><td>POST</td><td>Deploy initial parameters to specific app</td></tr>" \
                       "<tr><td><a href='/configuration/apps/:id/update'>/configuration/apps/:id/update</a></td><td>POST</td><td>Update parameters of specific app</td></tr>" \
                       "<tr><td><a href='/configuration/apps/:id/state'>/configuration/apps/:id/state</a></td><td>POST</td><td>Request App run state</td></tr>" \
                       //"<tr><td><a href='/configuration/apps/:id/toggle_txrx'>/configuration/apps/:id/toggle_txrx</a></td><td>POST</td><td>Toggle Send and Receive flag</td></tr>"
                       "<tr><td><a href='/configuration/stream/00-00-00-00-00-00:00-01/set_sendreceive'>/configuration/stream/:stream-id/set_sendreceive</a></td><td>POST</td><td>Set Send and Receive flag for a specific stream</td></tr>" \
                       "<tr><td><a href='/configuration/stream/00-00-00-00-00-00:00-01/get_sendreceive'>/configuration/stream/:stream-id/get_sendreceive</a></td><td>GET</td><td>Get the current Send and Receive flag for a specific stream</td></tr>" \
                       // Debug
                       "<tr><th>Debug</th></tr>" \
                       "<tr><td><a href='/debug/set_topology'>/debug/set_topology</a></td><td>GET</td><td>Set the topology</td></tr>" \
                       "<tr><td><a href='/debug/remove_topology'>/debug/remove_topology</a></td><td>GET</td><td>Remove the topology</td></tr>" \
                       "<tr><td><a href='/debug/websocket'>/debug/websocket</a></td><td>GET</td><td>Connect to WebSocket</td></tr>" \
                       "<tr><td><a href='/debug/trigger-event'>/debug/trigger-event</a></td><td>POST</td><td>Manual trigger an event</td><td>event_id (int), entry_id (string, optional)</td></tr>" \
                       "</table></html>";
    ulfius_set_string_body_response(response, 200, resp);

    return U_CALLBACK_CONTINUE;
}

// ------------------------------------
// Root
// ------------------------------------
static int
_api_uni_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get the root element from sysrepo
    TSN_Uni *uni = malloc(sizeof(TSN_Uni));
    rc = sysrepo_get_uni(&uni);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return data as JSON
    json_t *json_body = serialize_uni(uni);
    ulfius_set_json_body_response(response, 200, json_body);

    // Decrease the reference count so the jansson lib is able to release the resources
    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

// ------------------------------------
// Modules
// ------------------------------------
static int
_api_modules_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get all modules from sysrepo
    TSN_Modules *modules = malloc(sizeof(TSN_Modules));
    rc = module_get_all(&modules);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return modules as JSON
    json_t *json_body = serialize_modules(modules);
    ulfius_set_json_body_response(response, 200, json_body);

    // Decrease the reference count so the jansson lib is able to release the resources
    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

/*
static int
_api_modules_get_available_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    // Get the specific module from the list of available modules
    TSN_Module *module = malloc(sizeof(TSN_Module));
    rc = module_get_available(module_id, &module);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return module as JSON
    json_t *json_body = serialize_module(module);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_get_registered_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    // Get the specific module from the list of available modules
    TSN_Module *module = malloc(sizeof(TSN_Module));
    rc = module_get_registered(module_id, &module);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return module as JSON
    json_t *json_body = serialize_module(module);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}
*/

static int
_api_modules_get_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    // Get the specific module from the list of modules
    TSN_Module *module = malloc(sizeof(TSN_Module));
    rc = module_get_id(module_id, &module);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return module as JSON
    json_t *json_body = serialize_module(module);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}


static int
_api_modules_add(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get the post params
    json_error_t json_error;
    json_t *json_post_body = ulfius_get_json_body_request(request, &json_error);
    if (json_post_body == NULL) {
        //char *error_resp = malloc(160 * sizeof(char) + 1 + strlen("ERROR:  | "));
        //sprintf(error_resp, "ERROR: %s | %d", json_error.text, json_error.position);
        //ulfius_set_string_body_response(response, 200, error_resp);
        return U_CALLBACK_ERROR;
    }

    const char *name = json_string_value(json_object_get(json_post_body, "name"));
    const char *description = json_string_value(json_object_get(json_post_body, "description"));
    const char *path = json_string_value(json_object_get(json_post_body, "path"));
    //const uint32_t subscribed_events_mask = json_number_value(json_object_get(json_post_body, "subscribed_events_mask"));
    const uint64_t subscribed_events_mask = json_number_value(json_object_get(json_post_body, "subscribed_events_mask"));
    TSN_Module *module = malloc(sizeof(TSN_Module));

    if (name == NULL || strlen(name) <= 0
        || description == NULL || strlen(description) <= 0
        || path == NULL || strlen(path) <= 0
        || subscribed_events_mask < 0) {

        json_decref(json_post_body);
        return U_CALLBACK_ERROR;
    }

    module->name = strdup(name);
    module->description = strdup(description);
    module->path = strdup(path);
    module->subscribed_events_mask = subscribed_events_mask;

    json_decref(json_post_body);

    rc = sysrepo_add_module(&module);
    if (rc != EXIT_SUCCESS) {
        return U_CALLBACK_ERROR;
    }

    json_t *id_response = json_object();
    json_object_set_new(id_response, "id", json_integer(module->id));
    ulfius_set_json_body_response(response, 200, id_response);
    json_decref(id_response);

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_start(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    rc = module_start(module_id);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_stop(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    rc = module_stop(module_id);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_delete(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    rc = module_delete(module_id);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_register(const struct _u_request *request, struct _u_response *response, void *user_data)
{

    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    // Adjusting the subscribed events mask in the post parameters
    /*
    const char *adjusted_mask_param = u_map_get(request->map_post_body, "mask");
    int adjusted_mask = -1;
    if (adjusted_mask_param) {
        adjusted_mask = atoi(adjusted_mask_param);
    }
    */

    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    //int adjusted_mask = -1;
    //if (json_post_body) {
    //    adjusted_mask = json_number_value(json_object_get(json_post_body, "mask"));
    //}

    rc = module_register(module_id);
    json_decref(json_post_body);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_unregister(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    rc = module_unregister(module_id);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_get_data_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    TSN_Module_Data *data = NULL;
    data = malloc(sizeof(TSN_Module_Data));
    rc = module_get_data(module_id, &data);
    if (rc == EXIT_FAILURE) {
        free(data);
        return U_CALLBACK_ERROR;
    }

    // Return module data as JSON
    json_t *json_body = serialize_module_data(data);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);
    free(data);

    return U_CALLBACK_COMPLETE;
}

static int
_api_modules_set_data_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        return U_CALLBACK_ERROR;
    }

    TSN_Module_Data *module_data = deserialize_module_data(json_post_body);

    rc = sysrepo_update_module_data(module_id, *module_data);
    if (rc == EXIT_SUCCESS) {
        return U_CALLBACK_COMPLETE;
    }

    return U_CALLBACK_ERROR;
}

static int
_api_modules_update(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get ID from URL
    const char *param_id = u_map_get(request->map_url, "id");
    int module_id = atoi(param_id);
    if (module_id <= 0) {
        return U_CALLBACK_ERROR;
    }

    // Get the post params
    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        return U_CALLBACK_ERROR;
    }

    const char *name = json_string_value(json_object_get(json_post_body, "name"));
    const char *description = json_string_value(json_object_get(json_post_body, "description"));
    const char *path = json_string_value(json_object_get(json_post_body, "path"));
    //const uint32_t subscribed_events_mask = json_number_value(json_object_get(json_post_body, "subscribed_events_mask"));
    const uint64_t subscribed_events_mask = json_number_value(json_object_get(json_post_body, "subscribed_events_mask"));


    rc = sysrepo_update_module_attributes(module_id, name, description, path, subscribed_events_mask);
    json_decref(json_post_body);
    if (rc == EXIT_SUCCESS) {
        return U_CALLBACK_COMPLETE;
    }
    
    return U_CALLBACK_ERROR;
}

// ------------------------------------
// Streams
// ------------------------------------
static int
_api_streams_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get all streams from sysrepo
    TSN_Streams *streams = malloc(sizeof(TSN_Streams));
    rc = streams_get_all(&streams, 0);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return streams as JSON
    json_t *json_body = serialize_streams(streams);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_streams_get_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *url_id = u_map_get(request->map_url, "stream-id");
    if (url_id == NULL) {
        return U_CALLBACK_ERROR;
    }
    char *stream_id = strdup(url_id);
    TSN_Stream *stream = malloc(sizeof(TSN_Stream));
    rc = streams_get(stream_id, &stream);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return stream as JSON
    json_t *json_body = serialize_stream(stream);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}


static int
_api_streams_request(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get the stream request from the json body
    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        return U_CALLBACK_ERROR;
    }

    TSN_Request *req = deserialize_stream_request(json_object_get(json_post_body, "request"));

    // Write stream request to sysrepo
    char *generated_stream_id = NULL;
    rc = stream_request(req, &generated_stream_id);

    json_decref(json_post_body);
    if (rc == EXIT_SUCCESS) {
        // Return the stream id as response
        json_t *json_response = serialize_stream_id(generated_stream_id);
        ulfius_set_json_body_response(response, 200, json_response);

        return U_CALLBACK_COMPLETE;
    }

    return U_CALLBACK_ERROR;
}


/**
 * Prototype function to request a stream without knowing the full Qcc data model.
 * Currently only a test. Needs to be evaluated if this function is necessary and what interface it has.
 */
static int
_api_streams_request_simplified(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get the stream request from the json body
    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        return U_CALLBACK_ERROR;
    }

    // User input contains the talker and listener(s), the traffic specification and the qos for the talker and the listeners
    TSN_Enddevice *talker_device = deserialize_enddevice(json_object_get(json_post_body, "talker"));
    json_t *listeners = json_object_get(json_post_body, "listener_list");
    uint16_t count_listeners = json_array_size(listeners);
    TSN_Enddevice *listener_devices = (TSN_Enddevice *) malloc(sizeof(TSN_Enddevice) * count_listeners);
    for (int i=0; i<count_listeners; ++i) {
        json_t *e = json_array_get(listeners, i);
        TSN_Enddevice *x = deserialize_enddevice(e);
        listener_devices[i] = *x;
    }
    IEEE_TrafficSpecification *traffic_spec = deserialize_traffic_specification(json_object_get(json_post_body, "traffic_specification"));
    IEEE_UserToNetworkRequirements *qos_talker = deserialize_user_to_network_requirements(json_object_get(json_post_body, "qos_talker"));
    json_t *qos_listener_list = json_object_get(json_post_body, "qos_listeners");
    IEEE_UserToNetworkRequirements *qos_listeners = (IEEE_UserToNetworkRequirements *) malloc(sizeof(IEEE_UserToNetworkRequirements) * count_listeners);
    for (int i=0; i<count_listeners; ++i) {
        json_t *e = json_array_get(qos_listener_list, i);
        IEEE_UserToNetworkRequirements *x = deserialize_user_to_network_requirements(e);
        qos_listeners[i] = *x;
    }

    TSN_Request req = create_stream_request(talker_device, count_listeners, listener_devices, traffic_spec, qos_talker, qos_listeners);


    // Write stream request to sysrepo
    char *generated_stream_id = NULL;
    rc = stream_request(&req, &generated_stream_id);

    json_decref(json_post_body);
    if (rc == EXIT_SUCCESS) {
        // Return the stream id as response
        json_t *json_response = serialize_stream_id(generated_stream_id);
        ulfius_set_json_body_response(response, 200, json_response);

        return U_CALLBACK_COMPLETE;
    }

    return U_CALLBACK_ERROR;
}

static int
_api_streams_delete(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *url_id = u_map_get(request->map_url, "stream-id");
    if (url_id == NULL) {
        return U_CALLBACK_ERROR;
    }
    char *stream_id = strdup(url_id);
    rc = streams_delete(stream_id);

    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_streams_deploy(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *url_id = u_map_get(request->map_url, "stream-id");
    if (url_id == NULL) {
        return U_CALLBACK_ERROR;
    }
    char *stream_id = strdup(url_id);
    rc = sysrepo_send_notification(EVENT_STREAM_DEPLOY, stream_id, "deploy stream configuration");
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_streams_compute(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    rc =sysrepo_send_notification(EVENT_STREAM_COMPUTATION_REQUESTED, NULL, "stream computation requested via REST module");
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

// ------------------------------------
// Topology
// ------------------------------------
static int
_api_topology_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get the complete topology information from sysrepo
    TSN_Topology *topology = malloc(sizeof(TSN_Topology));
    rc = topology_get(&topology);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return as JSON
    json_t *json_body = serialize_topology(topology);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_topology_devices_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Devices *devices = malloc(sizeof(TSN_Devices));
    rc = topology_get_devices(&devices);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    json_t *json_body = serialize_devices(devices);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_topology_devices_update(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *mac = u_map_get(request->map_url, "mac");
    json_t *json_post_body;
    TSN_Enddevice *enddevice;
    if (mac == NULL) {
        return U_CALLBACK_ERROR;
    }

    json_post_body = ulfius_get_json_body_request(request, NULL);
    if (!json_post_body) {
        return U_CALLBACK_ERROR;
    }

    enddevice = deserialize_enddevice(json_post_body);
    if (!enddevice) {
        return U_CALLBACK_ERROR;
    }

    rc = sysrepo_update_enddevice(enddevice);
    json_decref(json_post_body);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_topology_graph_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Graph *graph = malloc(sizeof(TSN_Graph));
    rc = topology_get_graph(&graph);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    json_t *json_body = serialize_graph(graph);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_topology_graph_update(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get the connections from the post body
    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        return U_CALLBACK_ERROR;
    }

    TSN_Graph *graph = malloc(sizeof(TSN_Graph));
    graph = deserialize_graph(json_post_body);

    rc = sysrepo_set_topology_graph(graph);
    json_decref(json_post_body);
    if (rc == EXIT_SUCCESS) {
        return U_CALLBACK_COMPLETE;
    }
    
    return U_CALLBACK_ERROR;
}

static int
_api_topology_discover(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    rc = sysrepo_send_notification(EVENT_TOPOLOGY_DISCOVERY_REQUESTED, NULL, "topology discovery requested via REST module");
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

// ------------------------------------
// Application
// ------------------------------------
static int
_api_application_discover_post(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    int ret;

    ret = sysrepo_send_notification(EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED, NULL, "Image discovery requested via REST module");
    if (ret == EXIT_FAILURE)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}

static int
_api_application_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Application *application = malloc(sizeof(TSN_Application));
    rc = application_get(&application);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    json_t *json_body = serialize_application(application);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_application_apps_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Apps *apps = malloc(sizeof(TSN_Apps));
    json_t *json_body = NULL;
    int ret;

    if (!apps)
        return U_CALLBACK_ERROR;
    memset(apps, '\0', sizeof(*apps));

    ret = application_get_apps(&apps);
    if (ret == EXIT_FAILURE) {
        ret = U_CALLBACK_ERROR;
        goto cleanup;
    }

    json_body = serialize_apps(apps);
    ulfius_set_json_body_response(response, 200, json_body);

    ret = U_CALLBACK_COMPLETE;

cleanup:
    if (json_body)
        json_decref(json_body);
    application_put_apps(apps);

    return ret;
}

static int
_api_application_images_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Images *images = malloc(sizeof(TSN_Images));
    json_t *json_body = NULL;
    int ret;

    if (!images)
        return U_CALLBACK_ERROR;
    memset(images, '\0', sizeof(*images));

    ret = application_get_images(&images);
    if (ret == EXIT_FAILURE) {
        ret = U_CALLBACK_ERROR;
        goto cleanup;
    }

    json_body = serialize_images(images);
    ulfius_set_json_body_response(response, 200, json_body);

    ret = U_CALLBACK_COMPLETE;

cleanup:
    if (json_body)
        json_decref(json_body);
    application_put_images(images);

    return ret;
}

static int
_api_application_app_create(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    //const char *app_id = u_map_get(request->map_url, "id");
    json_t *json_post_body;
    TSN_App *app;
    int ret;

    //if (!app_id)
    //    return U_CALLBACK_ERROR;

    json_post_body = ulfius_get_json_body_request(request, NULL);
    if (!json_post_body)
        return U_CALLBACK_ERROR;

    app = deserialize_app(json_post_body);
    if (!app)
        return U_CALLBACK_ERROR;

    ret = sysrepo_set_application_app(app);
    if (ret == EXIT_FAILURE) {
        ret = U_CALLBACK_ERROR;
        goto out;
    }

    ret = U_CALLBACK_COMPLETE;

out:
    json_decref(json_post_body);
    application_app_put(app);

    return ret;
}

static int
_api_application_app_delete(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *app_id = u_map_get(request->map_url, "id");
    int ret;

    if (!app_id)
        return U_CALLBACK_ERROR;

    ret = sysrepo_remove_application_app(app_id);
    if (ret != EXIT_SUCCESS)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}

static int
_api_application_app_start(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *app_id = u_map_get(request->map_url, "id");
    int ret;

    if (!app_id)
        return U_CALLBACK_ERROR;

    // Notify container module to start the app
    ret = sysrepo_send_notification(EVENT_APPLICATION_APP_START_REQUESTED, NULL, (char *)app_id);
    if (ret == EXIT_FAILURE)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}

static int
_api_application_app_stop(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *app_id = u_map_get(request->map_url, "id");
    int ret;

    if (!app_id)
        return U_CALLBACK_ERROR;

    // Notify container module to stop the app
    ret = sysrepo_send_notification(EVENT_APPLICATION_APP_STOP_REQUESTED, NULL, (char *)app_id);
    if (ret == EXIT_FAILURE)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}

static int
_api_application_app_update(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    /*
    const char *app_id = u_map_get(request->map_url, "id");
    json_t *json_post_body;
    TSN_App *app;
    int ret;


    if (!app_id)
        return U_CALLBACK_ERROR;

    json_post_body = ulfius_get_json_body_request(request, NULL);
    if (!json_post_body)
        return U_CALLBACK_ERROR;

    app = deserialize_app(json_post_body);
    if (!app)
        return U_CALLBACK_ERROR;

    ret = sysrepo_set_application_app(app);
    if (ret == EXIT_FAILURE) {
        ret = U_CALLBACK_ERROR;
        goto out;
    }

    ret = U_CALLBACK_COMPLETE;

out:
    json_decref(json_post_body);
    application_app_put(app);

    return ret;
    */

    const char *app_id = u_map_get(request->map_url, "id");
    json_t *json_post_body;
    TSN_App *app;
    if (app_id == NULL) {
        return U_CALLBACK_ERROR;
    }

    json_post_body = ulfius_get_json_body_request(request, NULL);
    if (!json_post_body) {
        return U_CALLBACK_ERROR;
    }
    app = deserialize_app(json_post_body);
    if (!app) {
        return U_CALLBACK_ERROR;
    }

    rc = sysrepo_update_app(app);
    json_decref(json_post_body);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

// ------------------------------------
// Configuration
// ------------------------------------

static int
_api_configuration_app_deploy(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *app_id = u_map_get(request->map_url, "id");
    int ret;

    if (!app_id)
        return U_CALLBACK_ERROR;

    // Notify configuration module to deploy the initial paramers to the app
    ret = sysrepo_send_notification(EVENT_CONFIGURATION_DEPLOY, NULL, (char *)app_id);
    if (ret == EXIT_FAILURE)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}

static int
_api_configuration_app_update(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *app_id = u_map_get(request->map_url, "id");
    int ret;

    if (!app_id)
        return U_CALLBACK_ERROR;

    // Notify configuration module to update the parameters of the app
    ret = sysrepo_send_notification(EVENT_CONFIGURATION_CHANGED, NULL, (char *)app_id);
    if (ret == EXIT_FAILURE)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}

/*
static int
_api_configuration_app_run_state(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *app_id = u_map_get(request->map_url, "id");
    int ret;

    if (!app_id)
        return U_CALLBACK_ERROR;

    // Notify configuration module to request the run state of the app
    ret = sysrepo_send_notification(EVENT_CONFIGURATION_REQUEST_RUN_STATE, NULL, (char *)app_id);
    if (ret == EXIT_FAILURE)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}
*/


/*
static int
_api_configuration_app_toggle_txrx(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *app_id = u_map_get(request->map_url, "id");
    int ret;

    if (!app_id)
        return U_CALLBACK_ERROR;

    // Notify configuration module to toggle send and receive of the app
    ret = sysrepo_send_notification(EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE, NULL, (char *)app_id);
    if (ret == EXIT_FAILURE)
        return U_CALLBACK_ERROR;

    return U_CALLBACK_COMPLETE;
}
*/

static int
_api_configuration_set_stream_sendreceive(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    //const char *app_id = u_map_get(request->map_url, "id");
    //int ret;
//
    //if (!app_id)
    //    return U_CALLBACK_ERROR;
//
    //// Notify configuration module to toggle send and receive of the app
    //ret = sysrepo_send_notification(EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE, NULL, (char *)app_id);
    //if (ret == EXIT_FAILURE)
    //    return U_CALLBACK_ERROR;

    
    const char *url_id = u_map_get(request->map_url, "stream-id");
    if (url_id == NULL) {
        return U_CALLBACK_ERROR;
    }
    char *stream_id = strdup(url_id);

    // Get the flag in the post body
    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        return U_CALLBACK_ERROR;
    }
    bool flag = json_boolean_value(json_object_get(json_post_body, "flag"));
    
    // Set event to ENABLE / DISABLE a specific stream
    if (flag) {
        rc = sysrepo_send_notification(EVENT_CONFIGURATION_ENABLE_STREAM_SEND_RECEIVE, stream_id, "Enable SendReceive of stream");
    } else {
        rc = sysrepo_send_notification(EVENT_CONFIGURATION_DISABLE_STREAM_SEND_RECEIVE, stream_id, "Disable SendReceive of stream");
    }
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

static int
_api_configuration_get_stream_sendreceive(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *url_id = u_map_get(request->map_url, "stream-id");
    if (url_id == NULL) {
        return U_CALLBACK_ERROR;
    }
    char *stream_id = strdup(url_id);

    // TODO: Should this be in the Configuration Module or is it fine to have it here?
    // The call should return a value therefore it isn't that easy to have the logic 
    // in the configuration module and communicate with them via events
    // --> Therefore we read the value directly from here
    bool enabled = configuration_stream_get_sendreceive(stream_id);

    json_t *json_body = json_object();
    json_object_set_new(json_body, "flag", json_boolean(enabled));
    ulfius_set_json_body_response(response, 200, json_body);


    return U_CALLBACK_COMPLETE;

}

// ------------------------------------
// Debug
// ------------------------------------

static int
_api_debug_set_topology(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Enddevice_AppRef *ar = NULL;
    TSN_Enddevice *e1 = NULL;
    TSN_Enddevice *e2 = NULL;
    TSN_Switch *s1 = NULL;
    TSN_Devices *d = NULL;
    TSN_Connection *c1 = NULL;
    TSN_Connection *c2 = NULL;
    TSN_Graph *g = NULL;
    TSN_Topology *t = NULL;

    // 1 Enddevice App ref
    ar = malloc(sizeof(TSN_Enddevice_AppRef));
    ar->app_ref = strdup("Test_0.0.1");

    // 2 Enddevices
    e1 = malloc(sizeof(TSN_Enddevice));
    e1->mac = strdup("00-00-00-00-00-01");
    e1->has_app = 1;
    e1->count_apps = 1;
    e1->apps = (TSN_Enddevice_AppRef *) malloc(sizeof(TSN_Enddevice_AppRef) * e1->count_apps);
    e1->apps[0] = *ar;

    //e1->app_ref = NULL;
    e2 = malloc(sizeof(TSN_Enddevice));
    e2->mac = strdup("00-00-00-00-00-02");
    e2->has_app = 0;
    //e2->app_ref = NULL;

    // 1 Switch
    s1 = malloc(sizeof(TSN_Switch));
    s1->mac = strdup("01-02-03-04-05-06");
    srand(time(NULL));
    int test = rand() % 100;
    printf("Random: %d\n", test);
    s1->ports_count = test;
    //s1->ports_count = 2;

    d = malloc(sizeof(TSN_Devices));
    d->count_enddevices = 2;
    d->enddevices = (TSN_Enddevice *) malloc(sizeof(TSN_Enddevice) * d->count_enddevices);
    d->enddevices[0] = *e1;
    d->enddevices[1] = *e2;
    d->count_switches = 1;
    d->switches = (TSN_Switch *) malloc(sizeof(TSN_Switch) * d->count_switches);
    d->switches[0] = *s1;

    // 2 Connections (the two enddevices are connected through the switch)
    c1 = malloc(sizeof(TSN_Connection));
    c1->id = 1;
    c1->from_mac = e1->mac;
    c1->from_port = 1;
    c1->to_mac = s1->mac;
    c1->to_port = 1;
    c2 = malloc(sizeof(TSN_Connection));
    c2->id = 2;
    c2->from_mac = s1->mac;
    c2->from_port = 2;
    c2->to_mac = e2->mac;
    c2->to_port = 1;

    g = malloc(sizeof(TSN_Graph));
    g->count_connections = 2;
    g->connections = (TSN_Connection *) malloc(sizeof(TSN_Connection) * g->count_connections);
    g->connections[0] = *c1;
    g->connections[1] = *c2;

    t = malloc(sizeof(TSN_Topology));
    t->devices = *d;
    t->graph = *g;


    rc = sysrepo_set_topology(t);
    if (rc == EXIT_FAILURE) {
        printf("FEHLER BEIM SCHREIBEN DER TOPOLOGY!\n");
        goto cleanup;
    }

cleanup:
    free(e1);
    free(e2);
    free(s1);
    free(d);
    free(c1);
    free(c2);
    free(g);
    free(t);
    return rc ? U_CALLBACK_ERROR : U_CALLBACK_COMPLETE;
}

static int
_api_debug_remove_topology(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    rc = sysrepo_set_topology(NULL);
    if (rc == EXIT_FAILURE) {
        printf("FEHLER BEIM SCHREIBEN DER TOPOLOGY!\n");
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

void
_websocket_manager_cb(const struct _u_request *request, struct _websocket_manager *websocket_manager, void *websocket_manager_user_data)
{
    /*
    if (websocket_manager_user_data != NULL) {
        printf("[REST][WS]: websocket_manager_user_data is %s\n", websocket_manager_user_data);
    }

    if (ulfius_websocket_wait_close(websocket_manager, 2000) == U_WEBSOCKET_STATUS_OPEN) {
        if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, strlen("Message without fragmentation from server"), "Message without fragmentation from server") != U_OK) {
            printf("[REST][WS]: Error send message without fragmentation\n");
        }
        if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, strlen("Message without fragmentation from server #2"), "Message without fragmentation from server #2") != U_OK) {
            printf("[REST][WS]: Error send message without fragmentation\n");
        }
    }

    printf("[REST][WS]: Closing websocket_manager_callback\n");
    */

    _websocket_manager = websocket_manager;

    while(is_running) {
        /*
        Websocket_Message ws_msg = {
            .timestamp = time(NULL),
            .msg = "MSG from server"
        };
        char *msg_dump = json_dumps(serialize_websocket_message(&ws_msg), 0);

        if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, strlen(msg_dump), msg_dump) != U_OK) {
            printf("[REST][WS]: Error send message without fragmentation\n");
        }
        */
        sleep(1);
    }
}

void
_websocket_incoming_cb(const struct _u_request *request, struct _websocket_manager *websocket_manager, const struct _websocket_message *last_message, void *websocket_incoming_message_user_data)
{
    /*
    if (websocket_incoming_message_user_data != NULL) {
        printf("[REST][WS]: websocket_manager_user_data is %s\n", websocket_incoming_message_user_data);
    }
    printf("[REST][WS]: Incoming message, opcode: 0x%02x, mask: %d, len: %zu\n", last_message->opcode, last_message->has_mask, last_message->data_len);
    if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
        printf("[REST][WS]: text payload '%.*s'\n", last_message->data_len, last_message->data);
    } else if (last_message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
        printf("[REST][WS]: binary payload\n");
    }
    */
}

void
_websocket_onclose_cb(const struct _u_request *request, struct _websocket_manager *websocket_manager, void *websocket_onclose_user_data)
{
    printf("[REST][WS]: Closing WebSocket connection\n");
}

static int
_api_debug_websocket(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    int ret;

    if ((ret = ulfius_set_websocket_response(response, NULL, NULL, &_websocket_manager_cb, NULL, &_websocket_incoming_cb, NULL, &_websocket_onclose_cb, NULL)) == U_OK) {
        return U_CALLBACK_CONTINUE;
    } else {
        return U_CALLBACK_ERROR;
    }
}

static int
_api_debug_trigger_event(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    json_t *json_post_body;
    int ret;

    json_post_body = ulfius_get_json_body_request(request, NULL);
    if (!json_post_body) {
        return U_CALLBACK_ERROR;
    }

    //uint32_t event_id = json_integer_value(json_object_get(json_post_body, "event_id"));
    uint64_t event_id = json_integer_value(json_object_get(json_post_body, "event_id"));
    const char *entry_id_key = json_string_value(json_object_get(json_post_body, "entry_id"));
    char *entry_id = NULL;
    if (entry_id_key) {
        entry_id = strdup(entry_id_key);
    }

    ret = sysrepo_send_notification(event_id, entry_id, "Manual triggered event");
    if (ret == EXIT_FAILURE) {
        goto cleanup;
    }

    printf("[REST] Manual triggered event '%ld' with entry id '%s'!\n", event_id, entry_id);

cleanup:
    json_decref(json_post_body);

    return ret ? U_CALLBACK_ERROR : U_CALLBACK_COMPLETE;
}


// ------------------------------------
// Server initialization
// ------------------------------------
static int
_init_server()
{
    if (ulfius_init_instance(&server_instance, PORT, NULL, NULL) != U_OK) {
        printf("[REST] Error initializing server instance!\n");
        return EXIT_FAILURE;
    }

    // Add the API endpoints to the server
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, API_INDEX, 0, &_api_index_get, NULL);

    // Root
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, API_ROOT, 0, &_api_uni_get, NULL);

    // Modules
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES,                0, &_api_modules_get,               NULL);
    //ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES_AVAILABLE_ID,   0, &_api_modules_get_available_id,  NULL);
    //ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES_REGISTERD_ID,   0, &_api_modules_get_registered_id, NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES_ID,             0, &_api_modules_get_id,            NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ADD,            0, &_api_modules_add,               NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_START,       0, &_api_modules_start,             NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_STOP,        0, &_api_modules_stop,              NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_DELETE,      0, &_api_modules_delete,            NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_REGISTER,    0, &_api_modules_register,          NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_UNREGISTER,  0, &_api_modules_unregister,        NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES_ID_DATA,        0, &_api_modules_get_data_id,       NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_DATA,        0, &_api_modules_set_data_id,       NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_UPDATE,      0, &_api_modules_update,            NULL);
    // Streams
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_STREAMS,                    0, &_api_streams_get,                   NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_STREAMS_ID,                 0, &_api_streams_get_id,                NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_STREAMS_REQUEST,            0, &_api_streams_request,               NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_STREAMS_REQUEST_SIMPLIFIED, 0, &_api_streams_request_simplified,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_STREAMS_ID_DELETE,          0, &_api_streams_delete,                NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_STREAMS_COMPUTE,            0, &_api_streams_compute,               NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_STREAMS_ID_DEPLOY,          0, &_api_streams_deploy,                NULL);

    // Topology
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_TOPOLOGY,                   0, &_api_topology_get,              NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_TOPOLOGY_DEVICES,           0, &_api_topology_devices_get,      NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_TOPOLOGY_DEVICES_ID_UPDATE, 0, &_api_topology_devices_update,   NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_TOPOLOGY_GRAPH,             0, &_api_topology_graph_get,        NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_TOPOLOGY_DISCOVER,          0, &_api_topology_discover,         NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_TOPOLOGY_GRAPH_UPDATE,      0, &_api_topology_graph_update,     NULL);
    // Application
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_APPLICATION_DISCOVER,	 0, &_api_application_discover_post, NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_APPLICATION,		     0, &_api_application_get,           NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_APPLICATION_APPS,   	 0, &_api_application_apps_get,      NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_APPLICATION_IMAGES, 	 0, &_api_application_images_get,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_APPLICATION_APPS_CREATE, 0, &_api_application_app_create,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_APPLICATION_APPS_DELETE, 0, &_api_application_app_delete,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_APPLICATION_APPS_START,  0, &_api_application_app_start,     NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_APPLICATION_APPS_STOP, 	 0, &_api_application_app_stop,      NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_APPLICATION_APPS_UPDATE, 0, &_api_application_app_update,    NULL);
    // Configuration
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_CONFIGURATION_DEPLOY, 0, &_api_configuration_app_deploy,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_CONFIGURATION_UPDATE, 0, &_api_configuration_app_update,    NULL);
    //ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_CONFIGURATION_RUN_STATE, 0, &_api_configuration_app_run_state,    NULL);
    //ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_CONFIGURATION_TOGGLE_TXRX, 0, &_api_configuration_app_toggle_txrx,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_CONFIGURATION_SET_STREAM_SENDRECEIVE, 0, &_api_configuration_set_stream_sendreceive,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_CONFIGURATION_GET_STREAM_SENDRECEIVE, 0, &_api_configuration_get_stream_sendreceive,    NULL);

    // Debug
    ulfius_add_endpoint_by_val(&server_instance, "GET",  API_PREFIX, "/debug/set_topology",    0, &_api_debug_set_topology,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",  API_PREFIX, "/debug/remove_topology", 0, &_api_debug_remove_topology, NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",  API_PREFIX, "/debug/websocket",       0, &_api_debug_websocket,       NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST", API_PREFIX, "/debug/trigger-event",   0, &_api_debug_trigger_event,   NULL);

    // Default
    ulfius_set_default_endpoint(&server_instance, &_api_index_get, NULL);

    return EXIT_SUCCESS;
}

// ------------------------------------
// MAIN
// ------------------------------------
int
main(void)
{
    // Signal handling
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);

    // Init this module
    /*
    this_module.name = "REST";
    this_module.description = "Exposes a REST API to interact with the framework";
    this_module.path = "./RESTModule";
    this_module.subscribed_events_mask = (EVENT_ERROR);

    // sub mask = 1

    this_module.cb_event = _cb_event;

    rc = module_init(&this_module);
    */
    this_module = malloc(sizeof(TSN_Module));
    rc = module_init("REST", &this_module, -1, _cb_event);
    if (rc == EXIT_FAILURE) {
        printf("[REST] Error initializing module!\n");
        goto cleanup;
    }

    // Get saved module data
    TSN_Module_Data *module_data = malloc(sizeof(TSN_Module_Data));
    rc = module_get_data(this_module->id, &module_data);
    if (module_data) {

        // To this URL the saved data from Sysrepo will be exported. 
        TSN_Module_Data_Entry *export_api_entry = module_get_data_entry(module_data, MODULE_DATA_EXPORT_REST_API_URL);
        if (export_api_entry) {
            export_rest_api = strdup(export_api_entry->value.string_val);
        }
    }

    // Init the web server instance
    rc = _init_server();
    if (rc == EXIT_FAILURE) {
        goto cleanup;
    }

    // Start the server
    if (ulfius_start_framework(&server_instance) == U_OK) {
        printf("[REST] REST server successfully started and listening on '%s:%d'\n", "http://localhost", server_instance.port);

        // Keep the server running
        while (is_running) {
            sleep(1);
        }
    } else {
        printf("[REST] Error starting the server!\n");
        rc = EXIT_FAILURE;
        goto cleanup;
    }

    printf("[REST] Stopping the module...\n");
    if (ulfius_stop_framework(&server_instance) != U_OK) {
        printf("[REST] Error stopping server!\n");
        rc = EXIT_FAILURE;
        goto cleanup;
    }


cleanup:
    ulfius_clean_instance(&server_instance);
    rc = module_shutdown(this_module->id);
    if (rc == EXIT_FAILURE) {
        printf("[REST] Error shutting down the module!\n");
    }

    return rc;
}
