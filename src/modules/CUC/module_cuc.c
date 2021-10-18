#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <ulfius.h>

#include "../../common.h"
#include "../../logger.h"
#include "../../events_definitions.h"
#include "module_cuc.h"

#include "../../helper/json_serializer.h"

int rc;
volatile sig_atomic_t is_running = 1;

TSN_Module this_module;
TSN_Module_Data *this_module_data = NULL;

// THESE VARIABLES SHOULD BE STORED AS MODULE DATA ---------------
char cnc_url[] = "http://localhost:11067";
// ---------------------------------------------------------------


static void 
signal_handler(int signum)
{
    is_running = 0;
}




static char *
_concat_strings(char *s1, const char *s2)
{
    char *res = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(res, s1);
    strcat(res, s2);
    return res;
}

void
cnc_discover_topology()
{
    // Make HTTP Request to the CNC
    struct _u_response response;
    struct _u_request request;
    ulfius_init_request(&request);
    ulfius_init_response(&response);

    request.http_url = _concat_strings(cnc_url, CNC_INTERFACE_DISCOVER_TOPOLOGY);
    request.http_verb = strdup("GET");

    int res = ulfius_send_http_request(&request, &response);
    if (res == U_OK) {
        printf("[CUC] Successfully send request to CNC\n");

        // Process response
        printf("Headers:\n");
        char **keys = u_map_enum_keys(response.map_header);
        for (int i=0; keys[i] != NULL; i++) {
            printf("%s --> %s\n", keys[i], u_map_get(response.map_header, keys[i]));
        }
        json_t *json_body = ulfius_get_json_body_response(&response, NULL);
        TSN_Topology *discovered_topology = NULL;
        discovered_topology = malloc(sizeof(TSN_Topology));
        int ret = deserialize_topology(json_body, &discovered_topology);
        if (ret == EXIT_SUCCESS) {
            printf("Topology successfull deserialized!");
            print_topology(*discovered_topology);
        } else {
            printf("ERROR DESERIALIZING TOPOLOGY!\n");
        }


    } else {
        printf("[CUC] Failure sending request to CNC at '%s'\n", cnc_url);
    }

    ulfius_clean_response(&response);
    ulfius_clean_request(&request);

    return;
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void
_cb_event(TSN_Event_CB_Data data)
{
    printf("[CUC] Triggered callback for event ID %d\n", data.event_id);
    
    if (data.event_id == EVENT_ERROR) {
        printf("[REST][CB] ERROR (%s): %s\n", data.entry_id, data.msg);
    }
    else if (data.event_id == EVENT_TOPOLOGY_DISCOVERY_REQUESTED) {
        printf("[CUC][CB] Topology discovery requested!\n");
        cnc_discover_topology();

    }

    return;
}

void
_init_cuc()
{
    this_module_data = malloc(sizeof(TSN_Module_Data));
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
    this_module.name = "CUC";
    this_module.description = "Represents the Central User Controller in the network based on the central configuration approach of TSN";
    this_module.path ="./CUCModule";
    this_module.subscribed_events_mask = (EVENT_ERROR | EVENT_TOPOLOGY_DISCOVERY_REQUESTED);
    this_module.cb_event = _cb_event;

    rc = module_init(&this_module);
    if (rc == EXIT_FAILURE) {
        printf("[CUC] Error initializing module!\n");
        goto cleanup;
    }

    printf("[CUC] CUC module successfully started and running\n");

    // Keep running
    while (is_running) {
        sleep(1);
    }

    printf("[CUC] Stopping the module...\n");
    
cleanup:
    rc = module_shutdown();
    if (rc == EXIT_FAILURE) {
        printf("[REST] Error shutting down the module!\n");
    }

    return rc;
}
