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
    for (int i=0; i<streams->count_streams; ++i) {

        TSN_Configuration *conf = malloc(sizeof(TSN_Configuration));

        conf->status_info.talker_status = strdup("ready");
        conf->status_info.listener_status = strdup("ready");
        conf->status_info.failure_code = 0;
        conf->count_failed_interfaces = 0;
        conf->failed_interfaces = NULL;
        
        TSN_StatusTalker status_talker;
        status_talker.accumulated_latency = 60400;
        IEEE_InterfaceConfiguration interface_configuration;
        interface_configuration.count_interface_list_entries = 1;
        interface_configuration.interface_list = malloc(sizeof(IEEE_InterfaceList) * interface_configuration.count_interface_list_entries);
        IEEE_InterfaceList *interface_list = malloc(sizeof(IEEE_InterfaceList));
        interface_list->mac_address = strdup(streams->streams[i].request.talker.end_station_interfaces[0].mac_address);
        interface_list->interface_name = strdup(streams->streams[i].request.talker.end_station_interfaces[0].interface_name);
        interface_list->count_config_list_entries = 3;
        interface_list->config_list = malloc(sizeof(IEEE_ConfigList) * interface_list->count_config_list_entries);
        IEEE_ConfigList *config_list_1 = malloc(sizeof(IEEE_ConfigList));
        config_list_1->index = 0;
        config_list_1->field_type = CONFIG_LIST_MAC_ADDRESSES;
        config_list_1->ieee802_mac_addresses = malloc(sizeof(IEEE_MacAddresses));
        config_list_1->ieee802_mac_addresses->destination_mac_address = strdup(streams->streams[i].request.listener_list[0].end_station_interfaces[0].mac_address);
        config_list_1->ieee802_mac_addresses->source_mac_address = strdup(streams->streams[i].request.talker.end_station_interfaces[0].mac_address);
        interface_list->config_list[0] = *config_list_1;
        IEEE_ConfigList *config_list_2 = malloc(sizeof(IEEE_ConfigList));
        config_list_2->index = 1;
        config_list_2->field_type = CONFIG_LIST_VLAN_TAG;
        config_list_2->ieee802_vlan_tag = malloc(sizeof(IEEE_VlanTag));
        config_list_2->ieee802_vlan_tag->priority_code_point = 6;
        config_list_2->ieee802_vlan_tag->vlan_id = 3000;
        interface_list->config_list[1] = *config_list_2;
        IEEE_ConfigList *config_list_3 = malloc(sizeof(IEEE_ConfigList));
        config_list_3->index = 2;
        config_list_3->field_type = CONFIG_LIST_TIME_AWARE_OFFSET;
        config_list_3->time_aware_offset = 100;
        interface_list->config_list[2] = *config_list_3;

        interface_configuration.interface_list = interface_list;
        status_talker.interface_configuration = interface_configuration;
        conf->talker = status_talker;

        conf->count_listeners = 1;
        TSN_StatusListener *listener_list = malloc(sizeof(TSN_StatusListener) * conf->count_listeners);
        TSN_StatusListener *status_listener = malloc(sizeof(TSN_StatusListener));
        status_listener->index = 0;
        status_listener->accumulated_latency = 60400;

        IEEE_InterfaceConfiguration interface_configuration_2;
        interface_configuration_2.count_interface_list_entries = 1;
        interface_configuration_2.interface_list = malloc(sizeof(IEEE_InterfaceList) * interface_configuration_2.count_interface_list_entries);
        IEEE_InterfaceList *interface_list_2 = malloc(sizeof(IEEE_InterfaceList));
        interface_list_2->mac_address = strdup(streams->streams[i].request.listener_list[0].end_station_interfaces[0].mac_address);
        interface_list_2->interface_name = strdup(streams->streams[i].request.listener_list[0].end_station_interfaces[0].interface_name);
        interface_list_2->count_config_list_entries = 2;
        interface_list_2->config_list = malloc(sizeof(IEEE_ConfigList) * interface_list_2->count_config_list_entries);
        
        IEEE_ConfigList *config_list_2_1 = malloc(sizeof(IEEE_ConfigList));
        config_list_2_1->index = 0;
        config_list_2_1->field_type = CONFIG_LIST_MAC_ADDRESSES;
        config_list_2_1->ieee802_mac_addresses = malloc(sizeof(IEEE_MacAddresses));
        config_list_2_1->ieee802_mac_addresses->destination_mac_address = strdup(streams->streams[i].request.listener_list[0].end_station_interfaces[0].mac_address);
        config_list_2_1->ieee802_mac_addresses->source_mac_address = strdup(streams->streams[i].request.talker.end_station_interfaces[0].mac_address);
        interface_list_2->config_list[0] = *config_list_2_1;
        IEEE_ConfigList *config_list_2_2 = malloc(sizeof(IEEE_ConfigList));
        config_list_2_2->index = 1;
        config_list_2_2->field_type = CONFIG_LIST_VLAN_TAG;
        config_list_2_2->ieee802_vlan_tag = malloc(sizeof(IEEE_VlanTag));
        config_list_2_2->ieee802_vlan_tag->priority_code_point = 6;
        config_list_2_2->ieee802_vlan_tag->vlan_id = 3000;
        interface_list_2->config_list[1] = *config_list_2_2;

        interface_configuration_2.interface_list = interface_list_2;
        status_listener->interface_configuration = interface_configuration_2;
        listener_list[0] = *status_listener;
        conf->listener_list = listener_list;
        
        streams->streams[i].configuration = conf;
        streams->streams[i].configured = 1;

    }

    return serialize_streams(streams);
}

static json_t *
_test_emulate_stream_computation_opencnc(TSN_Request *request) {
    
    // ...

    return NULL;
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
    TSN_Streams *streams = deserialize_cnc_request(json_post_body);
    // Compute the configuration for the requests
    printf("[CNC] Computing stream requests ... \n");
    // TODO this is just a placeholder and should be replaced by the real cnc functionality 
    //sleep(1);
    printf("[CNC] Finished! Sending back stream configurations\n");

    json_t *body = _test_emulate_stream_computation_opencnc(streams);
    ulfius_set_json_body_response(response, 200, body);

#else
    TSN_Streams *streams = deserialize_streams(json_post_body);
    // Compute the configuration for the requests
    printf("[CNC] Computing stream requests ... \n");
    // TODO this is just a placeholder and should be replaced by the real cnc functionality 
    //sleep(1);
    printf("[CNC] Finished! Sending back stream configurations\n");

    json_t *body = _test_emulate_stream_computation(streams);
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
