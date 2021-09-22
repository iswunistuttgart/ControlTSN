#include <stdio.h>
#include <ulfius.h>
#include <signal.h>

#include "../../common.h"
#include "module_rest.h"

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
// API Endpoint Callback Functions
// ------------------------------------
// INDEX
/**
 * @brief API endpoint GET "/".
 * Index and default.
 */
/*
static int 
_cb_get_index(const struct _u_request *request, struct _u_response *response, void *user_data) 
{
    const char *resp = "<html><b>This is the ControlTSN REST Server.</b></br>" \
                       "Following endpoints are provided:</br></br>" \
                       "<table>" \
                       "<tr><td>/</td><td>GET</td><td>Index</td></tr>" \
                       "<tr><td><a href='/modules'>/modules</a></td><td>GET</td><td>Get all available and registered modules</td></tr>" \
                       "<tr><td><a href='/modules/:id'>/modules/:id</a></td><td>GET</td><td>Get a specific module by the ID</td></tr>" \
                       "<tr><td><a href='/modules/:id/start'>/modules/:id/start</a></td><td>POST</td><td>Start a specific module</td></tr>" \
                       "<tr><td><a href='/modules/:id/stop'>/modules/:id/stop</a></td><td>POST</td><td>Stop a specific module</td></tr>" \
                       "<tr><td><a href='/modules/:id/remove'>/modules/:id/remove</a></td><td>POST</td><td>Remove a specific module from the availble modules</td></tr>" \
                       "<tr><td><a href='/modules/:id/register'>/modules/:id/register</a></td><td>POST</td><td>Register a specific module in the core</td></tr>" \
                       "<tr><td><a href='/modules/:id/unregister'>/modules/:id/unregister</a></td><td>POST</td><td>Unregister a specific module from the core</td></tr>" \
                       "<tr><td><a href='/modules/add'>/modules/add</a></td><td>POST</td><td>Add a new module to the availble modules</td></tr>" \
                       "<tr><td><a href='/topology'>/topology</a></td><td>GET</td><td>Get the stored topology data</td></tr>" \
                       "<tr><td><a href='/topology/scan'>/topology/scan</a></td><td>POST</td><td>Scan the current topology</td></tr>" \
                       "<tr><td><a href='/streams'>/streams</a></td><td>GET</td><td>Gett all streams</td></tr>" \
                       "<tr><td><a href='/streams/:id'>/streams/:id</a></td><td>GET</td><td>Get a specific stream by the stream ID</td></tr>" \
                       "<tr><td><a href='/streams/:id/modify'>/streams/:id/modify</a></td><td>POST</td><td>Edit a existing stream</td></tr>" \
                       "<tr><td><a href='/streams/:id/remove'>/streams/:id/remove</a></td><td>POST</td><td>Remove a specific stream</td></tr>" \
                       "<tr><td><a href='/streams/add'>/streams/add</a></td><td>POST</td><td>Add a new stream</td></tr>" \
                       "</table></html>";
    ulfius_set_string_body_response(response, 200, resp);
    return U_CALLBACK_CONTINUE;
}
*/

// MODULES
/**
 * @brief API endpoint GET "/modules".
 * Query of the modules registered in the core
 */
/*
static int 
_cb_get_modules(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    
    // Get Modules from Sysrepo
    TSN_Modules *mods = malloc(sizeof(TSN_Modules));
    //int ret = get_modules_from_sysrepo(&mods);
    int ret = sysrepo_get_modules(&mods);

    json_t *json_body = NULL;
    json_body = json_object();
    json_object_set_new(json_body, "mod_count", json_integer(mods->count_all_modules));
    ulfius_set_json_body_response(response, 200, json_body);

    return U_CALLBACK_CONTINUE;
}
*/
   

/*
int main(void)
{
    // Signal handling
    signal(SIGINT, signal_handler);

    rc = sysrepo_connect();
    if (rc != SR_ERR_OK) {
        printf("[MODULE][REST] Failure while connecting to Sysrepo!\n");
        return EXIT_FAILURE;
    }
    printf("[MODULE][REST] Connected to Sysrepo!\n");

    struct _u_instance instance; // Ulfius instance

    // Init the instance
    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "[MODULE][REST] Error ulfius_init_instance, abort\n");
        return(1);
    }

    // Add the endpoints to the instance
    ulfius_add_endpoint_by_val(&instance, "GET", ENDPOINT_INDEX, NULL, 0, &_cb_get_index, NULL);

    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_MODULES,               NULL, 0, &_cb_get_modules,                  NULL);
    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_MODULES_ID,            NULL, 0, &_cb_get_modules_id,               NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_START,      NULL, 0, &_cb_post_modules_id_start,        NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_STOP,       NULL, 0, &_cb_post_modules_id_stop,         NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_REMOVE,     NULL, 0, &_cb_post_modules_id_remove,       NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_REGISTER,   NULL, 0, &_cb_post_modules_id_register,     NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_UNREGISTER, NULL, 0, &_cb_post_modules_id_unregister,   NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ADD,           NULL, 0, &_cb_post_modules_add,             NULL);

    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_TOPOLOGY,              NULL, 0, &_cb_get_topology,                 NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_TOPOLOGY_SCAN,         NULL, 0, &_cb_post_topology_scan,           NULL);

    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_STREAMS,               NULL, 0, &_cb_get_streams,                  NULL);
    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_STREAMS_ID,            NULL, 0, &_cb_get_streams_id,               NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_STREAMS_ID_MODIFY,     NULL, 0, &_cb_post_streams_id_modify,       NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_STREAMS_ID_REMOVE,     NULL, 0, &_cb_post_stream_id_remove,        NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_STREAMS_ADD,           NULL, 0, &_cb_post_stream_add,              NULL);

    ulfius_set_default_endpoint(&instance, &_cb_get_index, NULL);

    // Start the framework
    if (ulfius_start_framework(&instance) == U_OK) {
        printf("[MODULE][REST] Start REST server on port %d\n", instance.port);

        // Keep server running
        while (is_running) {
            sleep(1);
        }

    } else {
        fprintf(stderr, "[MODULE][REST] Error starting REST server\n");
    }

    printf("[MODULE][REST] End REST server\n");
    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}
*/



// ------------------------------------
// Callback handler
// ------------------------------------
static void
_cb_event(int event_id, TSN_Event_CB_Data data)
{
    printf("[REST] Triggered callback for event ID %d\n", event_id);
    
    if (event_id == EVENT_ERROR) {
        printf("[REST][CB] ERROR: Code %d - '%s'\n", data.error.error_code, data.error.error_msg);
    }

    return;
}

// ------------------------------------
// API Endpoint Callback Functions
// ------------------------------------
// INDEX '/'
static int
_cb_index_get(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *resp = "HALLO\n";
    ulfius_set_string_body_response(response, 200, resp);

    return U_CALLBACK_CONTINUE;
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
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_INDEX, NULL, 0, &_cb_index_get, NULL);

    ulfius_set_default_endpoint(&server_instance, &_cb_index_get, NULL);
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
    this_module.subscribed_events_mask = (EVENT_ERROR);
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

    return rc;    
}
