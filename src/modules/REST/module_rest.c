#include <stdio.h>
#include <ulfius.h>
#include <signal.h>
#include <string.h>

#include "../../common.h"
#include "../../events_definitions.h"
#include "module_rest.h"
#include "json_serializer.h"

int rc;
volatile sig_atomic_t is_running = 1;

TSN_Module this_module;
struct _u_instance server_instance;


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
    printf("[REST] Triggered callback for event ID %d\n", data.event_id);
    
    if (data.event_id == EVENT_ERROR) {
        //printf("[REST][CB] ERROR: Code %d - '%s'\n", data.error.error_code, data.error.error_msg);
        printf("[REST][CB] ERROR: %s\n", data.msg);
    }
    else if (data.event_id == EVENT_TOPOLOGY_DISCOVERY_REQUESTED) {
        printf("[REST][CB] Topology discovery requested!\n");
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
    const char *resp = "<html><b>This is the ControlTSN REST Server.</b></br>" \
                       "Following endpoints are provided:</br></br>" \
                       "<table>" \
                       // Index
                       "<tr><th style='text-align: left;'>Index</th></tr>" \
                       "<tr><td><a href='/'>/</a></td><td>GET</td><td>This site</td></tr>" \
                       // Modules
                       "<tr><th style='text-align: left;'>Modules</th></tr>" \
                       "<tr><td><a href='/modules'>/modules</a></td><td>GET</td><td>Get all available and registered modules</td></tr>" \
                       "<tr><td><a href='/modules/registered/1'>/modules/registered/:id</a></td><td>GET</td><td>Get a specific registered module based on the id</td></tr>" \
                       "<tr><td><a href='/modules/available/1'>/modules/available/:id</a></td><td>GET</td><td>Get a specific available module based on the id</td></tr>" \
                       "<tr><td><a href='/modules/1/start'>/modules/:id/start</a></td><td>POST</td><td>Start a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/stop'>/modules/:id/stop</a></td><td>POST</td><td>Stop a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/delete'>/modules/:id/delete</a></td><td>POST</td><td>Delete a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/register'>/modules/:id/register</a></td><td>POST</td><td>Register a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/unregister'>/modules/:id/unregister</a></td><td>POST</td><td>Unregister a specific module</td></tr>" \
                       "<tr><td><a href='/modules/1/data'>/modules/:id/data</a></td><td>GET</td><td>Get the data of a specific module</td></tr>" \
                       // Topology
                       "<tr><th style='text-align: left;'>Topology</th></tr>" \
                       "<tr><td><a href='/topology'>/topology</a></td><td>GET</td><td>Get the stored topology data</td></tr>" \
                       "<tr><td><a href='/topology/discover'>/topology/discover</a></td><td>POST</td><td>Trigger the topology discovery</td></tr>" \
                       "<tr><td><a href='/topology/devices'>/topology/devices</a></td><td>GET</td><td>Get the stored devices</td></tr>" \
                       "<tr><td><a href='/topology/graph'>/topology/graph</a></td><td>GET</td><td>Get the topology graph containing all connections</td></tr>" \
                       // Streams
                       "<tr><th style='text-align: left;'>Streams</th></tr>" \
                       "<tr><td><a href='/streams'>/streams</a></td><td>GET</td><td>Get all streams</td></tr>" \
                       "<tr><td><a href='/streams/request'>/streams/request</a></td><td>POST</td><td>Request a new stream</td></tr>" \
                       // Applications
                       "<tr><th style='text-align: left;'>Applications</th></tr>" \
                       "<tr><td><a href='/application'>/application</a></td><td>GET</td><td>Get the application containing all apps and images</td></tr>" \
                       "<tr><td><a href='/application/apps'>/application/apps</a></td><td>GET</td><td>Get all stored apps</td></tr>" \
                       "<tr><td><a href='/application/images'>/application/images</a></td><td>GET</td><td>Get all stored images</td></tr>" \
                       "<tr><td><a href='/application/apps/:id/start'>/application/apps/:id/start</a></td><td>POST</td><td>Start a specific app</td></tr>" \
                       "<tr><td><a href='/application/apps/:id/stop'>/application/apps/:id/stop</a></td><td>POST</td><td>Stop a specific app</td></tr>" \
                       // TEST
                       "<tr><th style='text-align: left;'>JUST TESTING</th></tr>" \
                       "<tr><td><a href='/testing/add_enddevice'>/testing/add_enddevice</a></td><td>GET</td><td>TESTING: Add a dummy enddevice</td></tr>" \
                       "</table></html>";
    ulfius_set_string_body_response(response, 200, resp);

    return U_CALLBACK_CONTINUE;
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

    rc = module_register(module_id);
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

// ------------------------------------
// Streams
// ------------------------------------
static int
_api_streams_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get all streams from sysrepo
    TSN_Streams *streams = malloc(sizeof(TSN_Streams));
    rc = streams_get_all(&streams);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    // Return streams as JSON
    json_t *json_body = serialize_streams(streams);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

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
_api_topology_discover(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    rc = sysrepo_send_notification(EVENT_TOPOLOGY_DISCOVERY_REQUESTED, NULL, NULL);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    return U_CALLBACK_COMPLETE;
}

// ------------------------------------
// Application
// ------------------------------------
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
    rc = application_get_apps(&apps);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    json_t *json_body = serialize_apps(apps);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_application_images_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Images *images = malloc(sizeof(TSN_Images));
    rc = application_get_images(&images);
    if (rc == EXIT_FAILURE) {
        return U_CALLBACK_ERROR;
    }

    json_t *json_body = serialize_images(images);
    ulfius_set_json_body_response(response, 200, json_body);

    json_decref(json_body);

    return U_CALLBACK_COMPLETE;
}



// ------------------------------------
// TESTING
// ------------------------------------
static int
_api_testing_add_enddevice(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    TSN_Enddevice e = {
        .mac = strdup("00:00:00:00:00:01"),
        .app_ref = NULL
    };
    TSN_Devices d;
    d.count_enddevices = 1;
    d.enddevices[0] = e;
    TSN_Topology t;
    t.devices = d;

    rc = sysrepo_set_topology(&t);
    if (rc == EXIT_FAILURE) {
        printf("FEHLER BEIM SCHREIBEN DER TOPOLOGY!\n");
        return U_CALLBACK_ERROR;
    }


    return U_CALLBACK_COMPLETE;
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
    
    // Modules
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES,                0, &_api_modules_get,               NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES_AVAILABLE_ID,   0, &_api_modules_get_available_id,  NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES_REGISTERD_ID,   0, &_api_modules_get_registered_id, NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_START,       0, &_api_modules_start,             NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_STOP,        0, &_api_modules_stop,              NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_DELETE,      0, &_api_modules_delete,            NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_REGISTERD_ID,   0, &_api_modules_register,          NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST",    API_PREFIX, API_MODULES_ID_UNREGISTER,  0, &_api_modules_unregister,        NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET",     API_PREFIX, API_MODULES_ID_DATA,        0, &_api_modules_get_data_id,       NULL);
    // Streams
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, API_STREAMS, 0, &_api_streams_get, NULL);
    // Topology
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX,     API_TOPOLOGY,           0, &_api_topology_get,          NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX,     API_TOPOLOGY_DEVICES,   0, &_api_topology_devices_get,  NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX,     API_TOPOLOGY_GRAPH,     0, &_api_topology_graph_get,    NULL);
    ulfius_add_endpoint_by_val(&server_instance, "POST", API_PREFIX,    API_TOPOLOGY_DISCOVER,  0, &_api_topology_discover,     NULL);
    // Application
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, API_APPLICATION,        0, &_api_application_get,           NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, API_APPLICATION_APPS,   0, &_api_application_apps_get,      NULL);
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, API_APPLICATION_IMAGES, 0, &_api_application_images_get,    NULL);
    

    // JUST TESTING
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, "/testing/add_enddevice", 0, &_api_testing_add_enddevice,    NULL);


    // Default
    ulfius_set_default_endpoint(&server_instance, &_api_index_get, NULL);
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

    // Init this module
    this_module.name = "REST";
    this_module.description = "Exposes a REST API to interact with the framework";
    this_module.path = "./RESTModule";
    this_module.subscribed_events_mask = (EVENT_ERROR | EVENT_TOPOLOGY_DISCOVERY_REQUESTED);
    this_module.cb_event = _cb_event;
    
    rc = module_init(&this_module);
    if (rc == EXIT_FAILURE) {
        printf("[REST] Error initializing module!\n");
        goto cleanup;
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
    rc = module_shutdown();
    if (rc == EXIT_FAILURE) {
        printf("[REST] Error shutting down the module!\n");
    }

    return rc;    
}
