#include <stdio.h>
#include <ulfius.h>
#include <signal.h>
#include <string.h>

#include "export_server.h"


int rc;
volatile sig_atomic_t is_running = 1;
struct _u_instance server_instance;

static int
_api_index(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *resp = "<style>td {padding-right: 20px;} th {text-align: left;}</style><html><b>This is the ControlTSN REST Server.</b></br>" \
                       "Following endpoints are provided:</br></br>" \
                       "<table>" \
                       "<tr><th>URI</th><th>Type</th><th>Description</th><th>POST Body</th></tr>" \

                       // Index
                       "<tr><td><a href='/'>/</a></td><td>GET</td><td>This site</td></tr>" \

                       // Export
                       "<tr><th>Export</th></tr>" \
                       "<tr><td><a href='/export'>/export</a></td><td>POST</td><td>Receive exported data</td><td>any JSON data</td></tr>" \

                       "</table></html>";
    ulfius_set_string_body_response(response, 200, resp);

    return U_CALLBACK_CONTINUE;
}

static int
_api_receive_exported_data(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // Get the data
    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        printf("[EXPORT][ERROR] Body of POST was empty. Abort!");
        ulfius_set_empty_body_response(response, 400);
        return U_CALLBACK_COMPLETE;
    }

    printf("[EXPORT] Received exported data:\n");
    printf("%s\n", json_dumps(json_post_body, JSON_INDENT(2)));
    ulfius_set_empty_body_response(response, 200);

    json_decref(json_post_body);
    return U_CALLBACK_COMPLETE;
}


static int
_init_server()
{
    if (ulfius_init_instance(&server_instance, PORT, NULL, NULL) != U_OK) {
        printf("[EXPORT] Error initializing server instance!\n");
        return EXIT_FAILURE;
    }

    // Add Endpoints
    ulfius_add_endpoint_by_val(&server_instance, "POST", API_PREFIX, API_EXPORT, 0, &_api_receive_exported_data, NULL);

    ulfius_set_default_endpoint(&server_instance, &_api_index, NULL);

    return EXIT_SUCCESS;
}

static void 
signal_handler(int signum)
{
    is_running = 0;
}

int
main(void)
{
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);

    rc = _init_server();
    if (rc == EXIT_FAILURE) {
        goto cleanup;
    }

    // Start the web server
    if (ulfius_start_framework(&server_instance) == U_OK) {
        printf("[EXPORT] REST server successfully started and listening on '%s:%d'\n", "http://localhost", server_instance.port);

        // Keep the server running
        while (is_running) {
            sleep(1);
        }

    } else {
        printf("[EXPORT] Error starting the server!\n");
        rc = EXIT_FAILURE;
        goto cleanup;
    }

    if (ulfius_stop_framework(&server_instance) != U_OK) {
        printf("[EXPORT] Error stopping server!\n");
        rc = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:
    ulfius_clean_instance(&server_instance);
    printf("[EXPORT] Shutting down Export Server!\n");

    return rc;
}
