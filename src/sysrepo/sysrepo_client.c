#include <string.h>
#include "sysrepo_client.h"

//  Sysrepo variables
int rc; // Result
sr_conn_ctx_t *connection = NULL;
sr_session_ctx_t *session = NULL;
sr_subscription_ctx_t *subscription = NULL;

//  Callbacks
void (*_cb_stream_requested)(TSN_Stream *);
void (*_cb_stream_configured)(TSN_Stream *);
void (*_cb_error)();


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

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

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


// -------------------------------------------------------- //
//  Static helper functions to interact with sysrepo
// -------------------------------------------------------- //
static void 
_create_xpath(char *xpath_base, char *xpath_append, char **result)
{
    (*result) = malloc(strlen(xpath_base) + strlen(xpath_append) + 1);
    sprintf((*result), "%s%s", xpath_base, xpath_append);
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
        uint32_t *cb_stream_iden = malloc(sizeof(uint32_t));
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
        uint32_t *cb_sequence_type = malloc(sizeof(uint32_t));
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
    rc = 
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
    sr_free_val(val_priority_code_point);
    sr_free_val(val_vlan_id);
    free(xpath_priority_code_point);
    free(xpath_vlan_id);

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
_read_status_stream(char *xpath, IEEE_StatusStream **ss)
{

}

static int
_read_status_talker(char *xpath, TSN_StatusTalker **st)
{

}

static int
_read_status_listener(char *xpath, TSN_StatusListener **sl)
{

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
    // TODO _read_talker

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
    // TODO _read_request

    // Configuration
    _create_xpath(xpath, "/configuration", &xpath_request);
    // TODO _read_configuration

cleanup:
    sr_free_val(val_stream_id);
    free(xpath_stream_id);
    free(xpath_request);
    free(xpath_configuration);

    return rc;
}

static int 
_read_root(char *xpath, TSN_Uni **root)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_stream_list;
    
    char *xpath_stream_list = NULL;

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

cleanup:
    sr_free_val(val_stream_list);
    free(xpath_stream_list);

    return rc;
}


// -------------------------------------------------------- //
//  CRUD methods
// -------------------------------------------------------- //
// The yang module root containing the stream-list
int 
sysrepo_get_root(TSN_Uni **root)
{
    rc = _read_root("/control-tsn-uni:tsn-uni", &(*root));

cleanup:
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
