/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

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

#include "../../../src_generated/tsndep_nodeids.h"


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

    request.http_url = _concat_strings(cnc_url, CNC_INTERFACE_COMPUTE_REQUESTS);
    //request.http_url = strdup(cnc_url);
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
                    printf("[CUC] Error writing stream (ID: %s) configuration to the datastore!\n", streams->streams[i].stream_id);
                }
            }
        }
    } else {
        printf("[CUC] Failure sending request to CNC at '%s'\n", request.http_url);
        // Send an error notification
        char *notif_msg = (char *) malloc(strlen("[CUC] Failure sending request to CNC at ''") + strlen(cnc_url));
        sprintf(notif_msg, "[CUC] Failure sending request to CNC at '%s'", cnc_url);
        rc = sysrepo_send_notification(EVENT_ERROR, NULL, notif_msg);
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
_deploy_bnm_interface_configuration(UA_Client *client, UA_NodeId rootNode, UA_NodeId interfaceConfigurationTypeNode, UA_NodeId macAddressNodeObject, UA_NodeId vlanTagNodeObject, IEEE_InterfaceList *iface, bool is_listener) {
    UA_StatusCode ret;
    UA_Variant *variant;
    UA_NodeId ifaceNodeObject;
    UA_ObjectAttributes ifaceObjAttr = UA_ObjectAttributes_default;
    ifaceObjAttr.displayName = UA_LOCALIZEDTEXT("en-US", "InterfaceConfiguration");
    ret = UA_Client_addObjectNode(client,
                                  UA_NODEID_NULL,
                                  rootNode,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                  UA_QUALIFIEDNAME(2, "InterfaceConfiguration"),
                                  interfaceConfigurationTypeNode,
                                  ifaceObjAttr,
                                  &ifaceNodeObject);

    if (ret != UA_STATUSCODE_GOOD) {
        return;
    }

    variant = UA_Variant_new();
    UA_NodeId resNodeId;

    // Mac Address
    UA_NodeId ns0MacAddress = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEETSNINTERFACECONFIGURATIONTYPE_MACADDRESS);
    ret = _findChildNodeID(client, ifaceNodeObject, ns0MacAddress, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        UA_String macAddress = UA_STRING(iface->mac_address);
        UA_Variant_setScalarCopy(variant, &macAddress, &UA_TYPES[UA_TYPES_STRING]);  
        ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write to node 'MacAddress'!\n");
        }
    } else {
        printf("[CUC][ERROR] Could not find node 'MacAddress' in the stream object in the endpoint!\n");
    }  

    // Interface Name
    UA_NodeId ns0InterfaceName = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEETSNINTERFACECONFIGURATIONTYPE_INTERFACENAME);
    ret = _findChildNodeID(client, ifaceNodeObject, ns0InterfaceName, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        UA_String interfaceName = UA_STRING(iface->interface_name);
        UA_Variant_setScalarCopy(variant, &interfaceName, &UA_TYPES[UA_TYPES_STRING]);  
        ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write to node 'InterfaceName'!\n");
        }
    } else {
        printf("[CUC][ERROR] Could not find node 'InterfaceName' in the stream object in the endpoint!\n");
    }  

    
    // Config Entries
    for (int i=0; i<iface->count_config_list_entries; ++i) {
        IEEE_ConfigList *configEntry = &iface->config_list[i];

        if (configEntry->field_type == CONFIG_LIST_MAC_ADDRESSES) {
            // MacAddress Object
            UA_NodeId macAddressNode = macAddressNodeObject;
            ret = _findChildNodeID(client, ifaceNodeObject, macAddressNode, &resNodeId);
            if (ret == UA_STATUSCODE_GOOD) {
                UA_NodeId_copy(&resNodeId, &macAddressNode);

                // Destination Address
                UA_NodeId ns0DestinationAddress = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEETSNMACADDRESSTYPE_DESTINATIONADDRESS);
                ret = _findChildNodeID(client, macAddressNode, ns0DestinationAddress, &resNodeId);
                if (ret == UA_STATUSCODE_GOOD) {
                    UA_Byte destinationAddress[6];
                    if (6 == sscanf(configEntry->ieee802_mac_addresses->destination_mac_address, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &destinationAddress[0], &destinationAddress[1], &destinationAddress[2], &destinationAddress[3], &destinationAddress[4], &destinationAddress[5])) {
                        UA_Variant_setArrayCopy(variant, &destinationAddress, 6, &UA_TYPES[UA_TYPES_BYTE]);
                        ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
                        if (ret != UA_STATUSCODE_GOOD) {
                            printf("[CUC][ERROR] Could not write to node 'DestinationAddress'!\n");
                        }
                    } else {
                        printf("[CUC][ERROR] Could not parse 'DestinationAddress'!\n");
                    }

                } else {
                    printf("[CUC][ERROR] Could not find node 'DestinationAddress' in the stream object in the endpoint!\n");
                }   

                // Source Address
                UA_NodeId ns0SourceAddress = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEETSNMACADDRESSTYPE_SOURCEADDRESS);
                ret = _findChildNodeID(client, macAddressNode, ns0SourceAddress, &resNodeId);
                if (ret == UA_STATUSCODE_GOOD) {
                    UA_Byte sourceAddress[6];
                    if (6 == sscanf(configEntry->ieee802_mac_addresses->source_mac_address, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &sourceAddress[0], &sourceAddress[1], &sourceAddress[2], &sourceAddress[3], &sourceAddress[4], &sourceAddress[5])) {
                        UA_Variant_setArrayCopy(variant, &sourceAddress, 6, &UA_TYPES[UA_TYPES_BYTE]);
                        ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
                        if (ret != UA_STATUSCODE_GOOD) {
                            printf("[CUC][ERROR] Could not write to node 'SourceAddress'!\n");
                        }
                    } else {
                        printf("[CUC][ERROR] Could not parse 'SourceAddress'!\n");
                    }

                } else {
                    printf("[CUC][ERROR] Could not find node 'SourceAddress' in the stream object in the endpoint!\n");
                }   
            }


        } else if (configEntry->field_type == CONFIG_LIST_VLAN_TAG) {
            // VLAN Tag Object
            UA_NodeId vlanTagNode = vlanTagNodeObject;
            ret = _findChildNodeID(client, ifaceNodeObject, vlanTagNode, &resNodeId);
            if (ret == UA_STATUSCODE_GOOD) {
                UA_NodeId_copy(&resNodeId, &vlanTagNode);

                // VlanId
                UA_NodeId ns0VlanId = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEETSNVLANTAGTYPE_VLANID);
                ret = _findChildNodeID(client, vlanTagNode, ns0VlanId, &resNodeId); 
                if (ret == UA_STATUSCODE_GOOD) {
                    UA_UInt16 vlanId = configEntry->ieee802_vlan_tag->vlan_id;
                    UA_Variant_setScalarCopy(variant, &vlanId, &UA_TYPES[UA_TYPES_UINT16]);  
                    ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
                    if (ret != UA_STATUSCODE_GOOD) {
                        printf("[CUC][ERROR] Could not write to node 'VlanId'!\n");
                    }
                } else {
                    printf("[CUC][ERROR] Could not find node 'VlanId' in the stream object in the endpoint!\n");
                }  

                // Priority Code Point
                UA_NodeId ns0PriorityCodePoint = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEETSNVLANTAGTYPE_PRIORITYCODEPOINT);
                ret = _findChildNodeID(client, vlanTagNode, ns0PriorityCodePoint, &resNodeId); 
                if (ret == UA_STATUSCODE_GOOD) {
                    UA_Byte priorityCodePoint = configEntry->ieee802_vlan_tag->priority_code_point;
                    UA_Variant_setScalarCopy(variant, &priorityCodePoint, &UA_TYPES[UA_TYPES_BYTE]);  
                    ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
                    if (ret != UA_STATUSCODE_GOOD) {
                        printf("[CUC][ERROR] Could not write to node 'PriorityCodePoint'!\n");
                    }
                } else {
                    printf("[CUC][ERROR] Could not find node 'PriorityCodePoint' in the stream object in the endpoint!\n");
                }  
            }


        } else if (configEntry->field_type == CONFIG_LIST_TIME_AWARE_OFFSET && !is_listener) {
            // Time Aware Offset
            UA_NodeId ns0TimeAwareOffset = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEETSNINTERFACECONFIGURATIONTALKERTYPE_TIMEAWAREOFFSET);
            ret = _findChildNodeID(client, ifaceNodeObject, ns0TimeAwareOffset, &resNodeId); 
            if (ret == UA_STATUSCODE_GOOD) {
                UA_UInt32 timeAwareOffset = configEntry->time_aware_offset;
                UA_Variant_setScalarCopy(variant, &timeAwareOffset, &UA_TYPES[UA_TYPES_UINT32]);  
                ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
                if (ret != UA_STATUSCODE_GOOD) {
                    printf("[CUC][ERROR] Could not write to node 'TimeAwareOffset'!\n");
                }
            } else {
                printf("[CUC][ERROR] Could not find node 'TimeAwareOffset' in the stream object in the endpoint!\n");
            }  

        } // TODO BNM specifies ReceiveOffset for listeners but Qcc not?
    }


cleanup:
    UA_Variant_delete(variant);

    return;
}

static void 
deploy_bnm(UA_Client *client, bool is_listener, uint16_t listener_nr, TSN_Stream *stream)
{
    #define UA_NODEID_ENDPOINT_STREAMS_FOLDER               5002
    #define UA_NODEID_ENDPOINT_TSN_STREAM_TYPE              1003
    #define UA_NODEID_ENDPOINT_INTERFACE_CONFIGURATION_TYPE 1010
    #define UA_NODEID_ENDPOINT_TRAFFIC_SPECIFICATION        5004
    #define UA_NODEID_ENDPOINT_STATUS_STREAM                5005
    #define UA_NODEID_ENDPOINT_INTERFACE_CONFIGURATION      5006
    #define UA_NODEID_ENDPOINT_MAC_ADDRESS                  5007
    #define UA_NODEID_ENDPOINT_VLAN_TAG                     5008

    UA_StatusCode ret;
    UA_Variant *variant;

    // Add Stream object
    UA_NodeId streamNodeObject;
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    //oAttr.writeMask = 4194303;
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
            printf("[CUC][ERROR] Could not parse 'StreamId'!\n");
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
        UA_Variant_setScalarCopy(variant, &accumulatedLatency, &UA_TYPES[UA_TYPES_UINT32]);  
        ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write to node 'AccumulatedLatency'!\n");
        }

    } else {
        printf("[CUC][ERROR] Could not find node 'AccumulatedLatency' in the stream object in the endpoint!\n");
    }  



    // ------------------------------
    // TrafficSpecification
    // ------------------------------
    UA_NodeId trafficSpecificationNodeObject = UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_TRAFFIC_SPECIFICATION);
    ret = _findChildNodeID(client, streamNodeObject, trafficSpecificationNodeObject, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        // Found TrafficSpecification Object as child of the created Stream root object
        UA_NodeId_copy(&resNodeId, &trafficSpecificationNodeObject);


        // Max Interval Frames
        UA_NodeId ns0MaxIntervalFrames = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNTRAFFICSPECIFICATIONTYPE_MAXINTERVALFRAMES);
        ret = _findChildNodeID(client, trafficSpecificationNodeObject, ns0MaxIntervalFrames, &resNodeId);
        if (ret == UA_STATUSCODE_GOOD) {
            UA_UInt16 maxIntervalFrames = stream->request.talker.traffic_specification.max_frames_per_interval;
            UA_Variant_setScalarCopy(variant, &maxIntervalFrames, &UA_TYPES[UA_TYPES_UINT16]);  
            ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
            if (ret != UA_STATUSCODE_GOOD) {
                printf("[CUC][ERROR] Could not write to node 'MaxIntervalFrames'!\n");
            }

        } else {
            printf("[CUC][ERROR] Could not find node 'MaxIntervalFrames' in the stream object in the endpoint!\n");
        }

        // Max Frame Size
        UA_NodeId ns0MaxFrameSize = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNTRAFFICSPECIFICATIONTYPE_MAXFRAMESIZE);
        ret = _findChildNodeID(client, trafficSpecificationNodeObject, ns0MaxFrameSize, &resNodeId);
        if (ret == UA_STATUSCODE_GOOD) {
            UA_UInt32 maxFrameSize = stream->request.talker.traffic_specification.max_frame_size;
            UA_Variant_setScalarCopy(variant, &maxFrameSize, &UA_TYPES[UA_TYPES_UINT32]);  
            ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
            if (ret != UA_STATUSCODE_GOOD) {
                printf("[CUC][ERROR] Could not write to node 'MaxFrameSize'!\n");
            }

        } else {
            printf("[CUC][ERROR] Could not find node 'MaxFrameSize' in the stream object in the endpoint!\n");
        }

        // Interval
        UA_NodeId ns0Interval = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNTRAFFICSPECIFICATIONTYPE_INTERVAL);
        ret = _findChildNodeID(client, trafficSpecificationNodeObject, ns0Interval, &resNodeId);
        if (ret == UA_STATUSCODE_GOOD) {
            UA_UnsignedRationalNumber interval;
            interval.numerator = stream->request.talker.traffic_specification.interval.numerator;
            interval.denominator = stream->request.talker.traffic_specification.interval.denominator;
            UA_Variant_setScalarCopy(variant, &interval, &UA_TYPES[UA_TYPES_UNSIGNEDRATIONALNUMBER]);  
            ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
            if (ret != UA_STATUSCODE_GOOD) {
                printf("[CUC][ERROR] Could not write to node 'Interval'!\n");
            }

        } else {
            printf("[CUC][ERROR] Could not find node 'Interval' in the stream object in the endpoint!\n");
        }
    }


    // ------------------------------
    // StatusStream
    // ------------------------------
    UA_NodeId statusStreamNodeObject = UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_STATUS_STREAM);
    ret = _findChildNodeID(client, streamNodeObject, statusStreamNodeObject, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        // Found StatusStream Object as child of the created Stream root object
        UA_NodeId_copy(&resNodeId, &statusStreamNodeObject);


        // Failure Code
        UA_NodeId ns0Failurecode = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNSTATUSSTREAMTYPE_FAILURECODE);
        ret = _findChildNodeID(client, statusStreamNodeObject, ns0Failurecode, &resNodeId);
        if (ret == UA_STATUSCODE_GOOD) {  
            UA_TsnFailureCode failureCode = (UA_TsnFailureCode) stream->configuration->status_info.failure_code;
            UA_Variant_setScalarCopy(variant, &failureCode, &UA_TYPES[UA_TYPES_TSNFAILURECODE]);
            ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
            if (ret != UA_STATUSCODE_GOOD) {
                printf("[CUC][ERROR] Could not write to node 'FailureCode'!\n");
            }

        } else {
            printf("[CUC][ERROR] Could not find node 'FailureCode' in the stream object in the endpoint!\n");
        }

        // Talker Status
        UA_NodeId ns0TalkerStatus = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNSTATUSSTREAMTYPE_TALKERSTATUS);
        ret = _findChildNodeID(client, statusStreamNodeObject, ns0TalkerStatus, &resNodeId);
        if (ret == UA_STATUSCODE_GOOD) {
            UA_TsnTalkerStatus talkerStatus = (UA_TsnTalkerStatus) stream->configuration->status_info.talker_status;
            UA_Variant_setScalarCopy(variant, &talkerStatus, &UA_TYPES[UA_TYPES_TSNTALKERSTATUS]);  
            ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
            if (ret != UA_STATUSCODE_GOOD) {
                printf("[CUC][ERROR] Could not write to node 'TalkerStatus'!\n");
            }

        } else {
            printf("[CUC][ERROR] Could not find node 'TalkerStatus' in the stream object in the endpoint!\n");
        }

        // Listener Status
        UA_NodeId ns0ListenerStatus = UA_NODEID_NUMERIC(0, UA_NS0ID_IIEEEBASETSNSTATUSSTREAMTYPE_LISTENERSTATUS);
        ret = _findChildNodeID(client, statusStreamNodeObject, ns0ListenerStatus, &resNodeId);
        if (ret == UA_STATUSCODE_GOOD) {
            UA_TsnListenerStatus listenerStatus = (UA_TsnListenerStatus) stream->configuration->status_info.listener_status;
            UA_Variant_setScalarCopy(variant, &listenerStatus, &UA_TYPES[UA_TYPES_TSNLISTENERSTATUS]);  
            ret = UA_Client_writeValueAttribute(client, resNodeId, variant);
            if (ret != UA_STATUSCODE_GOOD) {
                printf("[CUC][ERROR] Could not write to node 'ListenerStatus'!\n");
            }

        } else {
            printf("[CUC][ERROR] Could not find node 'ListenerStatus' in the stream object in the endpoint!\n");
        }

        // Failure System Identifier
        // --> TODO ...
    }


    // ------------------------------
    // InterfaceConfiguration
    // ------------------------------
    UA_NodeId interfaceConfigurationNodeObject = UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_INTERFACE_CONFIGURATION);
    ret = _findChildNodeID(client, streamNodeObject, interfaceConfigurationNodeObject, &resNodeId);
    if (ret == UA_STATUSCODE_GOOD) {
        // Found InterfaceConfiguration Object as child of the created Stream root object
        UA_NodeId_copy(&resNodeId, &interfaceConfigurationNodeObject);

        // Create Object for each interface
        if (!is_listener) {
            // Talker interfaces
            for (int i=0; i<stream->configuration->talker.interface_configuration.count_interface_list_entries; ++i) {
                IEEE_InterfaceList *iface = &stream->configuration->talker.interface_configuration.interface_list[i];
                _deploy_bnm_interface_configuration(client, 
                                                    interfaceConfigurationNodeObject, 
                                                    UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_INTERFACE_CONFIGURATION_TYPE), 
                                                    UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_MAC_ADDRESS),
                                                    UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_VLAN_TAG),
                                                    iface,
                                                    is_listener);
            }

        } else {
            // Listener interfaces
            for (int i=0; i<stream->configuration->listener_list[listener_nr].interface_configuration.count_interface_list_entries; ++i) {
                IEEE_InterfaceList *iface = &stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[i];
                _deploy_bnm_interface_configuration(client, 
                                                    interfaceConfigurationNodeObject, 
                                                    UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_INTERFACE_CONFIGURATION_TYPE), 
                                                    UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_MAC_ADDRESS),
                                                    UA_NODEID_NUMERIC(2, UA_NODEID_ENDPOINT_VLAN_TAG),
                                                    iface,
                                                    is_listener);
            }

        }
    }



cleanup:
    UA_Variant_delete(variant);
    //UA_Client_delete(client);

    return;
}

/**
 * @brief Deploys the configuration for the communication to the endpoint. 
 * The concept is to use the endpoint architecture (e.g. IEEE CAMAD 2023) in the future. 
 * There to configuration is splittet into communication, application and deployment.
 * The endpoint has one interface to receive that configuration (here e.g. OPC UA). 
 * All configuration data is stored centraly and the deployed applications on the endpoint can get their configuration by an provided interface.
 *  
*/
static void
deploy_communication_engineering(UA_Client *client, bool is_listener, uint16_t listener_nr, TSN_Stream *stream, TSN_Enddevice *enddevice)
{
    int rc;
    UA_StatusCode ret;
    UA_Variant *variant;
    UA_NodeId nodeId;
    TSN_Apps *all_apps = malloc(sizeof(TSN_Apps));
    TSN_App *app_talker = NULL;
    TSN_App *app_listener = NULL;
    rc = sysrepo_get_application_apps(&all_apps);
    if (rc != EXIT_SUCCESS) {
        printf("[CUC][ERROR] Could not read Apps from datastore. Abort!\n");
        free(all_apps);
        return;
    }

    variant = UA_Variant_new();

    // BaseTime
    // ... (currently not used as part of the engineering)

    // CycleTime
    nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_CYCLETIME);
    UA_Double interval = (UA_Double)stream->request.talker.traffic_specification.interval.numerator / (UA_Double)stream->request.talker.traffic_specification.interval.denominator;
    UA_UInt32 cycleTimeMicroSeconds = interval * 1000 * 1000;
    UA_Variant_setScalarCopy(variant, &cycleTimeMicroSeconds, &UA_TYPES[UA_TYPES_UINT32]);
    ret = UA_Client_writeValueAttribute(client, nodeId, variant);
    if (ret != UA_STATUSCODE_GOOD) {
        printf("[CUC][ERROR] Could not write CycleTime to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
    }
    
    if (!is_listener) {
        // ----------------
        // Is Talker
        // ----------------

        // Interface
        nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_INTERFACE);
        UA_String interface;
        // Find the Talker App to get the specified interface
        for (int i=0; i<all_apps->count_apps; ++i) {
            for (int j=0; j<all_apps->apps[i].stream_mapping.count_egress; ++j) {
                if (strcmp(all_apps->apps[i].stream_mapping.egress[j], stream->stream_id) == 0) {
                    app_talker = &all_apps->apps[i];
                    interface = UA_String_fromChars(app_talker->iface);
                }
            }
        }
        if (app_talker == NULL) {
            // Use the physical interface specified in the stream as a backup
            //interface = UA_String_fromChars(stream->request.talker.end_station_interfaces[0].interface_name);
            interface = UA_String_fromChars(stream->configuration->talker.interface_configuration.interface_list[0].interface_name);
        }
        UA_Variant_setScalarCopy(variant, &interface, &UA_TYPES[UA_TYPES_STRING]);
        ret = UA_Client_writeValueAttribute(client, nodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write Interface to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        } 

        // QbvOffset
        nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_QBVOFFSET);
        UA_UInt32 qbvOffset = 0;
        for (int i=0; i<stream->configuration->talker.interface_configuration.interface_list[0].count_config_list_entries; ++i) {
            if (stream->configuration->talker.interface_configuration.interface_list[0].config_list[i].field_type == CONFIG_LIST_TIME_AWARE_OFFSET) {
                qbvOffset = stream->configuration->talker.interface_configuration.interface_list[0].config_list[i].time_aware_offset;
            }
        }
        UA_UInt32 qbvOffsetMicroSeconds = qbvOffset / 1000; // qbvOffset in stream config is in nanoseconds
        UA_Variant_setScalarCopy(variant, &qbvOffsetMicroSeconds, &UA_TYPES[UA_TYPES_UINT32]);
        ret = UA_Client_writeValueAttribute(client, nodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write QbvOffset to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        }

        // DestinationMAC
        nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_DESTINATIONMAC);
        UA_String destinationMAC;
        for (int i=0; i<stream->configuration->talker.interface_configuration.interface_list[0].count_config_list_entries; ++i) {
            if (stream->configuration->talker.interface_configuration.interface_list[0].config_list[i].field_type == CONFIG_LIST_MAC_ADDRESSES) {
                destinationMAC = UA_String_fromChars(stream->configuration->talker.interface_configuration.interface_list[0].config_list[i].ieee802_mac_addresses->destination_mac_address);
            }
        }
        UA_Variant_setScalarCopy(variant, &destinationMAC, &UA_TYPES[UA_TYPES_STRING]);
        ret = UA_Client_writeValueAttribute(client, nodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write DestinationMAC to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        }

        // SocketPriority
        nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_SOCKETPRIORITY);
        UA_Byte socketPriority;
        for (int i=0; i<stream->configuration->talker.interface_configuration.interface_list[0].count_config_list_entries; ++i) {
            if (stream->configuration->talker.interface_configuration.interface_list[0].config_list[i].field_type == CONFIG_LIST_VLAN_TAG) {
                socketPriority = stream->configuration->talker.interface_configuration.interface_list[0].config_list[i].ieee802_vlan_tag->priority_code_point;
            }
        }
        UA_Variant_setScalarCopy(variant, &socketPriority, &UA_TYPES[UA_TYPES_BYTE]);
        ret = UA_Client_writeValueAttribute(client, nodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write SocketPriority to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        }

    } else {
        // ----------------
        // Is Listener 
        // ----------------

        // Interface
        nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_INTERFACE);
        UA_String interface;
        // Find the Listener App to get the specified interface
        for (int i=0; i<all_apps->count_apps; ++i) {
            for (int j=0; j<all_apps->apps[i].stream_mapping.count_ingress; ++j) {
                if (strcmp(all_apps->apps[i].stream_mapping.ingress[j], stream->stream_id) == 0) {
                    app_listener = &all_apps->apps[i];
                    //interface = UA_String_fromChars(app_listener->iface);
                    //app_listener = &all_apps->apps[i];
                    // also check if the app is part of the enddevice
                    for (int k=0; k<enddevice->count_apps; ++k) {
                        if (strcmp(enddevice->apps[k].app_ref, app_listener->id) == 0) {
                            interface = UA_String_fromChars(app_listener->iface);
                        }
                    }
                }
            }
        }
        if (app_listener == NULL) {
            // Use the physical interface specified in the stream as a backup
            //interface = UA_String_fromChars(stream->request.talker.end_station_interfaces[0].interface_name);
            interface = UA_String_fromChars(stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[0].interface_name);
        }
        UA_Variant_setScalarCopy(variant, &interface, &UA_TYPES[UA_TYPES_STRING]);
        ret = UA_Client_writeValueAttribute(client, nodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write Interface to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        } 

        // SourceMAC
        nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_SOURCEMAC);
        UA_String sourceMAC;
        for (int i=0; i<stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[0].count_config_list_entries; ++i) {
            if (stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[0].config_list[i].field_type == CONFIG_LIST_MAC_ADDRESSES) {
                sourceMAC = UA_String_fromChars(stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[0].config_list[i].ieee802_mac_addresses->source_mac_address);
            }
        }
        UA_Variant_setScalarCopy(variant, &sourceMAC, &UA_TYPES[UA_TYPES_STRING]);
        ret = UA_Client_writeValueAttribute(client, nodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write SourceMAC to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        }

        // SocketPriority
        nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_SOCKETPRIORITY);
        UA_Byte socketPriority;
        for (int i=0; i<stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[0].count_config_list_entries; ++i) {
            if (stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[0].config_list[i].field_type == CONFIG_LIST_VLAN_TAG) {
                socketPriority = stream->configuration->listener_list[listener_nr].interface_configuration.interface_list[0].config_list[i].ieee802_vlan_tag->priority_code_point;
            }
        }
        UA_Variant_setScalarCopy(variant, &socketPriority, &UA_TYPES[UA_TYPES_BYTE]);
        ret = UA_Client_writeValueAttribute(client, nodeId, variant);
        if (ret != UA_STATUSCODE_GOOD) {
            printf("[CUC][ERROR] Could not write SocketPriority to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
        }
    }

    // SendReceiveEnabled   --> Set to FALSE initially 
    nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_SENDRECEIVEENABLED);
    UA_Boolean sendReceiveEnabled = UA_FALSE;
    UA_Variant_setScalarCopy(variant, &sendReceiveEnabled, &UA_TYPES[UA_TYPES_BOOLEAN]);
    ret = UA_Client_writeValueAttribute(client, nodeId, variant);
    if (ret != UA_STATUSCODE_GOOD) {
        printf("[CUC][ERROR] Could not write SendReceiveEnabled to Node ns=%d;i=%d!\n", nodeId.namespaceIndex, nodeId.identifier.numeric);
    }

    // Trigger the re-configuration of the PubSub connection
    nodeId = UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING_TRIGGERCONFIGURATION);
    ret = UA_Client_call(client, UA_NODEID_NUMERIC(7, UA_TSNDEP_ID_PUBSUBENGINEERING), nodeId, 0, NULL, 0, NULL);
    if (ret != UA_STATUSCODE_GOOD) {
        printf("[CUC][ERROR] Could not trigger re-configuration!\n");
    }

    printf("[CUC] Engineering parameters written to app '%s' [%s]. Waiting for START signal...\n\n", !is_listener ? app_talker->name : app_listener->name, !is_listener ? "TALKER" : "LISTENER");

cleanup:
    UA_Variant_delete(variant);
    free(all_apps);
    
    return;
}

int 
deploy_configuration(TSN_Enddevice *enddevice, bool is_listener, uint16_t listener_nr, TSN_Stream *stream, char *app_id)
{
    UA_NodeId nodeID;
    UA_StatusCode retval;
    UA_Client *client;

    // Check if the enddevice has a interface URI
    if (!enddevice || !enddevice->interface_uri) {
        printf("[CUC][ERROR] Could not deploy stream configuration to enddevice (%s) because of missing interface URI!\n", enddevice->mac);
        // Send an error notification
        char *notif_msg = (char *) malloc(strlen("[CUC] Could not deploy stream configuration to enddevice () because of missing interface URI!") + strlen(enddevice->mac));
        sprintf(notif_msg, "[CUC] Could not deploy stream configuration to enddevice (%s) because of missing interface URI!", enddevice->mac);
        rc = sysrepo_send_notification(EVENT_ERROR, NULL, notif_msg);
    }

    // Connect to the server
    client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(config);
    
    retval = UA_Client_connect(client, enddevice->interface_uri);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("[CUC][ERROR] Could not connect to OPC UA Server '%s'\n", enddevice->interface_uri);
        // Send an error notification
        char *notif_msg = (char *) malloc(strlen("[CUC] Could not connect to OPC UA Server ''") + strlen(enddevice->interface_uri));
        sprintf(notif_msg, "[CUC] Could not connect to OPC UA Server '%s'", enddevice->interface_uri);
        rc = sysrepo_send_notification(EVENT_ERROR, NULL, notif_msg);
        
        UA_Client_delete(client);
        printf("[CUC] OPC UA client closed\n");
        return EXIT_FAILURE;
    }

    // Deploy Configuration based on the BNM of OPC UA (Part 22)
    // deploy_bnm(client, is_listener, listener_nr, stream);

    // Deploy Engineering configuration base on Endpoint concept (spliited into Configuration for communication, application and deployment)
    // see e.g. conference Paper IEEE CAMAD
    deploy_communication_engineering(client, is_listener, listener_nr, stream, enddevice);

cleanup:

    UA_Client_delete(client);
    printf("[CUC] OPC UA client closed\n");

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
    else if (data.event_id == EVENT_STREAM_REQUESTED) {
        // Check for join/leave Listener on an configured stream 
        // by comparing the listener count of the request and the count of the configuration
        TSN_Stream *stream = malloc(sizeof(TSN_Stream));
        rc = streams_get(data.entry_id, &stream); 
        if (rc == EXIT_SUCCESS) {
            if (stream->configured == 1) {
                if (stream->request.count_listeners > stream->configuration->count_listeners) {
                    // Listeners joint
                    rc = sysrepo_set_stream_unconfigured(stream->stream_id);
                    if (rc == EXIT_SUCCESS) {
                        TSN_Streams *streams = malloc(sizeof(TSN_Streams));
                        streams->count_streams = 1;
                        streams->streams = malloc(sizeof(TSN_Stream));
                        streams->streams[0] = *stream;
                        printf("[CUC][CB] Listener(s) joint! Sending new request to CNC\n");
                        cnc_compute_requests(streams);

                        free(streams);
                    }

                } else if (stream->request.count_listeners < stream->configuration->count_listeners) {
                    // Listeners left
                    // TODO
                    printf("Not implemented yet!\n");
                }
            }
        }
        free(stream);
    }
    */

    else if (data.event_id == EVENT_STREAM_LISTENER_JOINED
             || data.event_id == EVENT_STREAM_LISTENER_LEFT) {
        
        TSN_Stream *stream = malloc(sizeof(TSN_Stream));
        rc = streams_get(data.entry_id, &stream); 
        if (rc == EXIT_SUCCESS) {
            // Reset the configured flag first
            rc = sysrepo_set_stream_unconfigured(stream->stream_id);
            if (rc == EXIT_SUCCESS) {
                // Send updated request to CNC for computation
                TSN_Streams *streams = malloc(sizeof(TSN_Streams));
                streams->count_streams = 1;
                streams->streams = malloc(sizeof(TSN_Stream));
                streams->streams[0] = *stream;
                if (data.event_id == EVENT_STREAM_LISTENER_JOINED) {
                    printf("[CUC][CB] Listener(s) joint! Sending updated request to CNC\n");
                } else {
                    printf("[CUC][CB] Listener(s) left! Sending updated request to CNC\n");
                }
                cnc_compute_requests(streams);

                free(streams);
            }
        }
        free(stream);
    }

    else if (data.event_id == EVENT_STREAM_DELETED) {
        printf("[CUC][CB] Stream was deleted!\n");
        // TODO: Handle deleted stream --> inform CNC?
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

    // (data.event_id == EVENT_STREAM_CONFIGURED) removed so that the stream isn't automatically deployed!
    else if (data.event_id == EVENT_STREAM_DEPLOY) {  
        //if (data.event_id == EVENT_STREAM_CONFIGURED) {
        //    printf("[CUC][CB] Stream '%s' configured! Deploying the configuration to the enddevices...\n", data.entry_id);
        //} else {
        //    printf("[CUC][CB] Deploy configuration of stream '%s' to the enddevices \n", data.entry_id);
        //}
        printf("[CUC][CB] Deploy configuration of stream '%s' to the enddevices \n", data.entry_id);

        uint16_t success_counter = 0;
        
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
            } else {
                success_counter++;
            }
            free(talker);

            TSN_Enddevice *listener = malloc(sizeof(TSN_Enddevice));
            // Listeners
            for (uint16_t i=0; i<configured_stream->configuration->count_listeners; ++i) {
                rc = sysrepo_get_enddevice(configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address, &listener);
                if (rc != EXIT_SUCCESS) {
                    printf("[CUC][CB][ERROR] Could not read enddevice (listener #%d) with MAC '%s' from the datastore!\n", i, configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address);
                }

                rc = deploy_configuration(listener, true, i, configured_stream, "N/A");
                if (rc != EXIT_SUCCESS) {
                    printf("[CUC][CB][ERROR] Could not deploy stream configuration to enddevice (listener #%d, %s)!\n", i, configured_stream->configuration->listener_list[i].interface_configuration.interface_list[0].mac_address);
                } else {
                    success_counter++;
                }
            }
            free(listener);

        } else {
            printf("[CUC][CB][ERROR] Could not read applications from the datastore!\n");
        }

        uint16_t participants_count = configured_stream->configuration->count_listeners + 1;
        if (participants_count == success_counter) {
            // Send out an event to indicate the deployed stream
            rc = sysrepo_send_notification(EVENT_STREAM_DEPLOYED, configured_stream->stream_id, "stream configuration deployed!");
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
