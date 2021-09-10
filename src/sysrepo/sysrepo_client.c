#include <string.h>
#include <math.h>
#include "sysrepo_client.h"

//  Sysrepo variables
int rc; // Result
sr_conn_ctx_t *connection = NULL;
sr_session_ctx_t *session = NULL;
sr_subscription_ctx_t *subscription = NULL;

// Generic callback
void (*_cb_event)(TSN_Event_CB_Data data);

//  Callbacks - OLD
// void (*_cb_stream_requested)(TSN_Stream *);
// void (*_cb_stream_configured)(TSN_Stream *);
// void (*_cb_error)();


// -------------------------------------------------------- //
//  Initialization and connection stuff
// -------------------------------------------------------- //
int 
sysrepo_connect()
{
    // Turn sysrepo logging on
    sr_log_stderr(SR_LL_WRN);
    // Connect 
    rc = sr_connect(0, &connection);
    // Start the session
    rc = sr_session_start(connection, SR_DS_RUNNING, &session);

    if (rc != SR_ERR_OK) {
        sr_disconnect(connection);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// -------------------------------------------------------- //
//  Callbacks
// -------------------------------------------------------- //
void 
sysrepo_init_callback(void (*cb_event)(TSN_Event_CB_Data))
{
    _cb_event = cb_event;
}

/*
void
sysrepo_init_callbacks(
    void (*cb_stream_requested)(TSN_Stream *),
    void (*cb_stream_configured)(TSN_Stream *),
    // ...
    void (*cb_error)()
)
{
    _cb_stream_requested = cb_stream_requested;
    _cb_stream_configured = cb_stream_configured;
    //...
    _cb_error = cb_error;
}
*/

// -------------------------------------------------------- //
//  Notification listener
// -------------------------------------------------------- //
static void
_notif_listener_cb(sr_session_ctx_t *session, const sr_ev_notif_type_t notif_type, const char *path, const sr_val_t *values,
        const size_t values_cnt, time_t timestamp, void *private_data)
{
    // ...
}

int 
sysrepo_disconnect()
{
    rc = sr_disconnect(connection);

    if (rc) {
        printf("[SYSREPO] Failure while disconnecting!\n");
        return EXIT_FAILURE;
    }

    printf("[SYSREPO] Disconnected\n");
    return EXIT_SUCCESS;
}

/*
int
sysrepo_start_listening()
{
    rc = sr_event_notif_subscribe(session, "control-tsn-uni", NULL, 0, 0, _notif_listener_cb, NULL, 0, &subscription);
    
    if (rc != SR_ERR_OK) {
        sr_disconnect(connection);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
sysrepo_stop_listening()
{
    rc = sr_unsubscribe(subscription);

    if (rc != SR_ERR_OK) {
        sr_disconnect(connection);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
*/


// -------------------------------------------------------- //
//  Static helper functions to interact with sysrepo
// -------------------------------------------------------- //
static void 
_create_xpath(char *xpath_base, char *xpath_append, char **result)
{
    (*result) = malloc(strlen(xpath_base) + strlen(xpath_append) + 1);
    sprintf((*result), "%s%s", xpath_base, xpath_append);
}

void
_create_xpath_id(char *xpath_base, int id, char **result)
{
    size_t size_needed = snprintf(NULL, 0, xpath_base, id) + 1;
    (*result) = malloc(size_needed);
    sprintf((*result), xpath_base, id); 
}

static int
_read_interface_id(char *xpath, IEEE_InterfaceId **interface_id)
{
    int rc;
    sr_val_t *val_mac_address = NULL;
    sr_val_t *val_interface_name = NULL;
    char *xpath_mac_address = NULL;
    char *xpath_interface_name = NULL;

    // Read MAC Address
    _create_xpath(xpath, "/mac-address", &xpath_mac_address);
    rc = sr_get_item(session, xpath_mac_address, 0, &val_mac_address);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*interface_id)->mac_address = strdup(val_mac_address->data.string_val);

    // Read Interface Name
    _create_xpath(xpath, "/interface-name", &xpath_interface_name);
    rc = sr_get_item(session, xpath_interface_name, 0, &val_interface_name);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*interface_id)->interface_name = strdup(val_interface_name->data.string_val);

cleanup:
    sr_free_val(val_mac_address);
    sr_free_val(val_interface_name);
    free(xpath_mac_address);
    free(xpath_interface_name);

    return rc;
}

static int
_read_ieee802_mac_addresses(char *xpath, IEEE_MacAddresses **ma)
{
    int rc;
    sr_val_t *val_destination_mac_address = NULL;
    sr_val_t *val_source_mac_address = NULL;
    char *xpath_destination_mac_address = NULL;
    char *xpath_source_mac_address = NULL;

    // Read Destination MAC Address
    _create_xpath(xpath, "/destination-mac-address", &xpath_destination_mac_address);
    rc = sr_get_item(session, xpath_destination_mac_address, 0, &val_destination_mac_address);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ma)->destination_mac_address = strdup(val_destination_mac_address->data.string_val);

    // Read Source MAC Address
    _create_xpath(xpath, "/source-mac-address", &xpath_source_mac_address);
    rc = sr_get_item(session, xpath_source_mac_address, 0, &val_source_mac_address);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ma)->source_mac_address = strdup(val_source_mac_address->data.string_val);

cleanup:
    sr_free_val(val_destination_mac_address);
    sr_free_val(val_source_mac_address);
    free(xpath_destination_mac_address);
    free(xpath_source_mac_address);

    return rc;
}

static int
_read_ieee802_vlan_tag(char *xpath, IEEE_VlanTag **vt)
{
    int rc;
    sr_val_t *val_priority_code_point = NULL;
    sr_val_t *val_vlan_id = NULL;
    char *xpath_priority_code_point = NULL;
    char *xpath_vlan_id = NULL;

    // Read Priority Code Point
    _create_xpath(xpath, "/priority-code-point", &xpath_priority_code_point);
    rc = sr_get_item(session, xpath_priority_code_point, 0, &val_priority_code_point);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*vt)->priority_code_point = val_priority_code_point->data.uint8_val;

    // Read VLAN Id
    _create_xpath(xpath, "/vlan-id", &xpath_vlan_id);
    rc = sr_get_item(session, xpath_vlan_id, 0, &val_vlan_id);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*vt)->vlan_id = val_vlan_id->data.uint16_val;

cleanup:
    sr_free_val(val_priority_code_point);
    sr_free_val(val_vlan_id);
    free(xpath_priority_code_point);
    free(xpath_vlan_id);

    return rc;
}

static int
_read_ipv4_tuple(char *xpath, IEEE_IPv4Tuple **ipv4)
{
    int rc;
    sr_val_t *val_source_ip_address = NULL;
    sr_val_t *val_destination_ip_address = NULL;
    sr_val_t *val_dscp = NULL;
    sr_val_t *val_protocol = NULL;
    sr_val_t *val_source_port = NULL;
    sr_val_t *val_destination_port = NULL;
    char *xpath_source_ip_address = NULL;
    char *xpath_destination_ip_address = NULL;
    char *xpath_dscp = NULL;
    char *xpath_protocol = NULL;
    char *xpath_source_port = NULL;
    char *xpath_destination_port = NULL;

    // Read Source IP Address
    _create_xpath(xpath, "/source-ip-address", &xpath_source_ip_address);
    rc = sr_get_item(session, xpath_source_ip_address, 0, &val_source_ip_address);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv4)->source_ip_address = strdup(val_source_ip_address->data.string_val);

    // Read Destination IP Address
    _create_xpath(xpath, "/destination-ip-address", &xpath_destination_ip_address);
    rc = sr_get_item(session, xpath_destination_ip_address, 0, &val_destination_ip_address);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv4)->destination_ip_address = strdup(val_destination_ip_address->data.string_val);

    // Read DSCP
    _create_xpath(xpath, "/dscp", &xpath_dscp);
    rc = sr_get_item(session, xpath_dscp, 0, &val_dscp);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv4)->dscp = val_dscp->data.uint8_val;

    // Read Protocol
    _create_xpath(xpath, "/protocol", &xpath_protocol);
    rc = sr_get_item(session, xpath_protocol, 0, &val_protocol);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv4)->protocol = val_protocol->data.uint16_val;

    // Read Source Port
    _create_xpath(xpath, "/source-port", &xpath_source_port);
    rc = sr_get_item(session, xpath_source_port, 0, &val_source_port);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv4)->source_port = val_source_port->data.uint16_val;

    // Read Destination Port
    _create_xpath(xpath, "/destination-port", &xpath_destination_port);
    rc = sr_get_item(session, xpath_destination_port, 0, &val_destination_port);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv4)->destination_port = val_destination_port->data.uint16_val;

cleanup:
    sr_free_val(val_source_ip_address);
    sr_free_val(val_destination_ip_address);
    sr_free_val(val_dscp);
    sr_free_val(val_protocol);
    sr_free_val(val_source_port);
    sr_free_val(val_destination_port);
    free(xpath_source_ip_address);
    free(xpath_destination_ip_address);
    free(xpath_dscp);
    free(xpath_protocol);
    free(xpath_source_port);
    free(xpath_destination_port);

    return rc;
}

static int
_read_ipv6_tuple(char *xpath, IEEE_IPv6Tuple **ipv6)
{
    int rc;
    sr_val_t *val_source_ip_address = NULL;
    sr_val_t *val_destination_ip_address = NULL;
    sr_val_t *val_dscp = NULL;
    sr_val_t *val_protocol = NULL;
    sr_val_t *val_source_port = NULL;
    sr_val_t *val_destination_port = NULL;
    char *xpath_source_ip_address = NULL;
    char *xpath_destination_ip_address = NULL;
    char *xpath_dscp = NULL;
    char *xpath_protocol = NULL;
    char *xpath_source_port = NULL;
    char *xpath_destination_port = NULL;

    // Read Source IP Address
    _create_xpath(xpath, "/source-ip-address", &xpath_source_ip_address);
    rc = sr_get_item(session, xpath_source_ip_address, 0, &val_source_ip_address);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv6)->source_ip_address = strdup(val_source_ip_address->data.string_val);

    // Read Destination IP Address
    _create_xpath(xpath, "/destination-ip-address", &xpath_destination_ip_address);
    rc = sr_get_item(session, xpath_destination_ip_address, 0, &val_destination_ip_address);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv6)->destination_ip_address = strdup(val_destination_ip_address->data.string_val);

    // Read DSCP
    _create_xpath(xpath, "/dscp", &xpath_dscp);
    rc = sr_get_item(session, xpath_dscp, 0, &val_dscp);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv6)->dscp = val_dscp->data.uint8_val;

    // Read Protocol
    _create_xpath(xpath, "/protocol", &xpath_protocol);
    rc = sr_get_item(session, xpath_protocol, 0, &val_protocol);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv6)->protocol = val_protocol->data.uint16_val;

    // Read Source Port
    _create_xpath(xpath, "/source-port", &xpath_source_port);
    rc = sr_get_item(session, xpath_source_port, 0, &val_source_port);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv6)->source_port = val_source_port->data.uint16_val;

    // Read Destination Port
    _create_xpath(xpath, "/destination-port", &xpath_destination_port);
    rc = sr_get_item(session, xpath_destination_port, 0, &val_destination_port);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ipv6)->destination_port = val_destination_port->data.uint16_val;

cleanup:
    sr_free_val(val_source_ip_address);
    sr_free_val(val_destination_ip_address);
    sr_free_val(val_dscp);
    sr_free_val(val_protocol);
    sr_free_val(val_source_port);
    sr_free_val(val_destination_port);
    free(xpath_source_ip_address);
    free(xpath_destination_ip_address);
    free(xpath_dscp);
    free(xpath_protocol);
    free(xpath_source_port);
    free(xpath_destination_port);

    return rc;
}

static int
_read_user_to_network_requirements(char *xpath, IEEE_UserToNetworkRequirements **utnr)
{
    int rc;
    sr_val_t *val_num_seamless_trees = NULL;
    sr_val_t *val_max_latency = NULL;
    char *xpath_num_seamless_trees = NULL;
    char *xpath_max_latency = NULL;

    // Read Num Seamless Trees
    _create_xpath(xpath, "/num-seamless-trees", &xpath_num_seamless_trees);
    rc = sr_get_item(session, xpath_num_seamless_trees, 0, &val_num_seamless_trees);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*utnr)->num_seamless_trees = val_num_seamless_trees->data.uint8_val;

    // Read Max Latency
    _create_xpath(xpath, "/max-latency", &xpath_max_latency);
    rc = sr_get_item(session, xpath_max_latency, 0, &val_max_latency);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*utnr)->max_latency = val_max_latency->data.uint32_val;

cleanup:
    sr_free_val(val_num_seamless_trees);
    sr_free_val(val_max_latency);
    free(xpath_num_seamless_trees);
    free(xpath_max_latency);
    
    return rc;
}

static int
_read_interface_capabilities(char *xpath, IEEE_InterfaceCapabilities **ic)
{
    int rc;
    sr_val_t *val_vlan_tag_capable = NULL;
    sr_val_t *val_cb_stream_iden_type_list = NULL;
    sr_val_t *val_cb_sequence_type_list = NULL;
    char *xpath_vlan_tag_capable = NULL;
    char *xpath_cb_stream_iden_type_list = NULL;
    char *xpath_cb_sequence_type_list = NULL;

    // Read VLAN Tag Capable
    _create_xpath(xpath, "/vlan-tag-capable", &xpath_vlan_tag_capable);
    rc = sr_get_item(session, xpath_vlan_tag_capable, 0, &val_vlan_tag_capable);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ic)->vlan_tag_capable = val_vlan_tag_capable->data.bool_val;

    // Read CB Stream Iden Type List
    _create_xpath(xpath, "/cb-stream-iden-type-list", &xpath_cb_stream_iden_type_list);
    size_t count_cb_stream_iden_types = 0;
    rc = sr_get_items(session, xpath_cb_stream_iden_type_list, 0, 0, &val_cb_stream_iden_type_list, &count_cb_stream_iden_types);
    (*ic)->count_cb_stream_iden_types = count_cb_stream_iden_types;
    (*ic)->cb_stream_iden_type_list = (uint32_t *) malloc(sizeof(uint32_t) * count_cb_stream_iden_types);
    for (int i=0; i<count_cb_stream_iden_types; ++i) {
        uint32_t cb_stream_iden;
        cb_stream_iden = (&val_cb_stream_iden_type_list[i])->data.uint32_val;
        (*ic)->cb_stream_iden_type_list[i] = cb_stream_iden;
    }

    // Read CB Sequence Type List
    _create_xpath(xpath, "/cb-sequence-type-list", &xpath_cb_sequence_type_list);
    size_t count_cb_sequence_types = 0;
    rc = sr_get_items(session, xpath_cb_sequence_type_list, 0, 0, &val_cb_sequence_type_list, &count_cb_sequence_types);
    (*ic)->count_cb_sequence_types = count_cb_sequence_types;
    (*ic)->cb_sequence_type_list = (uint32_t *) malloc(sizeof(uint32_t) * count_cb_sequence_types);
    for (int i=0; i<count_cb_sequence_types; ++i) {
        uint32_t cb_sequence_type;
        cb_sequence_type = (&val_cb_sequence_type_list[i])->data.uint32_val;
        (*ic)->cb_sequence_type_list[i] = cb_sequence_type;
    }

cleanup:
    sr_free_val(val_vlan_tag_capable);
    sr_free_val(val_cb_stream_iden_type_list);
    sr_free_val(val_cb_sequence_type_list);
    free(xpath_vlan_tag_capable);
    free(xpath_cb_stream_iden_type_list);
    free(xpath_cb_sequence_type_list);
    
    return rc;
}

static int
_read_stream_rank(char *xpath, IEEE_StreamRank **sr)
{
    int rc;
    sr_val_t *val_rank = NULL;
    char *xpath_rank = NULL;

    _create_xpath(xpath, "/rank", &xpath_rank);
    rc = sr_get_item(session, xpath_rank, 0, &val_rank);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*sr)->rank = val_rank->data.uint8_val;

cleanup:
    sr_free_val(val_rank);
    free(xpath_rank);

    return rc;
}

static int
_read_data_frame_specification(char *xpath, IEEE_DataFrameSpecification **dfs)
{
    int rc;
    sr_val_t *val_index = NULL;
    char *xpath_index = NULL;
    char *xpath_choice = NULL;

    // Read Index
    _create_xpath(xpath, "/index", &xpath_index);
    rc = sr_get_item(session, xpath_index, 0, &val_index);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*dfs)->index = val_index->data.uint8_val;

    // Read the Choice field 
    // Check and Read Mac Addresses
    _create_xpath(xpath, "/ieee802-mac-addresses", &xpath_choice);
    IEEE_MacAddresses *ma = malloc(sizeof(IEEE_MacAddresses));
    rc = _read_ieee802_mac_addresses(xpath_choice, &ma);
    // If no error occured then the choice was ieee802-mac-addresses and we can stop
    if (rc == SR_ERR_OK) {
        (*dfs)->field.ieee802_mac_addresses = *ma;
        goto cleanup;
    }  

    // Check and Read VLAN Tag
    _create_xpath(xpath, "/ieee802-vlan-tag", &xpath_choice);
    IEEE_VlanTag *vt = malloc(sizeof(IEEE_VlanTag));
    rc = _read_ieee802_vlan_tag(xpath_choice, &vt);
    // If no error occured then the choice was ieee802-vlan-tag and we can stop
    if (rc == SR_ERR_OK) {
        (*dfs)->field.ieee802_vlan_tag = *vt;
        goto cleanup;
    }

    // Check and Read IPv4 Tuple
    _create_xpath(xpath, "/ipv4-tuple", &xpath_choice);
    IEEE_IPv4Tuple *ipv4 = malloc(sizeof(IEEE_IPv4Tuple));
    rc = _read_ipv4_tuple(xpath_choice, &ipv4);
    // If no error occured then the choice was ipv4-tuple and we can stop
    if (rc == SR_ERR_OK) {
        (*dfs)->field.ipv4_tuple = *ipv4;
        goto cleanup;
    }

    // Check and Read IPv6 Tuple
    _create_xpath(xpath, "/ipv6-tuple", &xpath_choice);
    IEEE_IPv6Tuple *ipv6 = malloc(sizeof(IEEE_IPv6Tuple));
    rc = _read_ipv6_tuple(xpath_choice, &ipv6);
    // If no error occured then the choice was ipv6-tuple and we can stop
    if (rc == SR_ERR_OK) {
        (*dfs)->field.ipv6_tuple = *ipv6;
        goto cleanup;
    }

cleanup:
    sr_free_val(val_index);
    free(xpath_index);
    free(xpath_choice);

    return rc;
}

static int
_read_interval(char *xpath, IEEE_Interval **in)
{
    int rc;
    sr_val_t *val_numerator = NULL;
    sr_val_t *val_denominator = NULL;
    char *xpath_numerator = NULL;
    char *xpath_denominator = NULL;

    // Read Numerator
    _create_xpath(xpath, "/numerator", &xpath_numerator);
    rc = sr_get_item(session, xpath_numerator, 0, &val_numerator);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*in)->numerator = val_numerator->data.uint32_val;

    // Read Denominator
    _create_xpath(xpath, "/denominator", &xpath_denominator);
    rc = sr_get_item(session, xpath_denominator, 0, &val_denominator);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*in)->denominator = val_denominator->data.uint32_val;

cleanup:
    sr_free_val(val_numerator);
    sr_free_val(val_denominator);
    free(xpath_numerator);
    free(xpath_denominator);

    return rc;
}

static int
_read_time_aware(char *xpath, IEEE_TimeAware **ta)
{
    int rc;
    sr_val_t *val_earliest_transmit_offset = NULL;
    sr_val_t *val_latest_transmit_offset = NULL;
    sr_val_t *val_jitter = NULL;
    char *xpath_earliest_transmit_offset = NULL;
    char *xpath_latest_transmit_offset = NULL;
    char *xpath_jitter = NULL;

    // Read Earliest Transmit Offset
    _create_xpath(xpath, "/earliest-transmit-offset", &xpath_earliest_transmit_offset);
    rc = sr_get_item(session, xpath_earliest_transmit_offset, 0, &val_earliest_transmit_offset);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ta)->earliest_transmit_offset = val_earliest_transmit_offset->data.uint32_val;

    // Read Latest Transmit Offset
    _create_xpath(xpath, "/latest-transmit-offset", &xpath_latest_transmit_offset);
    rc = sr_get_item(session, xpath_latest_transmit_offset, 0, &val_latest_transmit_offset);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ta)->latest_transmit_offset = val_latest_transmit_offset->data.uint32_val;

    // Read Jitter
    _create_xpath(xpath, "/jitter", &xpath_jitter);
    rc = sr_get_item(session, xpath_jitter, 0, &val_jitter);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ta)->jitter = val_jitter->data.uint32_val;

cleanup:
    sr_free_val(val_earliest_transmit_offset);
    sr_free_val(val_latest_transmit_offset);
    sr_free_val(val_jitter);
    free(xpath_earliest_transmit_offset);
    free(xpath_latest_transmit_offset);
    free(xpath_jitter);

    return rc;
}

static int
_read_traffic_specification(char *xpath, IEEE_TrafficSpecification **ts)
{
    int rc;
    sr_val_t *val_max_frames_per_interval = NULL;
    sr_val_t *val_max_frame_size = NULL;
    sr_val_t *val_transmission_selection = NULL;
    char *xpath_interval = NULL;
    char *xpath_max_frames_per_interval = NULL;
    char *xpath_max_frame_size = NULL;
    char *xpath_transmission_selection = NULL;
    char *xpath_time_aware = NULL;

    // Read Interval
    _create_xpath(xpath, "/interval", &xpath_interval);
    IEEE_Interval *in = malloc(sizeof(IEEE_Interval));
    rc = _read_interval(xpath_interval, &in);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ts)->interval = *in;

    // Read Max Frames Per Interval
    _create_xpath(xpath, "/max-frames-per-interval", &xpath_max_frames_per_interval);
    rc = sr_get_item(session, xpath_max_frames_per_interval, 0, &val_max_frames_per_interval);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ts)->max_frames_per_interval = val_max_frames_per_interval->data.uint16_val;

    // Read Max Frame Size
    _create_xpath(xpath, "/max-frame-size", &xpath_max_frame_size);
    rc = sr_get_item(session, xpath_max_frame_size, 0, &val_max_frame_size);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ts)->max_frame_size = val_max_frame_size->data.uint16_val;

    // Read Transmission Selection
    _create_xpath(xpath, "/transmission-selection", &xpath_transmission_selection);
    rc = sr_get_item(session, xpath_transmission_selection, 0, &val_transmission_selection);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ts)->transmission_selection = val_transmission_selection->data.uint8_val;

    // Read Time Aware
    _create_xpath(xpath, "/time-aware", &xpath_time_aware);
    IEEE_TimeAware *ta = malloc(sizeof(IEEE_TimeAware));
    rc = _read_time_aware(xpath_time_aware, &ta);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ts)->time_aware = *ta;

cleanup:
    sr_free_val(val_max_frames_per_interval);
    sr_free_val(val_max_frame_size);
    sr_free_val(val_transmission_selection);
    free(xpath_interval);
    free(xpath_max_frames_per_interval);
    free(xpath_max_frame_size);
    free(xpath_transmission_selection);
    free(xpath_time_aware);

    return rc;
}

static int
_read_talker(char *xpath, TSN_Talker **talker)
{
    int rc;
    sr_val_t *val_end_station_interfaces = NULL;
    sr_val_t *val_data_frame_specification = NULL;
    char *xpath_stream_rank = NULL;
    char *xpath_end_station_interfaces = NULL;
    char *xpath_data_frame_specification = NULL;
    char *xpath_traffic_specification = NULL;
    char *xpath_user_to_network_requirements = NULL;
    char *xpath_interface_capabilities = NULL;

    // Read Stream Rank
    _create_xpath(xpath, "/stream-rank", &xpath_stream_rank);
    IEEE_StreamRank *sr = malloc(sizeof(IEEE_StreamRank));
    rc = _read_stream_rank(xpath_stream_rank, &sr);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*talker)->stream_rank = *sr;

    // Read End Station Interfaces
    _create_xpath(xpath, "/end-station-interfaces", &xpath_end_station_interfaces);
    size_t count_end_station_interfaces = 0;
    rc = sr_get_items(session, xpath_end_station_interfaces, 0, 0, &val_end_station_interfaces, &count_end_station_interfaces);
    (*talker)->count_end_station_interfaces = count_end_station_interfaces;
    (*talker)->end_station_interfaces = (IEEE_InterfaceId *) malloc(sizeof(IEEE_InterfaceId) * count_end_station_interfaces);
    for (int i=0; i<count_end_station_interfaces; ++i) {
        IEEE_InterfaceId *iid = malloc(sizeof(IEEE_InterfaceId));
        rc = _read_interface_id((&val_end_station_interfaces[i])->xpath, &iid);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*talker)->end_station_interfaces[i] = *iid;
    }

    // Read Data Frame Specification
    _create_xpath(xpath, "/data-frame-specification", &xpath_data_frame_specification);
    size_t count_data_frame_specification = 0;
    rc = sr_get_items(session, xpath_data_frame_specification, 0, 0, &val_data_frame_specification, &count_data_frame_specification);
    (*talker)->count_data_frame_specifications = count_data_frame_specification;
    (*talker)->data_frame_specification = (IEEE_DataFrameSpecification *) malloc(sizeof(IEEE_DataFrameSpecification) * count_data_frame_specification);
    for (int i=0; i<count_data_frame_specification; ++i) {
        IEEE_DataFrameSpecification *dfs = malloc(sizeof(IEEE_DataFrameSpecification));
        rc = _read_data_frame_specification((&val_data_frame_specification[i])->xpath, &dfs);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*talker)->data_frame_specification[i] = *dfs;
    }

    // Read Traffic Specification
    _create_xpath(xpath, "/traffic-specification", &xpath_traffic_specification);
    IEEE_TrafficSpecification *ts = malloc(sizeof(IEEE_TrafficSpecification));
    rc = _read_traffic_specification(xpath_traffic_specification, &ts);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*talker)->traffic_specification = *ts;

    // Read User To Network Requirements
    _create_xpath(xpath, "/user-to-network-requirements", &xpath_user_to_network_requirements);
    IEEE_UserToNetworkRequirements *utnr = malloc(sizeof(IEEE_UserToNetworkRequirements));
    rc = _read_user_to_network_requirements(xpath_user_to_network_requirements, &utnr);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*talker)->user_to_network_requirements = *utnr;

    // Read Interface Capabilities
    _create_xpath(xpath, "/interface-capabilities", &xpath_interface_capabilities);
    IEEE_InterfaceCapabilities *ic = malloc(sizeof(IEEE_InterfaceCapabilities));
    rc = _read_interface_capabilities(xpath_interface_capabilities, &ic);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*talker)->interface_capabilities = *ic;
    
cleanup:
    sr_free_val(val_end_station_interfaces);
    sr_free_val(val_data_frame_specification);
    free(xpath_end_station_interfaces);
    free(xpath_data_frame_specification);
    free(xpath_traffic_specification);
    free(xpath_user_to_network_requirements);
    free(xpath_interface_capabilities);

    return rc;
}

static int
_read_listener(char *xpath, TSN_Listener **listener)
{
    int rc;
    sr_val_t *val_end_station_interfaces = NULL;
    char *xpath_end_station_interfaces = NULL;
    char *xpath_user_to_network_requirements = NULL;
    char *xpath_interface_capabilities = NULL;

    // Read End Station Interfaces
    //_create_xpath(xpath, "/*[contains(.,'end-station-interfaces')]", &xpath_end_station_interfaces);
    _create_xpath(xpath, "/end-station-interfaces", &xpath_end_station_interfaces);
    size_t count_end_station_interfaces = 0;
    rc = sr_get_items(session, xpath_end_station_interfaces, 0, 0, &val_end_station_interfaces, &count_end_station_interfaces);
    (*listener)->count_end_station_interfaces = count_end_station_interfaces;
    (*listener)->end_station_interfaces = (IEEE_InterfaceId *) malloc(sizeof(IEEE_InterfaceId) * count_end_station_interfaces);
    for (int i=0; i<count_end_station_interfaces; ++i) {
        IEEE_InterfaceId *iid = malloc(sizeof(IEEE_InterfaceId));
        rc = _read_interface_id((&val_end_station_interfaces[i])->xpath, &iid);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*listener)->end_station_interfaces[i] = *iid;
    }

    // Read User To Network Requirements
    _create_xpath(xpath, "/user-to-network-requirements", &xpath_user_to_network_requirements);
    IEEE_UserToNetworkRequirements *utnr = malloc(sizeof(IEEE_UserToNetworkRequirements));
    rc = _read_user_to_network_requirements(xpath_user_to_network_requirements, &utnr);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*listener)->user_to_network_requirements = *utnr;

    // Read Interface Capabilities
    _create_xpath(xpath, "/interface-capabilities", &xpath_interface_capabilities);
    IEEE_InterfaceCapabilities *ic = malloc(sizeof(IEEE_InterfaceCapabilities));
    rc = _read_interface_capabilities(xpath_interface_capabilities, &ic);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*listener)->interface_capabilities = *ic;

cleanup:
    sr_free_val(val_end_station_interfaces);
    free(xpath_end_station_interfaces);
    free(xpath_user_to_network_requirements);
    free(xpath_interface_capabilities);

    return rc;
}

static int
_read_status_info(char *xpath, IEEE_StatusInfo **si)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_talker_status = NULL;
    sr_val_t *val_listener_status = NULL;
    sr_val_t *val_failure_code = NULL;
    char *xpath_talker_status = NULL;
    char *xpath_listener_status = NULL;
    char *xpath_failure_code = NULL;
        
    // Read talker status
    _create_xpath(xpath, "/talker-status", &xpath_talker_status);
    rc = sr_get_item(session, xpath_talker_status, 0, &val_talker_status);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    //(*si)->talker_status = val_talker_status->data.enum_val;
    (*si)->talker_status = strdup(val_talker_status->data.enum_val);

    // Read listener status
    _create_xpath(xpath, "/listener-status", &xpath_listener_status);
    rc = sr_get_item(session, xpath_listener_status, 0, &val_listener_status);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    //(*si)->listener_status = val_listener_status->data.enum_val;
    (*si)->listener_status = strdup(val_listener_status->data.enum_val);

    // Read failure code
    _create_xpath(xpath, "/failure-code", &xpath_failure_code);
    rc = sr_get_item(session, xpath_failure_code, 0, &val_failure_code);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*si)->failure_code = val_failure_code->data.uint8_val;

cleanup:
    sr_free_val(val_talker_status);
    sr_free_val(val_listener_status);
    sr_free_val(val_failure_code);
    free(xpath_talker_status);
    free(xpath_listener_status);
    free(xpath_failure_code);

    return rc;
}

static int
_read_status_stream(char *xpath, IEEE_StatusStream **ss)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_failed_interfaces = NULL;
    char *xpath_status_info = NULL;
    char *xpath_failed_interfaces = NULL;

    // Read status info
    _create_xpath(xpath, "/status-info", &xpath_status_info);
    IEEE_StatusInfo *si = malloc(sizeof(IEEE_StatusInfo));
    rc = _read_status_info(xpath_status_info, &si);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*ss)->status_info = *si;

    // Read failed interfaces
    _create_xpath(xpath, "/failed-interfaces", &xpath_failed_interfaces);
    size_t count_failed_interfaces = 0;
    rc = sr_get_items(session, xpath_failed_interfaces, 0, 0, &val_failed_interfaces, &count_failed_interfaces);
    (*ss)->failed_interfaces = (IEEE_InterfaceId *) malloc(sizeof(IEEE_InterfaceId) * count_failed_interfaces);
    (*ss)->count_failed_interfaces = count_failed_interfaces;
    for (int i=0; i<count_failed_interfaces; ++i) {
        IEEE_InterfaceId *ii = malloc(sizeof(IEEE_InterfaceId));
        rc = _read_interface_id((&val_failed_interfaces[i])->xpath, &ii);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*ss)->failed_interfaces[i] = *ii;
    }

cleanup:
    sr_free_val(val_failed_interfaces);
    free(xpath_status_info);
    free(xpath_failed_interfaces);

    return rc;
}

static int
_read_request(char *xpath, TSN_Request **req)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_listener_list = NULL;
    char *xpath_talker = NULL;
    char *xpath_listener_list = NULL;

    // Read Talker
    _create_xpath(xpath, "/talker", &xpath_talker);
    TSN_Talker *t = malloc(sizeof(TSN_Talker));
    rc = _read_talker(xpath_talker, &t);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*req)->talker = *t;

    // Read Listeners
    _create_xpath(xpath, "/listener-list/*", &xpath_listener_list);
    size_t count_listeners = 0;
    rc = sr_get_items(session, xpath_listener_list, 0, 0, &val_listener_list, &count_listeners);
    (*req)->count_listeners = count_listeners;
    (*req)->listener_list = (TSN_Listener *) malloc(sizeof(TSN_Listener) * count_listeners);
    for (int i=0; i<count_listeners; ++i) {
        TSN_Listener *l = malloc(sizeof(TSN_Listener));
        rc = _read_listener((&val_listener_list[i])->xpath, &l);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*req)->listener_list[i] = *l;
    }

cleanup:
    sr_free_val(val_listener_list);
    free(xpath_talker);
    free(xpath_listener_list);

    return rc;
}

static int
_read_config_list(char *xpath, IEEE_ConfigList **cl)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_index = NULL;
    sr_val_t *val_time_aware_offset = NULL;
    char *xpath_index = NULL;
    char *xpath_choice = NULL;

    // Read Index
    _create_xpath(xpath, "/index", &xpath_index);
    rc = sr_get_item(session, xpath_index, 0, &val_index);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*cl)->index = val_index->data.uint8_val;

    // Read the choice field
    // Check and read MAC Addresses
    _create_xpath(xpath, "/ieee802-mac-addresses", &xpath_choice);
    IEEE_MacAddresses *ma = malloc(sizeof(IEEE_MacAddresses));
    rc = _read_ieee802_mac_addresses(xpath_choice, &ma);
    // If no error occured then the choice was ieee802-mac-addresses and we can stop
    if (rc == SR_ERR_OK) {
        (*cl)->config_value.ieee802_mac_addresses = *ma;
        goto cleanup;
    }  

    // Check and Read VLAN Tag
    _create_xpath(xpath, "/ieee802-vlan-tag", &xpath_choice);
    IEEE_VlanTag *vt = malloc(sizeof(IEEE_VlanTag));
    rc = _read_ieee802_vlan_tag(xpath_choice, &vt);
    // If no error occured then the choice was ieee802-vlan-tag and we can stop
    if (rc == SR_ERR_OK) {
        (*cl)->config_value.ieee802_vlan_tag = *vt;
        goto cleanup;
    }

    // Check and Read IPv4 Tuple
    _create_xpath(xpath, "/ipv4-tuple", &xpath_choice);
    IEEE_IPv4Tuple *ipv4 = malloc(sizeof(IEEE_IPv4Tuple));
    rc = _read_ipv4_tuple(xpath_choice, &ipv4);
    // If no error occured then the choice was ipv4-tuple and we can stop
    if (rc == SR_ERR_OK) {
        (*cl)->config_value.ipv4_tuple = *ipv4;
        goto cleanup;
    }

    // Check and Read IPv6 Tuple
    _create_xpath(xpath, "/ipv6-tuple", &xpath_choice);
    IEEE_IPv6Tuple *ipv6 = malloc(sizeof(IEEE_IPv6Tuple));
    rc = _read_ipv6_tuple(xpath_choice, &ipv6);
    // If no error occured then the choice was ipv6-tuple and we can stop
    if (rc == SR_ERR_OK) {
        (*cl)->config_value.ipv6_tuple = *ipv6;
        goto cleanup;
    }

    // Check and read Time aware offset
    _create_xpath(xpath, "/time-aware-offset", &xpath_choice);
    rc = sr_get_item(session, xpath_choice, 0, &val_time_aware_offset);
    // If no error occured then the choice was time-aware-offset and we can stop
    if (rc == SR_ERR_OK) {
        (*cl)->config_value.time_aware_offset = val_time_aware_offset->data.uint32_val;
        goto cleanup;
    }

cleanup:
    sr_free_val(val_index);
    free(xpath_index);
    free(xpath_choice);

    return rc;
}

static int
_read_interface_list(char *xpath, IEEE_InterfaceList **il)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_config_list = NULL;
    char *xpath_interface_id = NULL;
    char *xpath_config_list = NULL;

    // Read interface name and MAC address
    _create_xpath(xpath, "", &xpath_interface_id);
    IEEE_InterfaceId *ii = malloc(sizeof(IEEE_InterfaceId));
    rc = _read_interface_id(xpath_interface_id, &ii);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*il)->interface_name = strdup(ii->interface_name);
    (*il)->mac_address = strdup(ii->mac_address);

    // Config list
    _create_xpath(xpath, "/config-list", &xpath_config_list);
    size_t count_configs = 0;
    rc = sr_get_items(session, xpath_config_list, 0, 0, &val_config_list, &count_configs);
    (*il)->config_list = (IEEE_ConfigList *) malloc(sizeof(IEEE_ConfigList) * count_configs);
    for (int i=0; i<count_configs; ++i) {
        IEEE_ConfigList *cl = malloc(sizeof(IEEE_ConfigList));
        rc = _read_config_list((&val_config_list[i])->xpath, &cl);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*il)->config_list[i] = *cl;
    }

cleanup:
    sr_free_val(val_config_list);
    free(xpath_interface_id);
    free(xpath_config_list);

    return rc;
}

static int
_read_interface_configuration(char *xpath, IEEE_InterfaceConfiguration **ic)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_interface_list = NULL;
    char *xpath_interface_list = NULL;

    // Read Interface list
    _create_xpath(xpath, "/interface-list", &xpath_interface_list);
    size_t count_interfaces = 0;
    rc = sr_get_items(session, xpath_interface_list, 0, 0, &val_interface_list, &count_interfaces);
    (*ic)->interface_list = (IEEE_InterfaceList *) malloc(sizeof(IEEE_InterfaceList) * count_interfaces);
    for (int i=0; i<count_interfaces; ++i) {
        IEEE_InterfaceList *il = malloc(sizeof(IEEE_InterfaceList));
        rc = _read_interface_list(xpath_interface_list, &il);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*ic)->interface_list[i] = *il;
    }

cleanup:
    sr_free_val(val_interface_list);
    free(xpath_interface_list);

    return rc;
}

static int
_read_status_talker(char *xpath, TSN_StatusTalker **st)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_accumulated_latency = NULL;
    char *xpath_accumulated_latency = NULL;
    char *xpath_interface_configuration = NULL;

    _create_xpath(xpath, "/accumulated-latency", &xpath_accumulated_latency);
    rc = sr_get_item(session, xpath_accumulated_latency, 0, &val_accumulated_latency);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*st)->accumulated_latency = val_accumulated_latency->data.uint32_val;

    // Interface configuration
    _create_xpath(xpath_accumulated_latency, "/interface-configuration", &xpath_interface_configuration);
    IEEE_InterfaceConfiguration *ic = malloc(sizeof(IEEE_InterfaceConfiguration));
    rc = _read_interface_configuration(xpath_interface_configuration, &ic);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*st)->interface_configuration = *ic;

cleanup:
    sr_free_val(val_accumulated_latency);
    free(xpath_accumulated_latency);
    free(xpath_interface_configuration);

    return rc;
}

static int
_read_status_listener(char *xpath, TSN_StatusListener **sl)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_index = NULL;
    sr_val_t *val_accumulated_latency = NULL;
    char *xpath_index = NULL;
    char *xpath_accumulated_latency = NULL;
    char *xpath_interface_configuration = NULL;

    _create_xpath(xpath, "/index", &xpath_index);
    rc = sr_get_item(session, xpath_index, 0, &val_index);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*sl)->index = val_index->data.uint16_val;

    _create_xpath(xpath, "/accumulated-latency", &xpath_accumulated_latency);
    rc = sr_get_item(session, xpath_accumulated_latency, 0, &val_accumulated_latency);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*sl)->accumulated_latency = val_accumulated_latency->data.uint32_val;

    // Interface configuration
    _create_xpath(xpath_accumulated_latency, "/interface-configuration", &xpath_interface_configuration);
    IEEE_InterfaceConfiguration *ic = malloc(sizeof(IEEE_InterfaceConfiguration));
    rc = _read_interface_configuration(xpath_interface_configuration, &ic);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*sl)->interface_configuration = *ic;

cleanup:
    sr_free_val(val_index);
    sr_free_val(val_accumulated_latency);
    free(xpath_index);
    free(xpath_accumulated_latency);
    free(xpath_interface_configuration);

    return rc;
}

static int
_read_configuration(char *xpath, TSN_Configuration **con)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_listener_list = NULL;
    char *xpath_status_stream = NULL;
    char *xpath_status_talker = NULL;
    char *xpath_listener_list = NULL;

    // Read status stream
    _create_xpath(xpath, "", &xpath_status_stream);
    IEEE_StatusStream *ss = malloc(sizeof(IEEE_StatusStream));
    rc = _read_status_stream(xpath_status_stream, &ss);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*con)->status_info = ss->status_info;
    (*con)->count_failed_interfaces = ss->count_failed_interfaces;
    (*con)->failed_interfaces = ss->failed_interfaces;

    // Read Talker
    _create_xpath(xpath, "/talker", &xpath_status_talker);
    TSN_StatusTalker *st = malloc(sizeof(TSN_StatusTalker));
    rc = _read_status_talker(xpath_status_talker, &st);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*con)->talker = *st;

    // Read listener list
    _create_xpath(xpath, "/listener-list/*", &xpath_listener_list);
    size_t count_listeners = 0;
    rc = sr_get_items(session, xpath_listener_list, 0, 0, &val_listener_list, &count_listeners);
    (*con)->count_listeners = count_listeners;
    (*con)->listener_list = (TSN_StatusListener *) malloc(sizeof(TSN_StatusListener) * count_listeners);
    for (int i=0; i<count_listeners; ++i) {
        TSN_StatusListener *sl = malloc(sizeof(TSN_StatusListener));
        rc = _read_status_listener((&val_listener_list[i])->xpath, &sl);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*con)->listener_list[i] = *sl;
    }

cleanup:
    sr_free_val(val_listener_list);
    free(xpath_status_stream);
    free(xpath_status_talker);
    free(xpath_listener_list);

    return rc;
}

static int
_read_stream(char *xpath, TSN_Stream **stream)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_stream_id = NULL;
    char *xpath_stream_id = NULL;
    char *xpath_request = NULL;
    char *xpath_configuration = NULL;

    // Stream ID
    _create_xpath(xpath, "/stream-id", &xpath_stream_id);
    rc = sr_get_item(session, xpath_stream_id, 0, &val_stream_id);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*stream)->stream_id = strdup(val_stream_id->data.string_val);

    // Request
    _create_xpath(xpath, "/request", &xpath_request);
    TSN_Request *req = malloc(sizeof(TSN_Request));
    rc = _read_request(xpath_request, &req);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*stream)->request = *req;

    // Configuration
    _create_xpath(xpath, "/configuration", &xpath_request);
    TSN_Configuration *con = malloc(sizeof(TSN_Configuration));
    rc = _read_configuration(xpath_configuration, &con);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*stream)->configuration = *con;

cleanup:
    sr_free_val(val_stream_id);
    free(xpath_stream_id);
    free(xpath_request);
    free(xpath_configuration);

    return rc;
}

static int
_read_module(char *xpath, TSN_Module **mod)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_id = NULL;
    sr_val_t *val_name = NULL;
    sr_val_t *val_description = NULL;
    sr_val_t *val_path = NULL;
    sr_val_t *val_subscribed_events_mask = NULL;
    char *xpath_id = NULL;
    char *xpath_name = NULL;
    char *xpath_description = NULL;
    char *xpath_path = NULL;
    char *xpath_subscribed_events_mask = NULL;

    // ID
    _create_xpath(xpath, "/id", &xpath_id);
    rc = sr_get_item(session, xpath_id, 0, &val_id);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*mod)->id = val_id->data.uint16_val;

    // Name
    _create_xpath(xpath, "/name", &xpath_name);
    rc = sr_get_item(session, xpath_name, 0, &val_name);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*mod)->name = strdup(val_name->data.string_val);

    // Description
    _create_xpath(xpath, "/desc", &xpath_description);
    rc = sr_get_item(session, xpath_description, 0, &val_description);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*mod)->description = strdup(val_description->data.string_val);

    // Path
    _create_xpath(xpath, "/executable-path", &xpath_path);
    rc = sr_get_item(session, xpath_path, 0, &val_path);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*mod)->path = strdup(val_path->data.string_val);

    // Subscribed Events Mask
    _create_xpath(xpath, "/subscribed-events-mask", &xpath_subscribed_events_mask);
    rc = sr_get_item(session, xpath_subscribed_events_mask, 0, &val_subscribed_events_mask);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*mod)->subscribed_events_mask = val_subscribed_events_mask->data.uint16_val;

cleanup:
    sr_free_val(val_id);
    sr_free_val(val_name);
    sr_free_val(val_description);
    sr_free_val(val_path);
    sr_free_val(val_subscribed_events_mask);
    free(xpath_id);
    free(xpath_name);
    free(xpath_description);
    free(xpath_path);
    free(xpath_subscribed_events_mask);

    return rc;
}

static int
_write_module(char *xpath, TSN_Module *mod)
{
    int rc = SR_ERR_OK;
    char *xpath_id = NULL;
    char *xpath_name = NULL;
    char *xpath_description = NULL;
    char *xpath_path = NULL;
    char *xpath_subscribed_events_mask = NULL;

    // Write ID
    _create_xpath(xpath, "/id", &xpath_id);
    sr_val_t val_id;
    val_id.type = SR_UINT16_T;
    val_id.data.uint16_val = mod->id;
    rc = sr_set_item(session, xpath_id, &val_id, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write Name
    _create_xpath(xpath, "/name", &xpath_name);
    rc = sr_set_item_str(session, xpath_name, mod->name, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write Description
    _create_xpath(xpath, "/desc", &xpath_description);
    rc = sr_set_item_str(session, xpath_description, mod->description, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write Path
    _create_xpath(xpath, "/executable-path", &xpath_path);
    rc = sr_set_item_str(session, xpath_path, mod->path, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write Subscribed Events Mask
    _create_xpath(xpath, "/subscribed-events-mask", &xpath_subscribed_events_mask);
    sr_val_t val_subscribed_events_mask;
    val_subscribed_events_mask.type = SR_UINT16_T;
    val_subscribed_events_mask.data.uint16_val = mod->subscribed_events_mask;
    rc = sr_set_item(session, xpath_subscribed_events_mask, &val_subscribed_events_mask, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_id);
    free(xpath_name);
    free(xpath_description);
    free(xpath_path);
    free(xpath_subscribed_events_mask);

    return rc;
}

static int
_read_modules(char *xpath, TSN_Modules **modules)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_all_modules = NULL;
    sr_val_t *val_registered_modules = NULL;
    char *xpath_all_modules = NULL;
    char *xpath_registered_modules = NULL;

    // Read all modules
    _create_xpath(xpath, "/available-modules/*", &xpath_all_modules);
    size_t count_all_modules = 0;
    rc = sr_get_items(session, xpath_all_modules, 0, 0, &val_all_modules, &count_all_modules);
    (*modules)->count_available_modules = count_all_modules;
    (*modules)->available_modules = (TSN_Module *) malloc(sizeof(TSN_Module) * count_all_modules);
    for (int i=0; i<count_all_modules; ++i) {
        TSN_Module *m = malloc(sizeof(TSN_Module));
        m->p_id = -1;
        rc = _read_module((&val_all_modules[i])->xpath, &m);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*modules)->available_modules[i] = *m;
    }

    // Read registered modules
    _create_xpath(xpath, "/registered-modules/*", &xpath_registered_modules);
    size_t count_registered_modules = 0;
    rc = sr_get_items(session, xpath_registered_modules, 0, 0, &val_registered_modules, &count_registered_modules);
    (*modules)->count_registered_modules = count_registered_modules;
    (*modules)->registered_modules = (TSN_Module *) malloc(sizeof(TSN_Module) * count_registered_modules);
    for (int i=0; i<count_registered_modules; ++i) {
        TSN_Module *m = malloc(sizeof(TSN_Module));
        rc = _read_module((&val_registered_modules[i])->xpath, &m);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*modules)->registered_modules[i] = *m;
    }

cleanup:
    sr_free_val(val_all_modules);
    sr_free_val(val_registered_modules);
    free(xpath_all_modules);
    free(xpath_registered_modules);

    return rc;
}

static int 
_read_root(char *xpath, TSN_Uni **root)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_stream_list = NULL;
    char *xpath_stream_list = NULL;
    char *xpath_modules = NULL;

    // Read streams
    _create_xpath(xpath, "/stream-list/*", &xpath_stream_list);
    size_t count_streams = 0;
    rc = sr_get_items(session, xpath_stream_list, 0, 0, &val_stream_list, &count_streams);
    (*root)->count_streams = count_streams;
    (*root)->stream_list = (TSN_Stream *) malloc(sizeof(TSN_Stream) * count_streams);
    for (int i=0; i<count_streams; ++i) {
        TSN_Stream *s = malloc(sizeof(TSN_Stream));
        rc = _read_stream((&val_stream_list[i])->xpath, &s);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*root)->stream_list[i] = *s;
    }

    // Read modules
    _create_xpath(xpath, "/modules", &xpath_modules);
    TSN_Modules *modules = malloc(sizeof(TSN_Modules));
    rc = _read_modules(xpath_modules, &modules);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*root)->modules = *modules;

cleanup:
    sr_free_val(val_stream_list);
    free(xpath_stream_list);
    free(xpath_modules);

    return rc;
}




// -------------------------------------------------------- //
//  CRUD methods
// -------------------------------------------------------- //
// The yang module root containing the stream-list
/*
int 
sysrepo_get_root(TSN_Uni **root)
{
    rc = _read_root("/control-tsn-uni:tsn-uni", &(*root));
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

// Stream
int 
sysrepo_write_stream(TSN_Stream stream)
{

}

int 
sysrepo_read_stream(char *stream_id, TSN_Stream **stream)
{

}

int 
sysrepo_delete_stream(char *stream_id)
{

}

// Modules
int 
sysrepo_register_module(TSN_Module module)
{

}

int 
sysrepo_add_available_module(TSN_Module module)
{

}

int 
sysrepo_get_modules(TSN_Modules **modules)
{
    rc = _read_modules("/control-tsn-uni:tsn-uni/modules", &(*modules));
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
*/



// -------------------------------------------------------- //
//  Module handling
// -------------------------------------------------------- //
int 
sysrepo_add_new_module(TSN_Module new_module)
{
    int is_failure = 0;
    char *xpath_mod = NULL;

    // Get all modules in the datastore
    TSN_Modules *stored_modules = NULL;
    stored_modules = malloc(sizeof(TSN_Modules));
    rc = _read_modules("/control-tsn-uni:tsn-uni/modules", &stored_modules);
    if (rc != SR_ERR_OK) {
        is_failure = 1;
        goto cleanup;
    }

    // Search if the name and path already exists under the stored modules
    // simultaneously the highest used ID is determined in order to specify
    // the ID for the potential, new module
    int highest_used_id = 0;
    for (int i=0; i<stored_modules->count_available_modules; ++i) {
        // Compare name and path
        if ((strcmp(new_module.name, stored_modules->available_modules[i].name) == 0) &&
            (strcmp(new_module.path, stored_modules->available_modules[i].path) == 0)) {
            
            // We found a module with the same name and path
            printf("[SYSREPO] Error adding a new module. Module with the same name and path already exists!\n");
            is_failure = 1;
            goto cleanup;
        }

        if (stored_modules->available_modules[i].id > highest_used_id) {
            highest_used_id = stored_modules->available_modules[i].id;
        }
    }

    // Otherwise we can add the new module
    new_module.id = (highest_used_id + 1);
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']", new_module.id, &xpath_mod);
    rc = _write_module(xpath_mod, &new_module);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Error adding a new module to the list!\n");
        is_failure = 1;
        goto cleanup;
    }

cleanup:
    free(xpath_mod);
    free(stored_modules);

    return is_failure ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_register_module(int module_id)
{
    int is_failure = 0;
    // Get the module from the datastore
    char *xpath_stored_module = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']", module_id, &xpath_stored_module);
    TSN_Module *stored_module = malloc(sizeof(TSN_Module));
    rc = _read_module(xpath_stored_module, &stored_module);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Error finding module with the ID %d in the list!\n", module_id);
        is_failure = 1;
        goto cleanup;
    }

    // Check if the module is already in the list of registered modules
    char *xpath_module_check = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/name", module_id, &xpath_module_check);
    sr_val_t *val_module_name_check = NULL;
    rc = sr_get_item(session, xpath_module_check, 0, &val_module_name_check);
    if (rc == SR_ERR_OK) {
        // A module exists already under the specified ID
        printf("[SYSREPO] Error registering the Module. Module already exists under the specified ID!\n");
        is_failure = 1;
        goto cleanup;
    }

    // Otherwise add the module to the list
    char *xpath_module_add = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']", module_id, &xpath_module_add);
    rc = _write_module(xpath_module_add, stored_module);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Error registering the Module!\n");
        is_failure = 1;
        goto cleanup;
    }

cleanup:
    free(xpath_stored_module);
    free(stored_module);
    free(xpath_module_check);
    sr_free_val(val_module_name_check);
    free(xpath_module_add);

    return is_failure ? EXIT_FAILURE : EXIT_SUCCESS;
}

int 
sysrepo_unregister_module(int module_id)
{
    // Delete the entry
    char *xpath_module_delete = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']", module_id, &xpath_module_delete);
    rc = sr_delete_item(session, xpath_module_delete, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module_delete);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int 
sysrepo_delete_module(int module_id)
{
    // Delete the entry
    char *xpath_module_delete = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']", module_id, &xpath_module_delete);
    rc = sr_delete_item(session, xpath_module_delete, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module_delete);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int 
sysrepo_get_module_from_registered(int module_id, TSN_Module **module)
{
    char *xpath_module = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']", module_id, &xpath_module);
    rc = _read_module(xpath_module, module);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int 
sysrepo_get_module_from_all(int module_id, TSN_Module **module)
{
    char *xpath_module = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']", module_id, &xpath_module);
    rc = _read_module(xpath_module, module);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int 
sysrepo_get_all_modules(TSN_Modules **modules)
{
    char *xpath = NULL;
    rc = _read_modules("/control-tsn-uni:tsn-uni/modules", modules);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
