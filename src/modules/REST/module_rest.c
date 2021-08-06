#include <stdio.h>
#include <ulfius.h>
#include <signal.h>

#include "module_rest.h"
#include "../base_module.h"
#include "../../sysrepo/sysrepo_client.h"

int rc;

volatile sig_atomic_t is_running = 1;

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

// MODULES
/**
 * @brief API endpoint GET "/modules".
 * Query of the modules registered in the core
 */
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

/**
 * @brief API endpoint GET "/modules/:id".
 * Query of a specific module based on the module ID 
 */
static int 
_cb_get_modules_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/modules/:id/start".
 * Starting a specific module
 */
static int 
_cb_post_modules_id_start(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/modules/:id/stop".
 * Stopping a specific module
 */
static int 
_cb_post_modules_id_stop(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/modules/:id/remove".
 * Removing a specific module
 */
static int 
_cb_post_modules_id_remove(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/modules/:id/register".
 * Register a specific module
 */
static int 
_cb_post_modules_id_register(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/modules/:id/unregister".
 * Unregister a specific module
 */
static int 
_cb_post_modules_id_unregister(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/modules/add".
 * Adding a new module by passing the necessary information
 */
static int 
_cb_post_modules_add(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

// TOPOLOGY
/**
 * @brief API endpoint GET "/topology".
 * Query of the stored topology information
 */
static int 
_cb_get_topology(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/topology/scan".
 * Initiate the scan of the topology
 */
static int 
_cb_post_topology_scan(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

// STREAMS
/**
 * @brief API endpoint GET "/streams".
 * Query of all streams
 */
static int 
_cb_get_streams(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint GET "/streams/:id".
 * Query of a specific stream based on the stream ID
 */
static int 
_cb_get_streams_id(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/streams/:id/modify".
 * Modifying a specific streams by passing the necessary information
 */
static int 
_cb_post_streams_id_modify(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}

/**
 * @brief API endpoint POST "/streams/:id/remove".
 * Removing a specific stream
 */
static int 
_cb_post_stream_id_remove(const struct _u_request *request, struct _u_response *response, void *user_data) {

}

/**
 * @brief API endpoint POST "/streams/add".
 * Adding a new stream by passing the necessary information
 */
static int 
_cb_post_stream_add(const struct _u_request *request, struct _u_response *response, void *user_data)
{

}


void set_module_data(void *data){
    printf("HALLO----------------------\n");
}

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