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


#define EMULATE_OPENCNC      false


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

void 
cnc_join_listener(char *stream_id, TSN_Listener *listener)
{
    // TODO
}

void 
cnc_leave_listener(char *stream_id, TSN_StatusListener *listener)
{
    // TODO
}

void 
cnc_remove_stream(char *stream_id)
{
    // TODO
}

void 
cnc_update_stream(char *stream_id, TSN_Stream *stream)
{
    // TODO
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

static UA_StatusCode
_findChildNodeID(UA_Client *client, UA_NodeId rootNodeId, UA_NodeId searchNS0NodeId, UA_NodeId *resultNodeId) {
    UA_StatusCode ret;
    UA_QualifiedName searchBrowseName;
    UA_BrowseRequest bReq;
    UA_BrowseResponse bResp;

    ret = UA_Client_readBrowseNameAttribute(client, searchNS0NodeId, &searchBrowseName);
    if (ret != UA_STATUSCODE_GOOD) {
        goto cleanup;
    } 
    
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = rootNodeId;
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;
    bResp = UA_Client_Service_browse(client, bReq);
    for(size_t i = 0; i < bResp.resultsSize; ++i) {
        for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if (strcmp(ref->browseName.name.data, searchBrowseName.name.data) == 0) {
                UA_NodeId_copy(&ref->nodeId.nodeId, resultNodeId);
                goto cleanup;
            }  
        }
    }
    

cleanup:
    UA_QualifiedName_clear(&searchBrowseName);
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);

    return ret;
}


static void 
deploy_bnm(UA_Client *client, bool is_listener, uint16_t listener_nr, TSN_Stream *stream)
{
    #define UA_NODEID_ENDPOINT_STREAMS_FOLDER   5002
    #define UA_NODEID_ENDPOINT_TSN_STREAM_TYPE  1003
    #define UA_NODEID_ENDPOINT_TRAFFIC_SPECIFICATION  5004

    UA_StatusCode ret;
    UA_Variant *variant;

    // Add Stream object
    UA_NodeId streamNodeObject;
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.writeMask = 4194303;
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "TSNStream");
    ret = UA_Client_addObjectNode(client, 
                            UA_NODEID_NULL, 
                            UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_STREAMS_FOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(2, "TSNStream"), 
                            UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_TSN_STREAM_TYPE), 
                            oAttr, 
                            &streamNodeObject);

    variant = UA_Variant_new();
    UA_NodeId resNodeId;


    // ------------------------------
    // StreamType
    // ------------------------------

    // Get StreamName based on the NS0 NodeID defined by BNM
    UA_NodeId ns0StreamName = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNSTREAMTYPE_STREAMNAME);
    ret = _findChildNodeID(client, streamNodeObject, ns0StreamName, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        // Write StreamName
        char *sn = malloc(sizeof("Stream_") + sizeof(stream->stream_id) + 1);
        sprintf(sn, "Stream_%s", stream->stream_id);
        UA_String streamName = UA_STRING(sn);
        UA_Variant_setScalarCopy(variant, &streamName, &UA_TYPES[UA_TYPES_STRING]);  
        free(sn);
        ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write to node 'StreamName'!\n");
        }
    } else {
        printf("[CUC][ERROR] Could not find node 'StreamName' in the stream object in the endpoint!\n");
    }  

    // StreamId
    UA_NodeId ns0StreamId = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNSTREAMTYPE_STREAMID);
    ret = _findChildNodeID(client, streamNodeObject, ns0StreamId, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        // Write Stream ID
        UA_Byte streamId[8];
        if (8 == sscanf(stream->stream_id, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx:%hhx-%hhx", &streamId[0], &streamId[1], &streamId[2], &streamId[3], &streamId[4], &streamId[5], &streamId[6], &streamId[7])) {
            UA_Variant_setArrayCopy(variant, &streamId, 8, &UA_TYPES[UA_TYPES_BYTE]);
            ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
            if (ret != UA_STATUSCODE_GOOD) {
                printf("[CUC][ERROR] Could not write to node 'StreamId'!\n");
            }
        } else {
            printf("[CUC][ERROR] Could not parse StreamID!\n");
        }

    } else {
        printf("[CUC][ERROR] Could not find node 'StreamId' in the stream object in the endpoint!\n");
    }    

    // State
    UA_NodeId ns0State = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNSTREAMTYPE_STATE);
    ret = _findChildNodeID(client, streamNodeObject, ns0State, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        // Write State
        UA_TsnStreamState state = UA_TSNSTREAMSTATE_READY;
        UA_Variant_setScalarCopy(variant, &state, &UA_TYPES[UA_TYPES_TSNSTREAMSTATE]);  
        ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write to node 'State'!\n");
        }

    } else {
        printf("[CUC][ERROR] Could not find node 'State' in the stream object in the endpoint!\n");
    }            

    // Accumulated Latency
    UA_NodeId ns0AccumulatedLatency = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNSTREAMTYPE_ACCUMULATEDLATENCY);
    ret = _findChildNodeID(client, streamNodeObject, ns0AccumulatedLatency, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        UA_UInt32 accumulatedLatency;
        if (!is_listener) {
            accumulatedLatency = stream->configuration->talker.accumulated_latency;
        } else {
            accumulatedLatency = stream->configuration->listener_list[listener_nr].accumulated_latency;
        }
    } else {
        printf("[CUC][ERROR] Could not find node 'AccumulatedLatency' in the stream object in the endpoint!\n");
    }  



    // ------------------------------
    // TrafficSpecification
    // ------------------------------
    UA_NodeId trafficSpecificationObjectNode = UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_TRAFFIC_SPECIFICATION);
    ret = _findChildNodeID(client, streamNodeObject, trafficSpecificationObjectNode, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        // Found TrafficSpecification Object as child of the created Stream root object


    }




cleanup:
    UA_Variant_delete(variant);
    //UA_Client_delete(client);

    return;
}



int 
deploy_configuration(TSN_Enddevice *enddevice, bool is_listener, uint16_t listener_nr, TSN_Stream *stream, char *app_id)
{
    printf("--- Deploying configuration to: \n");
    print_enddevice(*enddevice);
    

    //UA_Variant *my_variant;
    UA_NodeId nodeID;
    UA_StatusCode retval;
    UA_Client *client;

    // Check if the enddevice has a interface URI
    if (!enddevice || !enddevice->interface_uri) {
        printf("[CUC][ERROR] Could not deploy stream configuration to enddevice (%s) because of missing interface URI!\n", enddevice->mac);
    }

    // Connect to the server
    client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(config);
    
    retval = UA_Client_connect(client, enddevice->interface_uri);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("[CUC][ERROR] Could not connect to OPC UA Server '%s'\n", enddevice->interface_uri);
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }

    /*
    // ---------------------------------------------------------
    // Write configuration to the OPC UA Server of the enddevice
    //
    // IMPORTANT NOTE:
    // The actual data model is defined in the Part 22 Base Network Model (BNM). 
    // In the current implementation, a simplified data model is used to 
    // primarily demonstrate and validate how it works. 
    // The following configurations are currently transmitted to the endpoint:
    // - Stream ID
    // - VLAN ID
    // - Qbv Offset
    //
    // The plan for the further development is to switch to the actual BNM.
    //
    // ---------------------------------------------------------
    my_variant = UA_Variant_new();

    //UA_String string_value = UA_STRING((char *) stream_configuration->talker.interface_configuration.interface_list[0].mac_address);
    
    // Stream ID
    nodeID = UA_NODEID_STRING(1, "Stream ID");
    UA_String streamID = UA_STRING((char *) stream_id);
    UA_Variant_setScalarCopy(my_variant, &streamID, &UA_TYPES[UA_TYPES_STRING]);
    retval = UA_Client_writeValueAttribute(client, nodeID, my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("[CUC][OPCUA][ERROR] Failed to write 'Stream ID' to %s!\n", enddevice->interface_uri);
        //goto cleanup;
    } else {
        printf("[CUC][OPCUA] Successfully written 'Stream ID' to enddevice!\n");
    }

    // Here we assume following config_list (see: /src/CNC/cnc_prototype.c, from line 112)
    // [0] --> ieee802_mac_addresses
    // [1] --> ieee802_vlan_tag
    // [2] --> time_aware_offset
    // TODO:  --> Implement a function to find the correct entries for the endpoint variables

    // VLAN ID
    nodeID = UA_NODEID_STRING(1, "VLAN ID");
    UA_UInt16 vlanID = (UA_UInt16) stream_configuration->talker.interface_configuration.interface_list[0].config_list[1].ieee802_vlan_tag->vlan_id;
    UA_Variant_setScalarCopy(my_variant, &vlanID, &UA_TYPES[UA_TYPES_UINT16]);
    retval = UA_Client_writeValueAttribute(client, nodeID, my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("[CUC][OPCUA][ERROR] Failed to write 'VLAN ID' to %s!\n", enddevice->interface_uri);
        //goto cleanup;
    } else {
        printf("[CUC][OPCUA] Successfully written 'VLAN ID' to enddevice!\n");
    }

    // Qbv Offset
    nodeID = UA_NODEID_STRING(1, "Qbv Offset");
    UA_UInt32 qbvOffset = (UA_UInt32) stream_configuration->talker.interface_configuration.interface_list[0].config_list[2].time_aware_offset;
    UA_Variant_setScalarCopy(my_variant, &qbvOffset, &UA_TYPES[UA_TYPES_UINT32]);
    retval = UA_Client_writeValueAttribute(client, nodeID, my_variant);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("[CUC][OPCUA][ERROR] Failed to write 'Qbv Offset' to %s!\n", enddevice->interface_uri);
        //goto cleanup;
    } else {
        printf("[CUC][OPCUA] Successfully written 'Qbv Offset' to enddevice!\n");
    }
    */

    // ------------------------------------------------------------------
    // Deploy Configuration based on the BNM of OPC UA (Part 22)

    deploy_bnm(client, is_listener, listener_nr, stream);
    // ------------------------------------------------------------------


cleanup:
    //UA_Variant_delete(my_variant);

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
            rc = deploy_configuration(talker, false, 0, configured_stream, "N/A");
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

                rc = deploy_configuration(listener, true, i, configured_stream, "N/A");
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



    // ------------------------------------------------------------------------------------------
    /*
    printf("----------> TEST:\n");
    // Connect to the server
    UA_Client *client;
    UA_StatusCode retval;
    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    retval = UA_Client_connect(client, "opc.tcp://localhost:62541");
    if (retval != UA_STATUSCODE_GOOD) {
        printf("ERROR!!!!!!!!!!\n");
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }
    
    deploy_bnm_test(client, true, NULL, "00-00-00-00-00-00:00-01");
    */


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
