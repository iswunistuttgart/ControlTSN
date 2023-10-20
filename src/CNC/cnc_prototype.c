/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#include <stdio.h>
#include <ulfius.h>
#include <signal.h>
#include <string.h>

#include "cnc_prototype.h"

#include "../helper/json_serializer.h" // Just for the emulated topology discovering


#define EMULATE_OPENCNC      false



int rc;
volatile sig_atomic_t is_running = 1;
struct _u_instance server_instance;


static json_t *
_test_emulate_discovered_topology()
{
    TSN_Enddevice *e1 = NULL;
    TSN_Enddevice *e2 = NULL;
    TSN_Switch *s1 = NULL;
    TSN_Devices *d = NULL;
    TSN_Connection *c1 = NULL;
    TSN_Connection *c2 = NULL;
    TSN_Graph *g = NULL;
    TSN_Topology *t = NULL;

    // 2 Enddevices
    e1 = malloc(sizeof(TSN_Enddevice));
    e1->mac = strdup("00-00-00-00-00-01");
    e1->has_app = 0;
    //e1->app_ref = NULL;
    
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

    return serialize_topology(t);
}

static json_t *
_test_emulate_stream_computation(TSN_Streams *streams)
{
    for (int s=0; s<streams->count_streams; ++s) {

        TSN_Configuration *conf = malloc(sizeof(TSN_Configuration));
        TSN_Stream *stream = &streams->streams[s];

        // Stream Status
        conf->status_info.talker_status     = 1;
        conf->status_info.listener_status   = 1;
        conf->status_info.failure_code      = 0;

        // Failed Interface
        conf->count_failed_interfaces = 0;
        
        // Talker
        TSN_StatusTalker status_talker;
        status_talker.accumulated_latency   = stream->request.talker.user_to_network_requirements.max_latency;  // Set to users max latency requirement
        IEEE_InterfaceConfiguration interface_configuration_talker;
        // Assuming only one interface per device for now
        interface_configuration_talker.count_interface_list_entries = 1;
        interface_configuration_talker.interface_list = malloc(sizeof(IEEE_InterfaceConfiguration) * interface_configuration_talker.count_interface_list_entries);
        IEEE_InterfaceList *interface_list_talker = malloc(sizeof(IEEE_InterfaceList));
        interface_list_talker->mac_address = strdup(stream->request.talker.end_station_interfaces[0].mac_address);
        interface_list_talker->interface_name = strdup(stream->request.talker.end_station_interfaces[0].interface_name);
        
        // Assuming fixed config list containing container 'VLAN tag (in data-frame-specification)' and 'time aware (not user defined --> +1)'
        interface_list_talker->count_config_list_entries = stream->request.talker.count_data_frame_specifications + 1;
        interface_list_talker->config_list = malloc(sizeof(IEEE_ConfigList) * interface_list_talker->count_config_list_entries);
        // Iterate over the data frame specifications on the request
        for (int i=0; i<stream->request.talker.count_data_frame_specifications; ++i) {
            if (stream->request.talker.data_frame_specification[i].field_type == CONFIG_LIST_MAC_ADDRESSES) {               
                // Mac Addresses (in case of multiple listeners we use the boradcast address for now --> would be a multicast address)
                IEEE_ConfigList *cl = malloc(sizeof(IEEE_ConfigList));
                cl->index = i;
                cl->field_type = CONFIG_LIST_MAC_ADDRESSES;
                cl->ieee802_mac_addresses = malloc(sizeof(IEEE_MacAddresses));
                cl->ieee802_mac_addresses->source_mac_address = strdup(stream->request.talker.data_frame_specification[i].ieee802_mac_addresses->source_mac_address);
                //cl->ieee802_mac_addresses->destination_mac_address = strdup(stream->request.talker.data_frame_specification[i].ieee802_mac_addresses->destination_mac_address);
                //cl->ieee802_mac_addresses->destination_mac_address = strdup("FF-FF-FF-FF-FF-FF");

                // Distinguish between 1:1 and 1:n connections
                if (stream->request.count_listeners == 1) {
                    cl->ieee802_mac_addresses->destination_mac_address = strdup(stream->request.talker.data_frame_specification[i].ieee802_mac_addresses->destination_mac_address);
                } else {
                    // In case of multiple listeners send as broadcast 
                    // TODO: Should be multicast?!
                    cl->ieee802_mac_addresses->destination_mac_address = strdup("FF-FF-FF-FF-FF-FF");
                }

                interface_list_talker->config_list[i] = *cl;
            }
            else if (stream->request.talker.data_frame_specification[i].field_type == CONFIG_LIST_VLAN_TAG) {
                // VLAN Tag
                IEEE_ConfigList *cl = malloc(sizeof(IEEE_ConfigList));
                cl->index = i;
                cl->field_type = CONFIG_LIST_VLAN_TAG;
                cl->ieee802_vlan_tag = malloc(sizeof(IEEE_VlanTag));
                cl->ieee802_vlan_tag->priority_code_point = stream->request.talker.data_frame_specification[i].ieee802_vlan_tag->priority_code_point;
                cl->ieee802_vlan_tag->vlan_id = stream->request.talker.data_frame_specification[i].ieee802_vlan_tag->vlan_id;
                interface_list_talker->config_list[i] = *cl;
            }
        }
        // Time Aware Offset is NOT in the request
        // --> Therefore we add it manually with a fixed qbv offset for now
        IEEE_ConfigList *cl = malloc(sizeof(IEEE_ConfigList));
        cl->index = interface_list_talker->count_config_list_entries - 1;
        cl->field_type = CONFIG_LIST_TIME_AWARE_OFFSET;
        cl->time_aware_offset = 120 * 1000; // [ns] = 100µs
        interface_list_talker->config_list[cl->index] = *cl;

        interface_configuration_talker.interface_list = interface_list_talker;
        status_talker.interface_configuration = interface_configuration_talker;
        conf->talker = status_talker;

        // Listeners
        conf->count_listeners = stream->request.count_listeners;
        TSN_StatusListener *listener_list = malloc(sizeof(TSN_StatusListener) * conf->count_listeners);
        for (int i=0; i<conf->count_listeners; ++i) {
            TSN_Listener *listener = &stream->request.listener_list[i];
        
            TSN_StatusListener *status_listener = malloc(sizeof(TSN_StatusListener));
            status_listener->index = i;
            status_listener->accumulated_latency = status_talker.accumulated_latency;       // Assume the same

            IEEE_InterfaceConfiguration interface_configuration_listener;
            interface_configuration_listener.count_interface_list_entries = 1;
            interface_configuration_listener.interface_list = malloc(sizeof(IEEE_InterfaceList) * interface_configuration_listener.count_interface_list_entries);
            IEEE_InterfaceList *interface_list_listener = malloc(sizeof(IEEE_InterfaceList));
            interface_list_listener->mac_address = strdup(listener->end_station_interfaces[0].mac_address);
            interface_list_listener->interface_name = strdup(listener->end_station_interfaces[0].interface_name);

            interface_list_listener->count_config_list_entries = interface_list_talker->count_config_list_entries - 1;  // same as talker without the VLAN tag
            interface_list_listener->config_list = malloc(sizeof(IEEE_ConfigList) * interface_list_listener->count_config_list_entries);

            // Iterate over the config list of the talker to use the same for the listener
            int index = 0;
            for (int j=0; j<interface_list_talker->count_config_list_entries; ++j) {
                if (interface_list_talker->config_list[j].field_type == CONFIG_LIST_MAC_ADDRESSES) {
                    // Mac addresses
                    IEEE_ConfigList *cl = malloc(sizeof(IEEE_ConfigList));
                    cl->index = index;
                    cl->field_type = CONFIG_LIST_MAC_ADDRESSES;
                    cl->ieee802_mac_addresses = malloc(sizeof(IEEE_MacAddresses));
                    //cl->ieee802_mac_addresses->source_mac_address = strdup(interface_list_talker->config_list[j].ieee802_mac_addresses->source_mac_address);
                    //cl->ieee802_mac_addresses->source_mac_address = strdup("FF-FF-FF-FF-FF-FF");
                    
                    // Distinguish between 1:1 and 1:n connections
                    if (stream->request.count_listeners == 1) {
                        cl->ieee802_mac_addresses->source_mac_address = strdup(interface_list_talker->config_list[j].ieee802_mac_addresses->source_mac_address);
                    } else {
                        // In case of multiple listeners send as broadcast 
                        // TODO: Should be multicast?!

                        // NOTE: In order for PubSub the subscribers need to listen to the Destination MAC of the Publisher
                    // --> Therefore in case of broadcast the publisher sends to FF-FF ... So the listeners need to set sourcmac to boradcast too
                        cl->ieee802_mac_addresses->source_mac_address = strdup("FF-FF-FF-FF-FF-FF");
                    }
                    
                    cl->ieee802_mac_addresses->destination_mac_address = strdup(interface_list_talker->config_list[j].ieee802_mac_addresses->destination_mac_address);
                    interface_list_listener->config_list[index] = *cl;
                    index++;
                }
                else if (interface_list_talker->config_list[j].field_type == CONFIG_LIST_VLAN_TAG) {
                    // VLAN tag
                    IEEE_ConfigList *cl = malloc(sizeof(IEEE_ConfigList));
                    cl->index = index;
                    cl->field_type = CONFIG_LIST_VLAN_TAG;
                    cl->ieee802_vlan_tag = malloc(sizeof(IEEE_VlanTag));
                    cl->ieee802_vlan_tag->priority_code_point = interface_list_talker->config_list[j].ieee802_vlan_tag->priority_code_point;
                    cl->ieee802_vlan_tag->vlan_id = interface_list_talker->config_list[j].ieee802_vlan_tag->vlan_id;
                    interface_list_listener->config_list[index] = *cl;
                    index++;
                }
            }

            interface_configuration_listener.interface_list = interface_list_listener;
            status_listener->interface_configuration = interface_configuration_listener;
            listener_list[i] = *status_listener;
        }

        conf->listener_list = listener_list;
        stream->configuration = conf;
        // Set stream as configured
        stream->configured = 1;
    }

    json_t *result = serialize_streams(streams);
    // Free memory ?!?!


    return result;
}

static json_t *
_test_emulate_stream_computation_opencnc() {
    //const char *openCNC_response_test = "{\"streams\":[{\"stream-id\":\"00-00-00-00-00-01:00-01\",\"configuration\":{\"status-info\":{\"talker-status\":1,\"listener-status\":1,\"failure-code\":0},\"failed-interfaces\":[],\"status-talker-listener\":[{\"index\":0,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-00-00-01\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-0F-00-00\",\"source-mac-address\":\"00-00-00-00-00-01\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}},{\"index\":2,\"time-aware-offset\":100}]}]}},{\"index\":1,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-0F-00-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-0F-00-00\",\"source-mac-address\":\"00-00-00-00-00-01\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}}]}]}}]}},{\"stream-id\":\"00-00-00-00-00-02:00-01\",\"configuration\":{\"status-info\":{\"talker-status\":1,\"listener-status\":1,\"failure-code\":0},\"failed-interfaces\":[],\"status-talker-listener\":[{\"index\":0,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-00-00-02\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-0F-00-00\",\"source-mac-address\":\"00-00-00-00-00-02\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}},{\"index\":2,\"time-aware-offset\":120}]}]}},{\"index\":1,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-0F-00-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-0F-00-00\",\"source-mac-address\":\"00-00-00-00-00-02\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}}]}]}}]}},{\"stream-id\":\"00-00-00-00-0A-00:00-01\",\"configuration\":{\"status-info\":{\"talker-status\":1,\"listener-status\":1,\"failure-code\":0},\"failed-interfaces\":[],\"status-talker-listener\":[{\"index\":0,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-00-0A-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-0F-00-00\",\"source-mac-address\":\"00-00-00-00-0A-00\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}},{\"index\":2,\"time-aware-offset\":140}]}]}},{\"index\":1,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-0F-00-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-0F-00-00\",\"source-mac-address\":\"00-00-00-00-0A-00\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}}]}]}}]}},{\"stream-id\":\"00-00-00-0F-00-00:00-01\",\"configuration\":{\"status-info\":{\"talker-status\":1,\"listener-status\":1,\"failure-code\":0},\"failed-interfaces\":[],\"status-talker-listener\":[{\"index\":0,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-0F-00-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-00-0A-00\",\"source-mac-address\":\"00-00-00-0F-00-00\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}},{\"index\":2,\"time-aware-offset\":180}]}]}},{\"index\":1,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-00-0A-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-00-00-0A-00\",\"source-mac-address\":\"00-00-00-0F-00-00\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}}]}]}}]}},{\"stream-id\":\"00-00-00-0F-00-00:00-02\",\"configuration\":{\"status-info\":{\"talker-status\":1,\"listener-status\":1,\"failure-code\":0},\"failed-interfaces\":[],\"status-talker-listener\":[{\"index\":0,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-00-0F-00-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-0D-00-00-00\",\"source-mac-address\":\"00-00-00-0F-00-00\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}},{\"index\":2,\"time-aware-offset\":220}]}]}},{\"index\":1,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"00-00-0D-00-00-00\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"00-00-0D-00-00-00\",\"source-mac-address\":\"00-00-00-0F-00-00\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}}]}]}}]}}]}";
    const char *openCNC_response_test = "{\"streams\":[{\"stream-id\":\"48-21-0b-26-3d-8a:00-01\",\"configuration\":{\"status-info\":{\"talker-status\":1,\"listener-status\":1,\"failure-code\":0},\"failed-interfaces\":[],\"status-talker-listener\":[{\"index\":0,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"48-21-0b-26-3d-8a\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"48-21-0b-26-3f-ed\",\"source-mac-address\":\"48-21-0b-26-3d-8a\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}},{\"index\":2,\"time-aware-offset\":0}]}]}},{\"index\":1,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"48-21-0b-26-3f-ed\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"48-21-0b-26-3f-ed\",\"source-mac-address\":\"48-21-0b-26-3d-8a\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}}]}]}}]}},{\"stream-id\":\"48-21-0b-26-3f-ed:00-01\",\"configuration\":{\"status-info\":{\"talker-status\":1,\"listener-status\":1,\"failure-code\":0},\"failed-interfaces\":[],\"status-talker-listener\":[{\"index\":0,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"48-21-0b-26-3f-ed\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"48-21-0b-26-3f-42\",\"source-mac-address\":\"48-21-0b-26-3f-ed\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}},{\"index\":2,\"time-aware-offset\":100}]}]}},{\"index\":1,\"accumulated-latency\":50000,\"interface-configuration\":{\"interface-list\":[{\"interface-id\":{\"mac-address\":\"48-21-0b-26-3f-42\",\"interface-name\":\"\"},\"config-list\":[{\"index\":0,\"ieee802-mac-addresses\":{\"destination-mac-address\":\"48-21-0b-26-3f-42\",\"source-mac-address\":\"48-21-0b-26-3f-ed\"}},{\"index\":1,\"ieee802-vlan-tag\":{\"priority-code-point\":6,\"vlan-id\":0}}]}]}}]}}]}";

    json_error_t error;
    json_t *conf = json_loads(openCNC_response_test, 0, &error);
    return conf;
}

static int
_api_index(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    const char *resp = "<style>td {padding-right: 20px;} th {text-align: left;}</style><html><b>This is the ControlTSN REST Server.</b></br>" \
                       "Following endpoints are provided:</br></br>" \
                       "<table>" \
                       "<tr><th>URI</th><th>Type</th><th>Description</th><th>POST Body</th></tr>" \

                       // Index
                       "<tr><td><a href='/'>/</a></td><td>GET</td><td>This site</td></tr>" \

                       // Streams
                       "<tr><th>Streams</th></tr>" \
                       "<tr><td><a href='/streams/compute_request'>/streams/compute_request</a></td><td>POST</td><td>Computes a single stream request</td><td>stream (TSN_Stream)</td></tr>" \
                       "<tr><td><a href='/streams/compute_requests'>/streams/compute_requests</a></td><td>POST</td><td>Computes all stream requests at once</td><td>streams (TSN_Streams)</td></tr>" \

                       // Topology
                       "<tr><th>Index</th></tr>" \
                       "<tr><td><a href='/topology/discover_topology'>/topology/discover_topology</a></td><td>GET</td><td>Discover the topology</td></tr>" \
                       
                       "</table></html>";
    ulfius_set_string_body_response(response, 200, resp);

    return U_CALLBACK_CONTINUE;
}

static int
_api_topology_discover_topology(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    // TODO this is just a placeholder... 
    // in practice here should the actual discovery take place (e.g. via LLDP?)
    printf("[CNC] Discovering topology ... \n");
    sleep(1);
    printf("[CNC] Finished! Sending back discovered topology\n");

    // We emulate the discovered topology by a predefined TSN_Topology struct.
    // The struct is serialized to a JSON object and set as the response body
    json_t *body = _test_emulate_discovered_topology();
    ulfius_set_json_body_response(response, 200, body);
    json_decref(body);

    return U_CALLBACK_COMPLETE;
}

static int
_api_streams_compute_requests(const struct _u_request *request, struct _u_response *response, void *user_data)
{
    printf("[CNC] Computing Requests ...\n");

    // Get the POST body
    json_t *json_post_body = ulfius_get_json_body_request(request, NULL);
    if (json_post_body == NULL) {
        return U_CALLBACK_ERROR;
    }

#if EMULATE_OPENCNC
    // Compute the configuration for the requests
    printf("[CNC] Computing stream requests (openCNC) ... \n");
    // TODO this is just a placeholder and should be replaced by the real cnc functionality 
    json_t *body = _test_emulate_stream_computation_opencnc();
    sleep(1);
    printf("[CNC] Finished! Sending back stream configurations\n");
    
    ulfius_set_json_body_response(response, 200, body);

#else
    TSN_Streams *streams = deserialize_streams(json_post_body);
    // Compute the configuration for the requests
    printf("[CNC] Computing stream requests ... \n");
    json_t *body = _test_emulate_stream_computation(streams);
    printf("[CNC] Finished! Sending back stream configurations\n");
    ulfius_set_json_body_response(response, 200, body);
#endif
    

    json_decref(json_post_body);
    json_decref(body);

    return U_CALLBACK_COMPLETE;
}


static int
_init_server()
{
    if (ulfius_init_instance(&server_instance, PORT, NULL, NULL) != U_OK) {
        printf("[CNC] Error initializing server instance!\n");
        return EXIT_FAILURE;
    }

    // Add endpoints
    // Topology
    ulfius_add_endpoint_by_val(&server_instance, "GET", API_PREFIX, API_TOPOLOGY_DISCOVER_TOPOLOGY, 0, &_api_topology_discover_topology, NULL);
    // Streams
    ulfius_add_endpoint_by_val(&server_instance, "POST", API_PREFIX, API_STREAMS_COMPUTE_REQUESTS, 0, &_api_streams_compute_requests, NULL);
    // Index
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

    // Init web server
    rc = _init_server();
    if (rc == EXIT_FAILURE) {
        goto cleanup;
    }

    // Start the web server
    if (ulfius_start_framework(&server_instance) == U_OK) {
        printf("[CNC] REST server successfully started and listening on '%s:%d'\n", "http://localhost", server_instance.port);

        // Keep the server running
        while (is_running) {
            sleep(1);
        }

    } else {
        printf("[CNC] Error starting the server!\n");
        rc = EXIT_FAILURE;
        goto cleanup;
    }

    if (ulfius_stop_framework(&server_instance) != U_OK) {
        printf("[CNC] Error stopping server!\n");
        rc = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:
    ulfius_clean_instance(&server_instance);
    printf("[CNC] Shutting down CNC!\n");

    return rc;
}
