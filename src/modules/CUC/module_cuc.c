#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <ulfius.h>

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>

#include "../../logger.h"
#include "../../events_definitions.h"
#include "module_cuc.h"

#include "../../helper/json_serializer.h"


#define EMULATE_OPENCNC      true


static int rc;
volatile sig_atomic_t is_running = 1;

TSN_Module *this_module;
TSN_Module_Data *this_module_data = NULL;

// Variables from module data stored in sysrepo
char *cnc_url = NULL;


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
        // Get JSON body containing the topology
        json_t *json_body = ulfius_get_json_body_response(&response, NULL);
        // Deserialize the topology
        TSN_Topology *discovered_topology = deserialize_topology(json_body);
        // Write the topology to sysrepo
        rc = sysrepo_set_topology(discovered_topology);
        if (rc != EXIT_SUCCESS) {
            printf("[CUC] Error writing topology to the datastore!\n");
        }

    } else {
        printf("[CUC] Failure sending request to CNC at '%s'\n", cnc_url);
    }

    ulfius_clean_response(&response);
    ulfius_clean_request(&request);

    return;
}

void 
cnc_compute_requests(TSN_Streams *streams)
{
    // Make HTTP Request to the CNC
    struct _u_response response;
    struct _u_request request;
    ulfius_init_request(&request);
    ulfius_init_response(&response);

    //request.http_url = _concat_strings(cnc_url, CNC_INTERFACE_COMPUTE_REQUESTS);
    request.http_url = strdup(cnc_url);
    request.http_verb = strdup("POST");

    json_t *body = NULL;
    

#if EMULATE_OPENCNC    
    body = serialize_cnc_request(streams);
#else
    body = serialize_streams(streams);
#endif

    ulfius_set_json_body_request(&request, body);

    int res = ulfius_send_http_request(&request, &response);
    if (res == U_OK) {
        printf("[CUC] Successfully sent request to CNC at '%s'\n", request.http_url);
        // get JSON body containing the computed configuration
        json_t *json_body = ulfius_get_json_body_response(&response, NULL);

        // Write Configurations back to sysrepo
        if (json_body != NULL) {

#if EMULATE_OPENCNC
            TSN_Streams *streams = deserialize_cnc_response(json_body);
#else
            TSN_Streams *streams = deserialize_streams(json_body);       
#endif
            for (int i=0; i<streams->count_streams; ++i) {
                rc = stream_set_computed(streams->streams[i].stream_id, streams->streams[i].configuration);
                if (rc != EXIT_SUCCESS) {
                    printf("[CUC] Error writing stream configuration to the datastore!\n");
                }
            }
        }
    } else {
        printf("[CUC] Failure sending request to CNC at '%s'\n", request.http_url);
    }

    ulfius_clean_response(&response);
    ulfius_clean_request(&request);

    return;
}


TSN_App *
_find_app_for_mac(char *mac, TSN_Apps *apps)
{
    // Search through the apps and find the one to which the given MAC belongs
    for (int i=0; i<apps->count_apps; ++i) {
        if (apps->apps[i].has_mac && (strcmp(apps->apps[i].mac, mac) == 0)) {
            return &apps->apps[i];
        }
    }

    return NULL;
}

TSN_App_Parameter *
_find_app_parameter(TSN_App *app, const char *parameter_name)
{
    for (int i=0; i<app->count_parameters; ++i) {
        if (strcmp(app->parameters[i].name, parameter_name) == 0) {
            return &(app->parameters[i]);
        }
    }

    return NULL;
}

/*
void 
deploy_configuration(TSN_App *app, TSN_Configuration *stream_configuration)
{
    // TODO!...

    // Get the OPC UA Endpoint from the app parameters
    TSN_App_Parameter *param_opcua_endpoint = _find_app_parameter(app, APP_PARAMETER_IDENTIFIER_OPCUA_ENDPOINT);
    if (param_opcua_endpoint == NULL) {
        printf("[CUC] Could not deploy configuration to app '%s' because of missing opc ua endpoint parameter!\n", app->name);
        return;
    }

    // Write the stream configuration to the OPC UA server of the app
    // ...
    
}
*/

int 
deploy_configuration(TSN_Enddevice *enddevice, TSN_Configuration *stream_configuration, char *app_id)
{
    printf("--- Deploying configuration to: \n");
    print_enddevice(*enddevice);
    

    UA_Variant *my_variant;
    UA_NodeId nodeID;
    UA_StatusCode retval;
    UA_Client *client;

    // Check if the enddevice has a interface URI
    if (!enddevice || !enddevice->interface_uri) {
        printf("[CUC][ERROR] Could not deploy stream configuration to enddevice (%s) because of missing interface URI!\n", enddevice->mac);
    }

    // Connect to the server
    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    retval = UA_Client_connect(client, enddevice->interface_uri);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("[CUC][ERROR] Could not connect to OPC UA Server '%s'\n", enddevice->interface_uri);
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }

    // Write configuration to the OPC UA Server of the enddevice
    my_variant = UA_Variant_new();
    // (currently just a test)
    //UA_String string_value = UA_STRING((char *) stream_configuration->talker.interface_configuration.interface_list[0].mac_address);
    nodeID = UA_NODEID_STRING(1, "Qbv Offset");
    UA_Int32 qbvOffset = (UA_Int32) stream_configuration->talker.interface_configuration.interface_list[0].config_list[2].time_aware_offset;
    UA_Variant_setScalarCopy(my_variant, &qbvOffset, &UA_TYPES[UA_TYPES_INT32]);
    retval = UA_Client_writeValueAttribute(client, nodeID, my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("[CUC][OPCUA][ERROR] Failed to write 'Qbv Offset' to %s!\n", enddevice->interface_uri);
        goto cleanup;
    }

    printf("[CUC][OPCUA] Successfully written the stream configuration to enddevice %s!\n", enddevice->interface_uri);


cleanup:
    UA_Variant_delete(my_variant);
    UA_Client_delete(client);

    return EXIT_SUCCESS;
}


// ------------------------------------
// Callback handler
// ------------------------------------
static void
_cb_event(TSN_Event_CB_Data data)
{
    if (data.event_id == EVENT_ERROR) {
        printf("[CUC][CB] ERROR (%s): %s\n", data.entry_id, data.msg);
    }

    // --- TOPOLOGY ----------------------
    else if (data.event_id == EVENT_TOPOLOGY_DISCOVERY_REQUESTED) {
        printf("[CUC][CB] Topology discovery requested!\n");
        cnc_discover_topology();
    }
    else if (data.event_id == EVENT_TOPOLOGY_DISCOVERED) {
        printf("[CUC][CB] Topology discovered!\n");
    }

    // --- STREAMS ----------------------
    else if (data.event_id == EVENT_STREAM_COMPUTATION_REQUESTED) {
        printf("[CUC][CB] Stream computation requested!\n");
        TSN_Streams *streams = malloc(sizeof(TSN_Streams));
        rc = streams_get_all(&streams, 1);
        if (rc == EXIT_SUCCESS) { 
            if (streams->count_streams > 0) {
                cnc_compute_requests(streams);
            } else {
                printf("[CUC][CB] There are no streams that are not yet configured!\n");
            }
        } else {
            printf("[CUC][CB] Error reading streams from sysrepo!\n");
        }
        free(streams);
    }

    /*
    else if (data.event_id == EVENT_STREAM_CONFIGURED) {
        printf("[CUC][CB] Stream '%s' configured!\n", data.entry_id);
        // Read Stream configuration and deploy it to the corresponding enddevices
        TSN_Stream *configured_stream = malloc(sizeof(TSN_Stream));
        rc = sysrepo_get_stream(data.entry_id, &configured_stream);
        if (rc == EXIT_SUCCESS) {

            // Read all apps from datastore
            TSN_Apps *all_apps = malloc(sizeof(TSN_Apps));
            rc = sysrepo_get_application_apps(&all_apps);
            if (rc == EXIT_SUCCESS) {
                // Get the app that belongs to the talker mac
                TSN_App *app_talker = NULL;
                // NOTICE: We use the first entry in the list of endstation interfaces (TODO: should we rether use all interfaces for the search?)
                app_talker = _find_app_for_mac(configured_stream->configuration->talker.interface_configuration.interface_list[0].mac_address, all_apps);
                if (app_talker != NULL) {
                    // Send the configuration to the talker
                    printf("[CUC][CB] Deploying configuration for stream '%s' to talker application '%s@%s'\n", configured_stream->stream_id, app_talker->name, app_talker->mac);
                    deploy_configuration(app_talker, configured_stream->configuration);
                } else {
                    printf("[CUC][CB][ERROR] Could not find Application for mac '%s'\n", configured_stream->configuration->talker.interface_configuration.interface_list[0].mac_address);
                }
                
                // Get the app(s) that belong to the listener macs
                for (int i=0; i<configured_stream->configuration->count_listeners; ++i) {
                    TSN_App *app_listener = NULL;
                    app_listener = _find_app_for_mac(configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address, all_apps);
                    if (app_listener != NULL) {
                        // Send the configuration to the listener
                        printf("[CUC][CB] Deploying configuration for stream '%s' to listener application '%s@%s'\n", configured_stream->stream_id, app_listener->name, app_listener->mac);
                        deploy_configuration(app_listener, configured_stream->configuration);
                    } else {
                        printf("[CUC][CB][ERROR] Could not find Application for mac '%s'\n", configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address);
                    }
                }   

            } else {
                printf("[CUC][CB][ERROR] Could not read applications from the datastore!\n");
            }
        }
        else {
            printf("ERROR READING STREAM... \n");
        }
        
        free(configured_stream);
    }
    */

    else if (data.event_id == EVENT_STREAM_CONFIGURED) {
        printf("[CUC][CB] Stream '%s' configured! Deploying the configuration to the enddevices...\n", data.entry_id);
    
    
        TSN_Stream *configured_stream = malloc(sizeof(TSN_Stream));
        rc = sysrepo_get_stream(data.entry_id, &configured_stream);
        if (rc == EXIT_SUCCESS) {

            // Deploy the configuration to the enddevices
            // Talker
            TSN_Enddevice *talker = malloc(sizeof(TSN_Enddevice));
            rc = sysrepo_get_enddevice(configured_stream->configuration->talker.interface_configuration.interface_list[0].mac_address, &talker);
            if (rc != EXIT_SUCCESS) {
                printf("[CUC][CB][ERROR] Could not read enddevice (talker) with MAC '%s' from the datastore!\n", configured_stream->configuration->talker.interface_configuration.interface_list[0].mac_address);
            }

            // TODO: When requesting a stream, the associated app should already be linked using the AppID. 
            // Otherwise, the assignment of stream and app is awkward afterwards (via the Talker/Listener MACs).
            rc = deploy_configuration(talker, configured_stream->configuration, "N/A");
            if (rc != EXIT_SUCCESS) {
                printf("[CUC][CB][ERROR] Could not deploy stream configuration to enddevice (talker, %s)!\n", configured_stream->configuration->talker.interface_configuration.interface_list[0].mac_address);
            }
            free(talker);

            // Listeners
            for (uint16_t i=0; i<configured_stream->configuration->count_listeners; ++i) {
                TSN_Enddevice *listener = malloc(sizeof(TSN_Enddevice));
                rc = sysrepo_get_enddevice(configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address, &listener);
                if (rc != EXIT_SUCCESS) {
                    printf("[CUC][CB][ERROR] Could not read enddevice (listener #%d) with MAC '%s' from the datastore!\n", i, configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address);
                }

                rc = deploy_configuration(listener, configured_stream->configuration, "N/A");
                if (rc != EXIT_SUCCESS) {
                    printf("[CUC][CB][ERROR] Could not deploy stream configuration to enddevice (listener #%d, %s)!\n", i, configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address);
                }
                free(listener);
            }

        } else {
            printf("[CUC][CB][ERROR] Could not read applications from the datastore!\n");
        }

        free(configured_stream);
    }

    return;
}

void
_init_cuc()
{
    /*
    this_module_data = malloc(sizeof(TSN_Module_Data));
    this_module_data->count_entries = 1;
    this_module_data->entries = (TSN_Module_Data_Entry *) malloc(sizeof(TSN_Module_Data_Entry) * this_module_data->count_entries);
    TSN_Module_Data_Entry entry;
    entry.name = strdup(MODULE_DATA_IDENTIFIER_CNC);
    entry.type = STRING;
    entry.value.string_val = strdup("http://localhost:11067");
    this_module_data->entries[0] = entry;
    this_module.data = *this_module_data;
    */
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
    this_module.name = "CUC";
    this_module.description = "Represents the Central User Controller in the network based on the central configuration approach of TSN";
    this_module.path ="./CUCModule";
    this_module.subscribed_events_mask = (EVENT_ERROR | EVENT_TOPOLOGY_DISCOVERY_REQUESTED | EVENT_TOPOLOGY_DISCOVERED);

    // sub mask = 196609

    this_module.cb_event = _cb_event;

    //rc = module_init(&this_module);
    //if (rc == EXIT_FAILURE) {
    //    printf("[CUC] Error initializing module!\n");
    //    goto cleanup;
    //}
    */

    this_module = malloc(sizeof(TSN_Module));
    rc = module_init("CUC", &this_module, -1, _cb_event);
    if (rc != EXIT_SUCCESS) {
        goto cleanup;
    }

    // Get saved module data
    TSN_Module_Data *module_data = malloc(sizeof(TSN_Module_Data));
    rc = module_get_data(this_module->id, &module_data);
    if (rc != EXIT_SUCCESS) {
        goto cleanup;
    }

    // Find cnc url
    TSN_Module_Data_Entry *cnc_entry = module_get_data_entry(module_data, MODULE_DATA_IDENTIFIER_CNC);
    if (cnc_entry) {
        cnc_url = strdup(cnc_entry->value.string_val);
    }

    printf("[CUC] CUC module successfully started and running\n");

    // Keep running
    while (is_running) {
        sleep(1);
    }

    printf("[CUC] Stopping the module...\n");
    
cleanup:
    rc = module_shutdown(this_module->id);
    if (rc == EXIT_FAILURE) {
        printf("[CUC] Error shutting down the module!\n");
    }

    return rc;
}
