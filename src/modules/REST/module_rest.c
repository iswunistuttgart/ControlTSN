#include <stdio.h>
#include <ulfius.h>
#include <signal.h>

#include "module_rest.h"
#include "../base_module.h"

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
    ulfius_set_string_body_response(response, 200, "This is the ControlTSN REST Server");
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

    struct _u_instance instance; // Ulfius instance

    // Init the instance
    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "[MODULE][REST] Error ulfius_init_instance, abort\n");
        return(1);
    }

    // Add the endpoints to the instance
    ulfius_add_endpoint_by_val(&instance, "GET", ENDPOINT_INDEX, NULL, 0, &_cb_get_index, NULL);

    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_MODULES,           NULL, 0, &_cb_get_modules,              NULL);
    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_MODULES_ID,        NULL, 0, &_cb_get_modules_id,           NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_START,  NULL, 0, &_cb_post_modules_id_start,    NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_STOP,   NULL, 0, &_cb_post_modules_id_stop,     NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ID_REMOVE, NULL, 0, &_cb_post_modules_id_remove,   NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_MODULES_ADD,       NULL, 0, &_cb_post_modules_add,         NULL);

    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_TOPOLOGY,          NULL, 0, &_cb_get_topology,             NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_TOPOLOGY_SCAN,     NULL, 0, &_cb_post_topology_scan,       NULL);

    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_STREAMS,           NULL, 0, &_cb_get_streams,              NULL);
    ulfius_add_endpoint_by_val(&instance, "GET",    ENDPOINT_STREAMS_ID,        NULL, 0, &_cb_get_streams_id,           NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_STREAMS_ID_MODIFY, NULL, 0, &_cb_post_streams_id_modify,   NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_STREAMS_ID_REMOVE, NULL, 0, &_cb_post_stream_id_remove,    NULL);
    ulfius_add_endpoint_by_val(&instance, "POST",   ENDPOINT_STREAMS_ADD,       NULL, 0, &_cb_post_stream_add,          NULL);

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