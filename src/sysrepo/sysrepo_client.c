#include <string.h>
#include <math.h>
#include "sysrepo_client.h"

#include "../logger.h"


//  Sysrepo variables
int rc; // Result
sr_conn_ctx_t *connection = NULL;
sr_session_ctx_t *session = NULL;
sr_subscription_ctx_t *subscription = NULL;

// Subscribed events mask
uint32_t _subscribed_mask = 0;

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
    if (connection != NULL) {
        return EXIT_SUCCESS;
    }

    // Trigger the sysrepo plugin daemon
    // the plugin itself checks if an instance is already runnning to prevent starting multiple instances
    // and thus sending the same events multiple times
    system("sysrepo-plugind");

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

int
sysrepo_disconnect()
{
    rc = sysrepo_stop_listening();
    if (rc == EXIT_SUCCESS) {
        printf("[SYSREPO] Successfully stopped listening!\n");
    }

    rc = sr_disconnect(connection);
    if (rc == EXIT_SUCCESS) {
        printf("[SYSREPO] Successfully disconneted!\n");
        return EXIT_SUCCESS;
    }

    printf("[SYSREPO] Failure while disconnecting!\n");
    return EXIT_FAILURE;
}

// -------------------------------------------------------- //
//  Callbacks
// -------------------------------------------------------- //
void
sysrepo_init_callback(uint32_t subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data))
{
    _subscribed_mask = subscribed_events_mask;
    _cb_event = cb_event;
}

int
sysrepo_send_notification(uint32_t event_id, char *entry_id, char *msg)
{
    sr_val_t notif_values[3];
    notif_values[0].xpath = strdup("/control-tsn-uni:notif-generic/event-id");
    notif_values[0].type = SR_UINT32_T;
    notif_values[0].data.uint32_val = event_id;
    notif_values[1].xpath = strdup("/control-tsn-uni:notif-generic/entry-id");
    notif_values[1].type = SR_STRING_T;
    notif_values[1].data.string_val = entry_id;
    notif_values[2].xpath = strdup("/control-tsn-uni:notif-generic/msg");
    notif_values[2].type = SR_STRING_T;
    notif_values[2].data.string_val = msg;

    rc = sr_event_notif_send(session, "/control-tsn-uni:notif-generic", notif_values, 3);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Failure while sending notification for event id %d!\n", event_id);
    }

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
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
    // Abort if no callback function is set
    if (!_cb_event) {
        return;
    }

    uint32_t _event_id = values[0].data.uint32_val;

    // Check if the module subscribed to this event
    if ((_subscribed_mask & values[0].data.uint32_val) == 0) {
        return;
    }

    // Creating the event data
    TSN_Event_CB_Data event_data = {
        .event_id = _event_id,
        .entry_id = values[1].data.string_val,
        .msg = values[2].data.string_val
    };

    // Calling the callback function
    _cb_event(event_data);
}

int
sysrepo_start_listening()
{
    // Subscribe to notifications
    rc = sr_event_notif_subscribe(session, "control-tsn-uni", NULL, 0, 0, _notif_listener_cb, NULL, 0, &subscription);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Failure while subscribing to notifications!\n");
    }

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_stop_listening()
{
    if (subscription == NULL) {
        printf("[SYSREPO] No subscriptions to unsubscribe!\n");
        return EXIT_FAILURE;
    }

    // Unsubscribe from notifications
    rc = sr_unsubscribe(subscription);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Failure while unsubscribing from notifications!\n");
    }

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
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

void
_create_xpath_id(char *xpath_base, int id, char **result)
{
    size_t size_needed = snprintf(NULL, 0, xpath_base, id) + 1;
    (*result) = malloc(size_needed);
    sprintf((*result), xpath_base, id);
}

void
_create_xpath_key(char *xpath_base, char *key, char **result)
{
    size_t size_needed = snprintf(NULL, 0, xpath_base, key) + 1;
    (*result) = malloc(size_needed);
    sprintf((*result), xpath_base, key);
}

void
_create_xpath_key_multi(char *xpath_base, char *key1, char *key2, char **result)
{
    size_t size_needed = snprintf(NULL, 0, xpath_base, key1, key2) + 1;
    (*result) = malloc(size_needed);
    sprintf((*result), xpath_base, key1, key2);
}


// -------------------------------------------------------- //
//  Stream ID generation method
// -------------------------------------------------------- //
static char *
_generate_stream_id(TSN_Request stream_request)
{
    char *stream_id = NULL;
    char *talker_mac = strdup(stream_request.talker.end_station_interfaces[0].mac_address);

    // Get a list of all streams
    TSN_Streams *all_streams = malloc(sizeof(TSN_Streams));
    rc = sysrepo_get_all_streams(&all_streams, 0);
    if (rc != EXIT_SUCCESS) {
        goto cleanup;
    }

    uint16_t count_talker_streams = 0; // The number of streams with the same talker mac
    for (int i=0; i<all_streams->count_streams; ++i) {
        if (strstr(all_streams->streams[i].stream_id, talker_mac) != NULL) {
            count_talker_streams += 1;
        }
    }
    char talker_number[5];
    count_talker_streams += 1; // Add one for the new stream
    sprintf(talker_number, "%04x", count_talker_streams);
    talker_number[4] = '\0';

    // Merge talker mac and the talker number for the new stream id
    size_t size_needed = snprintf(NULL, 0, "XX-XX-XX-XX-XX-XX:YY-YY") + 1;
    stream_id = malloc(size_needed);
    sprintf(stream_id, "%s:%c%c-%c%c", talker_mac, talker_number[0], talker_number[1], talker_number[2], talker_number[3]);

cleanup:
    free(all_streams);

    return stream_id;
}


// -------------------------------
// Streams
// -------------------------------
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
_write_interface_id(char *xpath, IEEE_InterfaceId *interface_id)
{
    int rc =SR_ERR_OK;
    char *xpath_mac_address = NULL;
    char *xpath_interface_name = NULL;

    _create_xpath(xpath, "/mac-address", &xpath_mac_address);
    rc = sr_set_item_str(session, xpath_mac_address, interface_id->mac_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/interface-name", &xpath_interface_name);
    rc = sr_set_item_str(session, xpath_interface_name, interface_id->interface_name, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_ieee802_mac_addresses(char *xpath, IEEE_MacAddresses *ma)
{
    int rc =SR_ERR_OK;
    char *xpath_destination_mac_address = NULL;
    char *xpath_source_mac_address = NULL;

    _create_xpath(xpath, "/destination-mac-address", &xpath_destination_mac_address);
    rc = sr_set_item_str(session, xpath_destination_mac_address, ma->destination_mac_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/source-mac-address", &xpath_source_mac_address);
    rc = sr_set_item_str(session, xpath_source_mac_address, ma->source_mac_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_ieee802_vlan_tag(char *xpath, IEEE_VlanTag *vt)
{
    int rc =SR_ERR_OK;
    char *xpath_priority_code_point = NULL;
    char *xpath_vlan_id = NULL;

    _create_xpath(xpath, "/priority-code-point", &xpath_priority_code_point);
    sr_val_t val_priority_code_point;
    val_priority_code_point.type = SR_UINT8_T;
    val_priority_code_point.data.uint8_val = vt->priority_code_point;
    rc = sr_set_item(session, xpath_priority_code_point, &val_priority_code_point, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/vlan-id", &xpath_vlan_id);
    sr_val_t val_vlan_id;
    val_vlan_id.type = SR_UINT16_T;
    val_vlan_id.data.uint16_val = vt->vlan_id;
    rc = sr_set_item(session, xpath_vlan_id, &val_vlan_id, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_ipv4_tuple(char *xpath, IEEE_IPv4Tuple *ipv4)
{
    int rc =SR_ERR_OK;
    char *xpath_source_ip_address = NULL;
    char *xpath_destination_ip_address = NULL;
    char *xpath_dscp = NULL;
    char *xpath_protocol = NULL;
    char *xpath_source_port = NULL;
    char *xpath_destination_port = NULL;

    _create_xpath(xpath, "/source-ip-address", &xpath_source_ip_address);
    rc = sr_set_item_str(session, xpath_source_ip_address, ipv4->source_ip_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/destination-ip-address", &xpath_destination_ip_address);
    rc = sr_set_item_str(session, xpath_destination_ip_address, ipv4->destination_ip_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/dscp", &xpath_dscp);
    sr_val_t val_dscp;
    val_dscp.type = SR_UINT8_T;
    val_dscp.data.uint8_val = ipv4->dscp;
    rc = sr_set_item(session, xpath_dscp, &val_dscp, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/protocol", &xpath_protocol);
    sr_val_t val_protocol;
    val_protocol.type = SR_UINT16_T;
    val_protocol.data.uint16_val = ipv4->protocol;
    rc = sr_set_item(session, xpath_protocol, &val_protocol, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/source-port", &xpath_source_port);
    sr_val_t val_source_port;
    val_source_port.type = SR_UINT16_T;
    val_source_port.data.uint16_val = ipv4->source_port;
    rc = sr_set_item(session, xpath_source_port, &val_source_port, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/destination-port", &xpath_destination_port);
    sr_val_t val_destination_port;
    val_destination_port.type = SR_UINT16_T;
    val_destination_port.data.uint16_val = ipv4->destination_port;
    rc = sr_set_item(session, xpath_destination_port, &val_destination_port, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_ipv6_tuple(char *xpath, IEEE_IPv6Tuple *ipv6)
{
    int rc =SR_ERR_OK;
    char *xpath_source_ip_address = NULL;
    char *xpath_destination_ip_address = NULL;
    char *xpath_dscp = NULL;
    char *xpath_protocol = NULL;
    char *xpath_source_port = NULL;
    char *xpath_destination_port = NULL;

    _create_xpath(xpath, "/source-ip-address", &xpath_source_ip_address);
    rc = sr_set_item_str(session, xpath_source_ip_address, ipv6->source_ip_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/destination-ip-address", &xpath_destination_ip_address);
    rc = sr_set_item_str(session, xpath_destination_ip_address, ipv6->destination_ip_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/dscp", &xpath_dscp);
    sr_val_t val_dscp;
    val_dscp.type = SR_UINT8_T;
    val_dscp.data.uint8_val = ipv6->dscp;
    rc = sr_set_item(session, xpath_dscp, &val_dscp, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/protocol", &xpath_protocol);
    sr_val_t val_protocol;
    val_protocol.type = SR_UINT16_T;
    val_protocol.data.uint16_val = ipv6->protocol;
    rc = sr_set_item(session, xpath_protocol, &val_protocol, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/source-port", &xpath_source_port);
    sr_val_t val_source_port;
    val_source_port.type = SR_UINT16_T;
    val_source_port.data.uint16_val = ipv6->source_port;
    rc = sr_set_item(session, xpath_source_port, &val_source_port, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/destination-port", &xpath_destination_port);
    sr_val_t val_destination_port;
    val_destination_port.type = SR_UINT16_T;
    val_destination_port.data.uint16_val = ipv6->destination_port;
    rc = sr_set_item(session, xpath_destination_port, &val_destination_port, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_user_to_network_requirements(char *xpath, IEEE_UserToNetworkRequirements *utnr)
{
    int rc =SR_ERR_OK;
    char *xpath_num_seamless_trees = NULL;
    char *xpath_max_latency = NULL;

    _create_xpath(xpath, "/num-seamless-trees", &xpath_num_seamless_trees);
    sr_val_t val_num_seamless_trees;
    val_num_seamless_trees.type = SR_UINT8_T;
    val_num_seamless_trees.data.uint8_val = utnr->num_seamless_trees;
    rc = sr_set_item(session, xpath_num_seamless_trees, &val_num_seamless_trees, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/max-latency", &xpath_max_latency);
    sr_val_t val_max_latency;
    val_max_latency.type = SR_UINT32_T;
    val_max_latency.data.uint32_val = utnr->max_latency;
    rc = sr_set_item(session, xpath_max_latency, &val_max_latency, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_interface_capabilities(char *xpath, IEEE_InterfaceCapabilities *ic)
{
    int rc =SR_ERR_OK;
    char *xpath_vlan_tag_capable = NULL;
    char *xpath_cb_stream_iden_type_list = NULL;
    char *xpath_cb_sequence_type_list = NULL;

    _create_xpath(xpath, "/vlan-tag-capable", &xpath_vlan_tag_capable);
    sr_val_t val_vlan_tag_capable;
    val_vlan_tag_capable.type = SR_BOOL_T;
    val_vlan_tag_capable.data.bool_val = ic->vlan_tag_capable;
    rc = sr_set_item(session, xpath_vlan_tag_capable, &val_vlan_tag_capable, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/cb-stream-iden-type-list[.='%d']", &xpath_cb_stream_iden_type_list);
    for (int i=0; i<ic->count_cb_stream_iden_types; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_id(xpath_cb_stream_iden_type_list, ic->cb_stream_iden_type_list[i], &xpath_entry);
        sr_val_t val_entry;
        val_entry.type = SR_UINT32_T;
        val_entry.data.uint32_val = ic->cb_stream_iden_type_list[i];
        rc = sr_set_item(session, xpath_entry, &val_entry, 0);
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    _create_xpath(xpath, "/cb-sequence-type-list[.='%d']", &xpath_cb_sequence_type_list);
    for (int i=0; i<ic->count_cb_sequence_types; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_id(xpath_cb_sequence_type_list, ic->cb_sequence_type_list[i], &xpath_entry);
        sr_val_t val_entry;
        val_entry.type = SR_UINT32_T;
        val_entry.data.uint32_val = ic->cb_sequence_type_list[i];
        rc = sr_set_item(session, xpath_entry, &val_entry, 0);
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

cleanup:
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
_write_stream_rank(char *xpath, IEEE_StreamRank *sr)
{
    int rc =SR_ERR_OK;
    char *xpath_rank = NULL;

    _create_xpath(xpath, "/rank", &xpath_rank);
    sr_val_t val_rank;
    val_rank.type = SR_UINT8_T;
    val_rank.data.uint8_val = sr->rank;
    rc = sr_set_item(session, xpath_rank, &val_rank, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_rank);

    return rc;
}

static int
_read_data_frame_specification(char *xpath, IEEE_DataFrameSpecification **dfs)
{
    int rc;
    sr_val_t *val_index = NULL;
    sr_val_t *val_root = NULL;
    char *xpath_index = NULL;
    char *xpath_root = NULL;
    char *xpath_choice = NULL;

    // Read Index
    _create_xpath(xpath, "/index", &xpath_index);
    rc = sr_get_item(session, xpath_index, 0, &val_index);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*dfs)->index = val_index->data.uint8_val;

    // Read the Choice field
    _create_xpath(xpath, "/*", &xpath_root);
    size_t count_root = 0;
    rc = sr_get_items(session, xpath_root, 0, 0, &val_root, &count_root);
    DataFrameSpecification_FieldType type = 0;

    for (int i=0; i<count_root; ++i) {
        if (strstr(val_root[i].xpath, "/ieee802-mac-addresses") != NULL) {
            type = DATA_FRAME_SPECIFICATION_MAC_ADDRESSES;
        } else if (strstr(val_root[i].xpath, "/ieee802-vlan-tag") != NULL) {
            type = DATA_FRAME_SPECIFICATION_VLAN_TAG;
        } else if (strstr(val_root[i].xpath, "/ipv4-tuple") != NULL) {
            type = DATA_FRAME_SPECIFICATION_IPV4_TUPLE;
        } else if (strstr(val_root[i].xpath, "/ipv4-tuple") != NULL) {
            type = DATA_FRAME_SPECIFICATION_IPV6_TUPLE;
        }
    }

    (*dfs)->field_type = type;

    if (type == DATA_FRAME_SPECIFICATION_MAC_ADDRESSES) {
        // Read Mac Addresses
        _create_xpath(xpath, "/ieee802-mac-addresses", &xpath_choice);
        IEEE_MacAddresses *ma = malloc(sizeof(IEEE_MacAddresses));
        rc = _read_ieee802_mac_addresses(xpath_choice, &ma);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*dfs)->ieee802_mac_addresses = ma;

    } else if (type == DATA_FRAME_SPECIFICATION_VLAN_TAG) {
        // Read VLAN Tag
        _create_xpath(xpath, "/ieee802-vlan-tag", &xpath_choice);
        IEEE_VlanTag *vt = malloc(sizeof(IEEE_VlanTag));
        rc = _read_ieee802_vlan_tag(xpath_choice, &vt);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*dfs)->ieee802_vlan_tag = vt;

    }
     else if (type == DATA_FRAME_SPECIFICATION_IPV4_TUPLE) {
        // Read IPv4 Tuple
        _create_xpath(xpath, "/ipv4-tuple", &xpath_choice);
        IEEE_IPv4Tuple *ipv4 = malloc(sizeof(IEEE_IPv4Tuple));
        rc = _read_ipv4_tuple(xpath_choice, &ipv4);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*dfs)->ipv4_tuple = ipv4;

    } else if (type == DATA_FRAME_SPECIFICATION_IPV6_TUPLE) {
        // Read IPv6 Tuple
        _create_xpath(xpath, "/ipv6-tuple", &xpath_choice);
        IEEE_IPv6Tuple *ipv6 = malloc(sizeof(IEEE_IPv6Tuple));
        rc = _read_ipv6_tuple(xpath_choice, &ipv6);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*dfs)->ipv6_tuple = ipv6;
    }


cleanup:
    sr_free_val(val_index);
    sr_free_val(val_root);
    free(xpath_index);
    free(xpath_root);
    free(xpath_choice);

    return rc;
}

static int
_write_data_frame_specification(char *xpath, IEEE_DataFrameSpecification *dfs)
{
    int rc =SR_ERR_OK;
    char *xpath_index = NULL;
    char *xpath_choice = NULL;

    _create_xpath(xpath, "/index", &xpath_index);
    sr_val_t val_index;
    val_index.type = SR_UINT8_T;
    val_index.data.uint8_val = dfs->index;
    rc = sr_set_item(session, xpath_index, &val_index, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    if (dfs->field_type == DATA_FRAME_SPECIFICATION_MAC_ADDRESSES) {
        _create_xpath(xpath, "/ieee802-mac-addresses", &xpath_choice);
        rc = _write_ieee802_mac_addresses(xpath_choice, dfs->ieee802_mac_addresses);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    } else if (dfs->field_type == DATA_FRAME_SPECIFICATION_VLAN_TAG) {
        _create_xpath(xpath, "/ieee802-vlan-tag", &xpath_choice);
        rc = _write_ieee802_vlan_tag(xpath_choice, dfs->ieee802_vlan_tag);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    } else if (dfs->field_type == DATA_FRAME_SPECIFICATION_IPV4_TUPLE) {
        _create_xpath(xpath, "/ipv4-tuple", &xpath_choice);
        rc = _write_ipv4_tuple(xpath_choice, dfs->ipv4_tuple);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    } else if (dfs->field_type == DATA_FRAME_SPECIFICATION_IPV6_TUPLE) {
        _create_xpath(xpath, "/ipv6-tuple", &xpath_choice);
        rc = _write_ipv6_tuple(xpath_choice, dfs->ipv6_tuple);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    }

cleanup:
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
_write_interval(char *xpath, IEEE_Interval *in)
{
    int rc = SR_ERR_OK;
    char *xpath_numerator = NULL;
    char *xpath_denominator = NULL;

    _create_xpath(xpath, "/numerator", &xpath_numerator);
    sr_val_t val_numerator;
    val_numerator.type = SR_UINT32_T;
    val_numerator.data.uint32_val = in->numerator;
    rc = sr_set_item(session, xpath_numerator, &val_numerator, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/denominator", &xpath_denominator);
    sr_val_t val_denominator;
    val_denominator.type = SR_UINT32_T;
    val_denominator.data.uint32_val = in->denominator;
    rc = sr_set_item(session, xpath_denominator, &val_denominator, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_time_aware(char *xpath, IEEE_TimeAware *ta)
{
    int rc = SR_ERR_OK;
    char *xpath_earliest_transmit_offset = NULL;
    char *xpath_latest_transmit_offset = NULL;
    char *xpath_jitter = NULL;

    _create_xpath(xpath, "/earliest-transmit-offset", &xpath_earliest_transmit_offset);
    sr_val_t val_earliest_transmit_offset;
    val_earliest_transmit_offset.type = SR_UINT32_T;
    val_earliest_transmit_offset.data.uint32_val = ta->earliest_transmit_offset;
    rc = sr_set_item(session, xpath_earliest_transmit_offset, &val_earliest_transmit_offset, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/latest-transmit-offset", &xpath_latest_transmit_offset);
    sr_val_t val_latest_transmit_offset;
    val_latest_transmit_offset.type = SR_UINT32_T;
    val_latest_transmit_offset.data.uint32_val = ta->latest_transmit_offset;
    rc = sr_set_item(session, xpath_latest_transmit_offset, &val_latest_transmit_offset, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/jitter", &xpath_jitter);
    sr_val_t val_jitter;
    val_jitter.type = SR_UINT32_T;
    val_jitter.data.uint32_val = ta->jitter;
    rc = sr_set_item(session, xpath_jitter, &val_jitter, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_traffic_specification(char *xpath, IEEE_TrafficSpecification *ts)
{
    int rc = SR_ERR_OK;
    char *xpath_interval = NULL;
    char *xpath_max_frames_per_interval = NULL;
    char *xpath_max_frame_size = NULL;
    char *xpath_transmission_selection = NULL;
    char *xpath_time_aware = NULL;

    _create_xpath(xpath, "/interval", &xpath_interval);
    rc = _write_interval(xpath_interval, &ts->interval);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/max-frames-per-interval", &xpath_max_frames_per_interval);
    sr_val_t val_max_frames_per_interval;
    val_max_frames_per_interval.type = SR_UINT16_T;
    val_max_frames_per_interval.data.uint16_val = ts->max_frames_per_interval;
    rc = sr_set_item(session, xpath_max_frames_per_interval, &val_max_frames_per_interval, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/max-frame-size", &xpath_max_frame_size);
    sr_val_t val_max_frame_size;
    val_max_frame_size.type = SR_UINT16_T;
    val_max_frame_size.data.uint16_val = ts->max_frame_size;
    rc = sr_set_item(session, xpath_max_frame_size, &val_max_frame_size, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/transmission-selection", &xpath_transmission_selection);
    sr_val_t val_transmission_selection;
    val_transmission_selection.type = SR_UINT8_T;
    val_transmission_selection.data.uint8_val = ts->transmission_selection;
    rc = sr_set_item(session, xpath_transmission_selection, &val_transmission_selection, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/time-aware", &xpath_time_aware);
    rc = _write_time_aware(xpath_time_aware, &ts->time_aware);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
_write_talker(char *xpath, TSN_Talker *talker)
{
    int rc = SR_ERR_OK;
    char *xpath_stream_rank = NULL;
    char *xpath_end_station_interfaces = NULL;
    char *xpath_data_frame_specification = NULL;
    char *xpath_traffic_specification = NULL;
    char *xpath_user_to_network_requirements = NULL;
    char *xpath_interface_capabilities = NULL;

    _create_xpath(xpath, "/stream-rank", &xpath_stream_rank);
    rc = _write_stream_rank(xpath_stream_rank, &talker->stream_rank);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/end-station-interfaces[mac-address='%s'][interface-name='%s']", &xpath_end_station_interfaces);
    for (int i=0; i<talker->count_end_station_interfaces; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_key_multi(xpath_end_station_interfaces, talker->end_station_interfaces[i].mac_address, talker->end_station_interfaces[i].interface_name, &xpath_entry);
        rc = _write_interface_id(xpath_entry, &(talker->end_station_interfaces[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    _create_xpath(xpath, "/data-frame-specification[index='%d']", &xpath_data_frame_specification);
    for (int i=0; i<talker->count_data_frame_specifications; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_id(xpath_data_frame_specification, talker->data_frame_specification[i].index, &xpath_entry);
        rc = _write_data_frame_specification(xpath_entry, &(talker->data_frame_specification[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    _create_xpath(xpath, "/traffic-specification", &xpath_traffic_specification);
    rc = _write_traffic_specification(xpath_traffic_specification, &(talker->traffic_specification));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/user-to-network-requirements", &xpath_user_to_network_requirements);
    rc = _write_user_to_network_requirements(xpath_user_to_network_requirements, &(talker->user_to_network_requirements));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/interface-capabilities", &xpath_interface_capabilities);
    rc = _write_interface_capabilities(xpath_interface_capabilities, &(talker->interface_capabilities));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_stream_rank);
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
    sr_val_t *val_index = NULL;
    sr_val_t *val_end_station_interfaces = NULL;
    char *xpath_index = NULL;
    char *xpath_end_station_interfaces = NULL;
    char *xpath_user_to_network_requirements = NULL;
    char *xpath_interface_capabilities = NULL;

    _create_xpath(xpath, "/index", &xpath_index);
    rc = sr_get_item(session, xpath_index, 0, &val_index);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*listener)->index = val_index->data.uint16_val;

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
    sr_free_val(val_index);
    sr_free_val(val_end_station_interfaces);
    free(xpath_index);
    free(xpath_end_station_interfaces);
    free(xpath_user_to_network_requirements);
    free(xpath_interface_capabilities);

    return rc;
}

static int
_write_listener(char *xpath, TSN_Listener *listener)
{
    int rc = SR_ERR_OK;
    char *xpath_index = NULL;
    char *xpath_end_station_interfaces = NULL;
    char *xpath_user_to_network_requirements = NULL;
    char *xpath_interface_capabilities = NULL;

    _create_xpath(xpath, "/index", &xpath_index);
    sr_val_t val_index;
    val_index.type = SR_UINT16_T;
    val_index.data.uint16_val = listener->index;
    rc = sr_set_item(session, xpath_index, &val_index, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/end-station-interfaces[mac-address='%s'][interface-name='%s']", &xpath_end_station_interfaces);
    for (int i=0; i<listener->count_end_station_interfaces; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_key_multi(xpath_end_station_interfaces, listener->end_station_interfaces[i].mac_address, listener->end_station_interfaces[i].interface_name, &xpath_entry);
        rc = _write_interface_id(xpath_entry, &(listener->end_station_interfaces[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    _create_xpath(xpath, "/user-to-network-requirements", &xpath_user_to_network_requirements);
    rc = _write_user_to_network_requirements(xpath_user_to_network_requirements, &(listener->user_to_network_requirements));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/interface-capabilities", &xpath_interface_capabilities);
    rc = _write_interface_capabilities(xpath_interface_capabilities, &(listener->interface_capabilities));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_index);
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
_write_status_info(char *xpath, IEEE_StatusInfo *si)
{
    int rc = SR_ERR_OK;
    char *xpath_talker_status = NULL;
    char *xpath_listener_status = NULL;
    char *xpath_failure_code = NULL;

    _create_xpath(xpath, "/talker-status", &xpath_talker_status);
    sr_val_t val_talker_status;
    val_talker_status.type = SR_ENUM_T;
    val_talker_status.data.enum_val = strdup(si->talker_status);
    rc = sr_set_item(session, xpath_talker_status, &val_talker_status, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/listener-status", &xpath_listener_status);
    sr_val_t val_listener_status;
    val_listener_status.type = SR_ENUM_T;
    val_listener_status.data.enum_val = strdup(si->listener_status);
    rc = sr_set_item(session, xpath_listener_status, &val_listener_status, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/failure-code", &xpath_failure_code);
    sr_val_t val_failure_code;
    val_failure_code.type = SR_UINT8_T;
    val_failure_code.data.uint8_val = si->failure_code;
    rc = sr_set_item(session, xpath_failure_code, &val_failure_code, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_talker_status);
    free(xpath_listener_status);
    free(xpath_failure_code);

    return rc;
}

/*
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
*/

/*
static int
_write_status_stream(char *xpath, IEEE_StatusStream *ss)
{
    int rc = SR_ERR_OK;
    char *xpath_status_info = NULL;
    char *xpath_failed_interfaces = NULL;

    _create_xpath(xpath, "/status-info", &xpath_status_info);
    rc = _write_status_info(xpath_status_info, &(ss->status_info));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/failed-interfaces[mac-address='%s'][interface-name='%s']", &xpath_failed_interfaces);
    for (int i=0; i<ss->count_failed_interfaces; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_key_multi(xpath_failed_interfaces, ss->failed_interfaces[i].mac_address, ss->failed_interfaces[i].interface_name, &xpath_entry);
        rc = _write_interface_id(xpath_entry, &(ss->failed_interfaces[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }


cleanup:
    free(xpath_status_info);
    free(xpath_failed_interfaces);

    return rc;
}
*/

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
_write_request(char *xpath, TSN_Request *req)
{
    int rc = SR_ERR_OK;
    char *xpath_talker = NULL;
    char *xpath_listener_list = NULL;

    _create_xpath(xpath, "/talker", &xpath_talker);
    rc = _write_talker(xpath_talker, &(req->talker));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/listener-list/listener[index='%d']", &xpath_listener_list);
    for (int i=0; i<req->count_listeners; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_id(xpath_listener_list, req->listener_list[i].index, &xpath_entry);
        rc = _write_listener(xpath_entry, &(req->listener_list[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }


cleanup:
    free(xpath_talker);
    free(xpath_listener_list);

    return rc;
}

static int
_read_config_list(char *xpath, IEEE_ConfigList **cl)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_index = NULL;
    sr_val_t *val_root = NULL;
    sr_val_t *val_time_aware_offset = NULL;
    char *xpath_index = NULL;
    char *xpath_root = NULL;
    char *xpath_choice = NULL;

    // Read Index
    _create_xpath(xpath, "/index", &xpath_index);
    rc = sr_get_item(session, xpath_index, 0, &val_index);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*cl)->index = val_index->data.uint8_val;


    // Read the Choice field
    _create_xpath(xpath, "/*", &xpath_root);
    size_t count_root = 0;
    rc = sr_get_items(session, xpath_root, 0, 0, &val_root, &count_root);
    ConfigList_FieldType type = 0;

    for (int i=0; i<count_root; ++i) {
        if (strstr(val_root[i].xpath, "/ieee802-mac-addresses") != NULL) {
            type = CONFIG_LIST_MAC_ADDRESSES;
        } else if (strstr(val_root[i].xpath, "/ieee802-vlan-tag") != NULL) {
            type = CONFIG_LIST_VLAN_TAG;
        } else if (strstr(val_root[i].xpath, "/ipv4-tuple") != NULL) {
            type = CONFIG_LIST_IPV4_TUPLE;
        } else if (strstr(val_root[i].xpath, "/ipv4-tuple") != NULL) {
            type = CONFIG_LIST_IPV6_TUPLE;
        } else if (strstr(val_root[i].xpath, "/time-aware-offset") != NULL) {
            type = CONFIG_LIST_TIME_AWARE_OFFSET;
        }
    }

    (*cl)->field_type = type;

    // Read the choice field
    if (type == CONFIG_LIST_MAC_ADDRESSES) {
        // Read MAC Addresses
        _create_xpath(xpath, "/ieee802-mac-addresses", &xpath_choice);
        IEEE_MacAddresses *ma = malloc(sizeof(IEEE_MacAddresses));
        rc = _read_ieee802_mac_addresses(xpath_choice, &ma);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*cl)->ieee802_mac_addresses = ma;

    } else if (type == CONFIG_LIST_VLAN_TAG) {
        // Read VLAN Tag
        _create_xpath(xpath, "/ieee802-vlan-tag", &xpath_choice);
        IEEE_VlanTag *vt = malloc(sizeof(IEEE_VlanTag));
        rc = _read_ieee802_vlan_tag(xpath_choice, &vt);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*cl)->ieee802_vlan_tag = vt;

    } else if (type == CONFIG_LIST_IPV4_TUPLE) {
        // Read IPv4 Tuple
        _create_xpath(xpath, "/ipv4-tuple", &xpath_choice);
        IEEE_IPv4Tuple *ipv4 = malloc(sizeof(IEEE_IPv4Tuple));
        rc = _read_ipv4_tuple(xpath_choice, &ipv4);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*cl)->ipv4_tuple = ipv4;

    } else if (type == CONFIG_LIST_IPV6_TUPLE) {
        // Check and Read IPv6 Tuple
        _create_xpath(xpath, "/ipv6-tuple", &xpath_choice);
        IEEE_IPv6Tuple *ipv6 = malloc(sizeof(IEEE_IPv6Tuple));
        rc = _read_ipv6_tuple(xpath_choice, &ipv6);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*cl)->ipv6_tuple = ipv6;

    } else if (type == CONFIG_LIST_TIME_AWARE_OFFSET) {
        // Check and read Time aware offset
        _create_xpath(xpath, "/time-aware-offset", &xpath_choice);
        rc = sr_get_item(session, xpath_choice, 0, &val_time_aware_offset);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*cl)->time_aware_offset = val_time_aware_offset->data.uint32_val;
    }


cleanup:
    sr_free_val(val_index);
    sr_free_val(val_root);
    free(xpath_index);
    free(xpath_root);
    free(xpath_choice);

    return rc;
}

static int
_write_config_list(char *xpath, IEEE_ConfigList *cl)
{
    int rc = SR_ERR_OK;
    char *xpath_index = NULL;
    char *xpath_choice = NULL;

    _create_xpath(xpath, "/index", &xpath_index);
    sr_val_t val_index;
    val_index.type = SR_UINT8_T;
    val_index.data.uint8_val = cl->index;
    rc = sr_set_item(session, xpath_index, &val_index, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    if (cl->field_type == CONFIG_LIST_MAC_ADDRESSES) {
        _create_xpath(xpath, "/ieee802-mac-addresses", &xpath_choice);
        rc = _write_ieee802_mac_addresses(xpath_choice, cl->ieee802_mac_addresses);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    } else if (cl->field_type == CONFIG_LIST_VLAN_TAG) {
        _create_xpath(xpath, "/ieee802-vlan-tag", &xpath_choice);
        rc = _write_ieee802_vlan_tag(xpath_choice, cl->ieee802_vlan_tag);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    } else if (cl->field_type == CONFIG_LIST_IPV4_TUPLE) {
        _create_xpath(xpath, "/ipv4-tuple", &xpath_choice);
        rc = _write_ipv4_tuple(xpath_choice, cl->ipv4_tuple);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    } else if (cl->field_type == CONFIG_LIST_IPV6_TUPLE) {
        _create_xpath(xpath, "/ipv6-tuple", &xpath_choice);
        rc = _write_ipv6_tuple(xpath_choice, cl->ipv6_tuple);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }

    } else if (cl->field_type == CONFIG_LIST_TIME_AWARE_OFFSET) {
        _create_xpath(xpath, "/time-aware-offset", &xpath_choice);
        sr_val_t val_time_aware_offset;
        val_time_aware_offset.type = SR_UINT32_T;
        val_time_aware_offset.data.uint32_val = cl->time_aware_offset;
        rc = sr_set_item(session, xpath_choice, &val_time_aware_offset, 0);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

cleanup:
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
    //(*il)->interface_id = *ii;
    (*il)->interface_name = strdup(ii->interface_name);
    (*il)->mac_address = strdup(ii->mac_address);

    // Config list
    _create_xpath(xpath, "/config-list", &xpath_config_list);
    size_t count_configs = 0;
    rc = sr_get_items(session, xpath_config_list, 0, 0, &val_config_list, &count_configs);
    (*il)->count_config_list_entries = count_configs;
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
_write_interface_list(char *xpath, IEEE_InterfaceList *il)
{
    int rc = SR_ERR_OK;
    //char *xpath_interface_id = NULL;
    char *xpath_mac_address = NULL;
    char *xpath_interface_name = NULL;
    char *xpath_config_list = NULL;

    /*
    _create_xpath(xpath, "", &xpath_interface_id);
    rc = _write_interface_id(xpath_interface_id, &(il->interface_id));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    */
    _create_xpath(xpath, "/mac-address", &xpath_mac_address);
    rc = sr_set_item_str(session, xpath_mac_address, il->mac_address, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/interface-name", &xpath_interface_name);
    rc = sr_set_item_str(session, xpath_interface_name, il->interface_name, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/config-list[index='%d']", &xpath_config_list);
    for (int i=0; i<il->count_config_list_entries; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_id(xpath_config_list, il->config_list[i].index, &xpath_entry);
        rc = _write_config_list(xpath_entry, &(il->config_list[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }


cleanup:
    //free(xpath_interface_id);
    free(xpath_mac_address);
    free(xpath_interface_name);
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
    (*ic)->count_interface_list_entries = count_interfaces;
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
_write_interface_configuration(char *xpath, IEEE_InterfaceConfiguration *ic)
{
    int rc = SR_ERR_OK;
    char *xpath_interface_list = NULL;

    _create_xpath(xpath, "/interface-list[mac-address='%s'][interface-name='%s']", &xpath_interface_list);
    for (int i=0; i<ic->count_interface_list_entries; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_key_multi(xpath_interface_list, ic->interface_list[i].mac_address, ic->interface_list[i].interface_name, &xpath_entry);
        rc = _write_interface_list(xpath_entry, &(ic->interface_list[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

cleanup:
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
    _create_xpath(xpath, "/interface-configuration", &xpath_interface_configuration);
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
_write_status_talker(char *xpath, TSN_StatusTalker *st)
{
    int rc = SR_ERR_OK;
    char *xpath_accumulated_latency = NULL;
    char *xpath_interface_configuration = NULL;

    _create_xpath(xpath, "/accumulated-latency", &xpath_accumulated_latency);
    sr_val_t val_accumulated_latency;
    val_accumulated_latency.type = SR_UINT32_T;
    val_accumulated_latency.data.uint32_val = st->accumulated_latency;
    rc = sr_set_item(session, xpath_accumulated_latency, &val_accumulated_latency, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/interface-configuration", &xpath_interface_configuration);
    rc = _write_interface_configuration(xpath_interface_configuration, &(st->interface_configuration));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
    _create_xpath(xpath, "/interface-configuration", &xpath_interface_configuration);
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
_write_status_listener(char *xpath, TSN_StatusListener *sl)
{
    int rc = SR_ERR_OK;
    char *xpath_index = NULL;
    char *xpath_accumulated_latency = NULL;
    char *xpath_interface_configuration = NULL;

    _create_xpath(xpath, "/index", &xpath_index);
    sr_val_t val_index;
    val_index.type = SR_UINT16_T;
    val_index.data.uint16_val = sl->index;
    rc = sr_set_item(session, xpath_index, &val_index, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/accumulated-latency", &xpath_accumulated_latency);
    sr_val_t val_accumulated_latency;
    val_accumulated_latency.type = SR_UINT32_T;
    val_accumulated_latency.data.uint32_val = sl->accumulated_latency;
    rc = sr_set_item(session, xpath_accumulated_latency, &val_accumulated_latency, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/interface-configuration", &xpath_interface_configuration);
    rc = _write_interface_configuration(xpath_interface_configuration, &(sl->interface_configuration));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
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
    sr_val_t *val_failed_interfaces = NULL;
    //char *xpath_status_stream = NULL;
    char *xpath_status_info = NULL;
    char *xpath_failed_interfaces = NULL;
    char *xpath_status_talker = NULL;
    char *xpath_listener_list = NULL;

    /*
    // Read status stream
    _create_xpath(xpath, "", &xpath_status_stream);
    IEEE_StatusStream *ss = malloc(sizeof(IEEE_StatusStream));
    rc = _read_status_stream(xpath_status_stream, &ss);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    //(*con)->status_info = ss->status_info;
    //(*con)->count_failed_interfaces = ss->count_failed_interfaces;
    //(*con)->failed_interfaces = ss->failed_interfaces;
    (*con)->status_stream = *ss;
    */

    // Read status info
    _create_xpath(xpath, "/status-info", &xpath_status_info);
    IEEE_StatusInfo *si = malloc(sizeof(IEEE_StatusInfo));
    rc = _read_status_info(xpath_status_info, &si);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*con)->status_info = *si;

    // Read failed interfaces
    _create_xpath(xpath, "/failed-interfaces", &xpath_failed_interfaces);
    size_t count_failed_interfaces = 0;
    rc = sr_get_items(session, xpath_failed_interfaces, 0, 0, &val_failed_interfaces, &count_failed_interfaces);
    (*con)->failed_interfaces = (IEEE_InterfaceId *) malloc(sizeof(IEEE_InterfaceId) * count_failed_interfaces);
    (*con)->count_failed_interfaces = count_failed_interfaces;
    for (int i=0; i<count_failed_interfaces; ++i) {
        IEEE_InterfaceId *ii = malloc(sizeof(IEEE_InterfaceId));
        rc = _read_interface_id((&val_failed_interfaces[i])->xpath, &ii);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*con)->failed_interfaces[i] = *ii;
    }


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
    //free(xpath_status_stream);
    free(xpath_status_info);
    free(xpath_failed_interfaces);
    free(xpath_status_talker);
    free(xpath_listener_list);

    return rc;
}

static int
_write_configuration(char *xpath, TSN_Configuration *con)
{
    int rc = SR_ERR_OK;
    //char *xpath_status_stream = NULL;
    char *xpath_status_info = NULL;
    char *xpath_failed_interfaces = NULL;
    char *xpath_status_talker = NULL;
    char *xpath_listener_list = NULL;

    /*
    _create_xpath(xpath, "", &xpath_status_stream);
    rc = _write_status_stream(xpath_status_stream, &(con->status_stream));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    */


    _create_xpath(xpath, "/status-info", &xpath_status_info);
    rc = _write_status_info(xpath_status_info, &(con->status_info));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/failed-interfaces[mac-address='%s'][interface-name='%s']", &xpath_failed_interfaces);
    for (int i=0; i<con->count_failed_interfaces; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_key_multi(xpath_failed_interfaces, con->failed_interfaces[i].mac_address, con->failed_interfaces[i].interface_name, &xpath_entry);
        rc = _write_interface_id(xpath_entry, &(con->failed_interfaces[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    _create_xpath(xpath, "/talker", &xpath_status_talker);
    rc = _write_status_talker(xpath_status_talker, &(con->talker));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/listener-list/listener[index='%d']", &xpath_listener_list);
    for (int i=0; i<con->count_listeners; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_id(xpath_listener_list, con->listener_list[i].index, &xpath_entry);
        rc = _write_status_listener(xpath_entry, &(con->listener_list[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

cleanup:
    //free(xpath_status_stream);
    free(xpath_status_info);
    free(xpath_failed_interfaces);
    free(xpath_status_talker);
    free(xpath_listener_list);

    return rc;
}

static int
_read_stream(char *xpath, TSN_Stream **stream)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_stream_id = NULL;
    sr_val_t *val_configured = NULL;
    char *xpath_stream_id = NULL;
    char *xpath_request = NULL;
    char *xpath_configured = NULL;
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

    // Check if configuration is available
    _create_xpath(xpath, "/configured", &xpath_configured);
    rc = sr_get_item(session, xpath_configured, 0, &val_configured);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*stream)->configured = val_configured->data.uint8_val;

    TSN_Configuration *con = NULL;
    if ((*stream)->configured) {
        // Configuration
        _create_xpath(xpath, "/configuration", &xpath_configuration);
        con = malloc(sizeof(TSN_Configuration));
        rc = _read_configuration(xpath_configuration, &con);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }
    (*stream)->configuration = con;


cleanup:
    sr_free_val(val_stream_id);
    sr_free_val(val_configured);
    free(xpath_stream_id);
    free(xpath_request);
    free(xpath_configured);
    free(xpath_configuration);

    return rc;
}

static int
_write_stream(char *xpath, TSN_Stream *stream)
{
    int rc = SR_ERR_OK;
    char *xpath_stream_id = NULL;
    char *xpath_request = NULL;
    char *xpath_configured = NULL;
    char *xpath_configuration = NULL;

    _create_xpath(xpath, "/stream-id", &xpath_stream_id);
    rc = sr_set_item_str(session, xpath_stream_id, stream->stream_id, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/request", &xpath_request);
    rc = _write_request(xpath_request, &(stream->request));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    _create_xpath(xpath, "/configured", &xpath_configured);
    sr_val_t val_configured;
    val_configured.type = SR_UINT8_T;
    val_configured.data.uint8_val = stream->configured;
    rc = sr_set_item(session, xpath_configured, &val_configured, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    if (stream->configured) {
        _create_xpath(xpath, "/configuration", &xpath_configuration);
        rc = _write_configuration(xpath_configuration, stream->configuration);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

cleanup:
    free(xpath_stream_id);
    free(xpath_request);
    free(xpath_configured);
    free(xpath_configuration);

    return rc;
}

static int
_read_streams(char *xpath, TSN_Streams **streams, uint8_t without_configured_ones)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_streams = NULL;
    char *xpath_streams = NULL;

    _create_xpath(xpath, "/*", &xpath_streams);
    size_t count_streams = 0;
    rc = sr_get_items(session, xpath_streams, 0, 0, &val_streams, &count_streams);
    //(*streams)->count_streams = count_streams;
    uint16_t count_s = 0;
    (*streams)->streams = (TSN_Stream *) malloc(sizeof(TSN_Stream) * count_streams);
    for (int i=0; i<count_streams; ++i) {
        TSN_Stream *s = malloc(sizeof(TSN_Stream));
        rc = _read_stream((&val_streams[i])->xpath, &s);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        if (without_configured_ones && s->configured) {
            continue;
        }
        (*streams)->streams[i] = *s;
        count_s += 1;
    }
    (*streams)->count_streams = count_s;

cleanup:
    sr_free_val(val_streams);
    free(xpath_streams);

    return rc;
}

static int
_write_streams(char *xpath, TSN_Streams *streams)
{
    int rc = SR_ERR_OK;
    char *xpath_streams = NULL;

    _create_xpath(xpath, "/stream[stream-id='%s']", &xpath_streams);
    for (int i=0; i<streams->count_streams; ++i) {
        char *xpath_entry = NULL;
        _create_xpath_key(xpath_streams, streams->streams[i].stream_id, &xpath_entry);
        rc = _write_stream(xpath_entry, &(streams->streams[i]));
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

cleanup:
    free(xpath_streams);

    return rc;
}

// -------------------------------
// Modules
// -------------------------------
static int
_read_module_data_entry(char *xpath, TSN_Module_Data_Entry **entry)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_data_name = NULL;
    sr_val_t *val_data_type = NULL;
    sr_val_t *val_data_value = NULL;
    char *xpath_data_name = NULL;
    char *xpath_data_type = NULL;
    char *xpath_data_value = NULL;

    // Name
    _create_xpath(xpath, "/data-name", &xpath_data_name);
    rc = sr_get_item(session, xpath_data_name, 0, &val_data_name);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*entry)->name = strdup(val_data_name->data.string_val);

    // Type
    _create_xpath(xpath, "/data-type", &xpath_data_type);
    rc = sr_get_item(session, xpath_data_type, 0, &val_data_type);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*entry)->type = string_to_data_type(val_data_type->data.enum_val);

    // Value
    _create_xpath(xpath, "/data-value", &xpath_data_value);
    rc = sr_get_item(session, xpath_data_value, 0, &val_data_value);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*entry)->value = sysrepo_data_to_data_value(val_data_value->data, (*entry)->type);

cleanup:
    sr_free_val(val_data_name);
    sr_free_val(val_data_type);
    sr_free_val(val_data_value);
    free(xpath_data_name);
    free(xpath_data_type);
    free(xpath_data_value);

    return rc;
}

static int
_write_module_data_entry(char *xpath, TSN_Module_Data_Entry *entry)
{
    int rc = SR_ERR_OK;
    char *xpath_data_name = NULL;
    char *xpath_data_type = NULL;
    char *xpath_data_value = NULL;

    // Name
    _create_xpath(xpath, "/data-name", &xpath_data_name);
    rc = sr_set_item_str(session, xpath_data_name, entry->name, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Type
    _create_xpath(xpath, "/data-type", &xpath_data_type);
    sr_val_t val_type;
    val_type.type = SR_ENUM_T;
    char *enum_string = strdup(data_type_to_string(entry->type));
    val_type.data.enum_val = enum_string;
    rc = sr_set_item(session, xpath_data_type, &val_type, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Value
    _create_xpath(xpath, "/data-value", &xpath_data_value);
    sr_val_t val_value;
    val_value = data_value_to_sysrepo_value(entry->value, entry->type);
    rc = sr_set_item(session, xpath_data_value, &val_value, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    free(xpath_data_name);
    free(xpath_data_type);
    free(xpath_data_value);

    return rc;
}

static int
_read_module_data(char *xpath, TSN_Module_Data **module_data)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_data = NULL;
    char *xpath_data = NULL;

    _create_xpath(xpath, "/*", &xpath_data);
    size_t count_data_entries = 0;
    rc = sr_get_items(session, xpath_data, 0, 0, &val_data, &count_data_entries);
    (*module_data)->count_entries = count_data_entries;
    (*module_data)->entries = (TSN_Module_Data_Entry *) malloc(sizeof(TSN_Module_Data_Entry) * count_data_entries);
    for (int i=0; i<count_data_entries; ++i) {
        TSN_Module_Data_Entry *e = malloc(sizeof(TSN_Module_Data_Entry));
        rc = _read_module_data_entry((&val_data[i])->xpath, &e);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*module_data)->entries[i] = *e;
        free(e);
    }

cleanup:
    sr_free_val(val_data);
    free(xpath_data);

    return rc;
}

static int
_write_module_data(char *xpath, TSN_Module_Data *module_data)
{
    int rc = SR_ERR_OK;
    char *xpath_data = NULL;

    _create_xpath(xpath, "/entry[data-name='%s']", &xpath_data);

    for (int i=0; i<module_data->count_entries; ++i) {
        char *xpath_data_entry = NULL;
        TSN_Module_Data_Entry *e = &(module_data->entries[i]);
        _create_xpath_key(xpath_data, e->name, &xpath_data_entry);
        rc = _write_module_data_entry(xpath_data_entry, e);
        free(xpath_data_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_data);

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
    sr_val_t *val_pid = NULL;
    sr_val_t *val_data = NULL;
    sr_val_t *val_registered = NULL;
    char *xpath_id = NULL;
    char *xpath_name = NULL;
    char *xpath_description = NULL;
    char *xpath_path = NULL;
    char *xpath_subscribed_events_mask = NULL;
    char *xpath_pid = NULL;
    char *xpath_data = NULL;
    char *xpath_registered = NULL;

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
    (*mod)->subscribed_events_mask = val_subscribed_events_mask->data.uint32_val;

    // Registered
    _create_xpath(xpath, "/registered", &xpath_registered);
    rc = sr_get_item(session, xpath_registered, 0, &val_registered);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*mod)->registered = val_registered->data.uint8_val;

    // Module Data
    _create_xpath(xpath, "/data", &xpath_data);
    TSN_Module_Data *data = malloc(sizeof(TSN_Module_Data));
    rc = _read_module_data(xpath_data, &data);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*mod)->data = *data;
    free(data);

    // ONLY READ PID OF REGISTERED MODULES
    if ((*mod)->registered) {
        // PID
        _create_xpath(xpath, "/pid", &xpath_pid);
        rc = sr_get_item(session, xpath_pid, 0, &val_pid);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*mod)->p_id = val_pid->data.uint32_val;
    }

cleanup:
    sr_free_val(val_id);
    sr_free_val(val_name);
    sr_free_val(val_description);
    sr_free_val(val_path);
    sr_free_val(val_subscribed_events_mask);
    sr_free_val(val_pid);
    sr_free_val(val_data);
    sr_free_val(val_registered);
    free(xpath_id);
    free(xpath_name);
    free(xpath_description);
    free(xpath_path);
    free(xpath_subscribed_events_mask);
    free(xpath_pid);
    free(xpath_data);
    free(xpath_registered);

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
    char *xpath_pid = NULL;
    char *xpath_data = NULL;
    char *xpath_registered = NULL;

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
    val_subscribed_events_mask.type = SR_UINT32_T;
    val_subscribed_events_mask.data.uint32_val = mod->subscribed_events_mask;
    rc = sr_set_item(session, xpath_subscribed_events_mask, &val_subscribed_events_mask, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write Registered
    _create_xpath(xpath, "/registered", &xpath_registered);
    sr_val_t val_registered;
    val_registered.type = SR_UINT8_T;
    val_registered.data.uint8_val = mod->registered;
    rc = sr_set_item(session, xpath_registered, &val_registered, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write Data
    _create_xpath(xpath, "/data", &xpath_data);
    rc = _write_module_data(xpath_data, &(mod->data));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // ONLY WRITE PID TO REGISTERED MODULES
    if (mod->registered) {
        // Write PID
        _create_xpath(xpath, "/pid", &xpath_pid);
        sr_val_t val_pid;
        val_pid.type = SR_UINT32_T;
        val_pid.data.uint32_val = mod->p_id;
        rc = sr_set_item(session, xpath_pid, &val_pid, 0);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    // Apply the changes
    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    free(xpath_id);
    free(xpath_name);
    free(xpath_description);
    free(xpath_path);
    free(xpath_subscribed_events_mask);
    free(xpath_pid);
    free(xpath_data);

    return rc;
}

static int
_read_modules(char *xpath, TSN_Modules **modules)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_modules = NULL;
    char *xpath_modules = NULL;

    /*
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
    */

    _create_xpath(xpath, "/*", &xpath_modules);
    size_t count_modules = 0;
    rc = sr_get_items(session, xpath_modules, 0, 0, &val_modules, &count_modules);
    (*modules)->count_modules = count_modules;
    (*modules)->modules = (TSN_Module *) malloc(sizeof(TSN_Module) * count_modules);
    for (int i=0; i<count_modules; ++i) {
        TSN_Module *m = malloc(sizeof(TSN_Module));
        rc = _read_module((&val_modules[i])->xpath, &m);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*modules)->modules[i] = *m;
        free(m);
    }

cleanup:
    sr_free_val(val_modules);
    free(xpath_modules);

    return rc;
}

// -------------------------------
// Topology
// -------------------------------
static int 
_read_enddevice_app_ref(char *xpath, TSN_Enddevice_AppRef **app_ref)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_app_ref = NULL;
    char *xpath_app_ref = NULL;

    _create_xpath(xpath, "/app-ref", &xpath_app_ref);
    rc = sr_get_item(session, xpath_app_ref, 0, &val_app_ref);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*app_ref)->app_ref = strdup(val_app_ref->data.string_val);

cleanup:
    sr_free_val(val_app_ref);
    free(xpath_app_ref);

    return rc;
}

static int
_read_enddevice(char *xpath, TSN_Enddevice **enddevice)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_mac = NULL;
    sr_val_t *val_apps = NULL;
    sr_val_t *val_has_app = NULL;
    //sr_val_t *val_app_ref = NULL;
    char *xpath_mac = NULL;
    char *xpath_apps = NULL;
    char *xpath_has_app = NULL;
    //char *xpath_app_ref = NULL;

    // MAC
    _create_xpath(xpath, "/mac", &xpath_mac);
    rc = sr_get_item(session, xpath_mac, 0, &val_mac);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*enddevice)->mac = strdup(val_mac->data.string_val);

    
    // Has App
    _create_xpath(xpath, "/has-app", &xpath_has_app);
    rc = sr_get_item(session, xpath_has_app, 0, &val_has_app);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*enddevice)->has_app = val_has_app->data.uint8_val;

    /*
    // App Ref
    if ((*enddevice)->has_app) {
        _create_xpath(xpath, "/app-ref", &xpath_app_ref);
        rc = sr_get_item(session, xpath_app_ref, 0, &val_app_ref);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*enddevice)->app_ref = strdup(val_app_ref->data.string_val);
    }
    */

    // Apps
    if ((*enddevice)->has_app) {
        _create_xpath(xpath, "/apps/*", &xpath_apps);
        size_t count_apps = 0;
        rc = sr_get_items(session, xpath_apps, 0, 0, &val_apps, &count_apps);
        (*enddevice)->count_apps = count_apps;
        (*enddevice)->apps = (TSN_Enddevice_AppRef *) malloc(sizeof(TSN_Enddevice_AppRef) * count_apps);
        for (int i=0; i<count_apps; ++i) {
            TSN_Enddevice_AppRef *a = malloc(sizeof(TSN_Enddevice_AppRef));
            rc = _read_enddevice_app_ref((&val_apps[i])->xpath, &a);
            if (rc != SR_ERR_OK) {
                goto cleanup;
            }
            (*enddevice)->apps[i] = *a;
            free(a);
        }
    }

cleanup:
    sr_free_val(val_mac);
    sr_free_val(val_has_app);
    //sr_free_val(val_app_ref);
    sr_free_val(val_apps);
    free(xpath_mac);
    free(xpath_has_app);
    //free(xpath_app_ref);
    free(xpath_apps);

    return rc;
}

static int
_read_switch(char *xpath, TSN_Switch **sw)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_mac = NULL;
    sr_val_t *val_ports_count = NULL;
    char *xpath_mac = NULL;
    char *xpath_ports_count = NULL;

    // MAC
    _create_xpath(xpath, "/mac", &xpath_mac);
    rc = sr_get_item(session, xpath_mac, 0, &val_mac);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*sw)->mac = strdup(val_mac->data.string_val);

    // Ports Count
    _create_xpath(xpath, "/ports-count", &xpath_ports_count);
    rc = sr_get_item(session, xpath_ports_count, 0, &val_ports_count);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*sw)->ports_count = val_ports_count->data.uint8_val;

cleanup:
    sr_free_val(val_mac);
    sr_free_val(val_ports_count);
    free(xpath_mac);
    free(xpath_ports_count);

    return rc;
}

static int
_read_devices(char *xpath, TSN_Devices **devices)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_enddevices = NULL;
    sr_val_t *val_switches = NULL;
    char *xpath_enddevices = NULL;
    char *xpath_switches = NULL;

    // Read all enddevices
    _create_xpath(xpath, "/enddevices/*", &xpath_enddevices);
    size_t count_enddevices = 0;
    rc = sr_get_items(session, xpath_enddevices, 0, 0, &val_enddevices, &count_enddevices);
    (*devices)->count_enddevices = count_enddevices;
    (*devices)->enddevices = (TSN_Enddevice *) malloc(sizeof(TSN_Enddevice) * count_enddevices);
    for (int i=0; i<count_enddevices; ++i) {
        TSN_Enddevice *e = malloc(sizeof(TSN_Enddevice));
        rc = _read_enddevice((&val_enddevices[i])->xpath, &e);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*devices)->enddevices[i] = *e;
        free(e);
    }

    // Read all switches
    _create_xpath(xpath, "/switches/*", &xpath_switches);
    size_t count_switches = 0;
    rc = sr_get_items(session, xpath_switches, 0, 0, &val_switches, &count_switches);
    (*devices)->count_switches = count_switches;
    (*devices)->switches = (TSN_Switch *) malloc(sizeof(TSN_Switch) * count_switches);
    for (int i=0; i<count_switches; ++i) {
        TSN_Switch *s = malloc(sizeof(TSN_Switch));
        rc = _read_switch((&val_switches[i])->xpath, &s);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*devices)->switches[i] = *s;
        free(s);
    }

cleanup:
    sr_free_val(val_enddevices);
    sr_free_val(val_switches);
    free(xpath_enddevices);
    free(xpath_switches);

    return rc;
}

static int
_read_connection(char *xpath, TSN_Connection **connection)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_id = NULL;
    sr_val_t *val_from_mac = NULL;
    sr_val_t *val_from_port = NULL;
    sr_val_t *val_to_mac = NULL;
    sr_val_t *val_to_port = NULL;
    char *xpath_id = NULL;
    char *xpath_from_mac = NULL;
    char *xpath_from_port = NULL;
    char *xpath_to_mac = NULL;
    char *xpath_to_port = NULL;

    // ID
    _create_xpath(xpath, "/id", &xpath_id);
    rc = sr_get_item(session, xpath_id, 0, &val_id);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*connection)->id = val_id->data.uint8_val;

    // From MAC
    _create_xpath(xpath, "/from-mac", &xpath_from_mac);
    rc = sr_get_item(session, xpath_from_mac, 0, &val_from_mac);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*connection)->from_mac = strdup(val_from_mac->data.string_val);

    // From Port
    _create_xpath(xpath, "/from-port", &xpath_from_port);
    rc = sr_get_item(session, xpath_from_port, 0, &val_from_port);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*connection)->from_port = val_from_port->data.uint8_val;

    // To MAC
    _create_xpath(xpath, "/to-mac", &xpath_to_mac);
    rc = sr_get_item(session, xpath_to_mac, 0, &val_to_mac);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*connection)->to_mac = strdup(val_to_mac->data.string_val);

    // To Port
    _create_xpath(xpath, "/to-port", &xpath_to_port);
    rc = sr_get_item(session, xpath_to_port, 0, &val_to_port);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*connection)->to_port = val_to_port->data.uint8_val;

cleanup:
    sr_free_val(val_id);
    sr_free_val(val_from_mac);
    sr_free_val(val_from_port);
    sr_free_val(val_to_mac);
    sr_free_val(val_to_port);
    free(xpath_id);
    free(xpath_from_mac);
    free(xpath_from_port);
    free(xpath_to_mac);
    free(xpath_to_port);

    return rc;
}

static int
_read_graph(char *xpath, TSN_Graph **graph)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_connections = NULL;
    char *xpath_connections = NULL;

    // Read all enddevices
    _create_xpath(xpath, "/connections/*", &xpath_connections);
    size_t count_connections = 0;
    rc = sr_get_items(session, xpath_connections, 0, 0, &val_connections, &count_connections);
    (*graph)->count_connections = count_connections;
    (*graph)->connections = (TSN_Connection *) malloc(sizeof(TSN_Connection) * count_connections);
    for (int i=0; i<count_connections; ++i) {
        TSN_Connection *c = malloc(sizeof(TSN_Connection));
        rc = _read_connection((&val_connections[i])->xpath, &c);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*graph)->connections[i] = *c;
        free(c);
    }

cleanup:
    sr_free_val(val_connections);
    free(xpath_connections);

    return rc;
}

static int
_read_topology(char *xpath, TSN_Topology **topology)
{
    int rc = SR_ERR_OK;
    char *xpath_devices = NULL;
    char *xpath_graph = NULL;

    // Devices
    _create_xpath(xpath, "/devices", &xpath_devices);
    TSN_Devices *devices = NULL;
    devices = malloc(sizeof(TSN_Devices));
    rc = _read_devices(xpath_devices, &devices);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*topology)->devices = *devices;
    free(devices);

    // Graph
    _create_xpath(xpath, "/graph", &xpath_graph);
    TSN_Graph *graph = NULL;
    graph = malloc(sizeof(TSN_Graph));
    rc = _read_graph(xpath_graph, &graph);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*topology)->graph = *graph;
    free(graph);

cleanup:
    free(xpath_devices);
    free(xpath_graph);

    return rc;
}

static int
_write_enddevice_app_ref(char *xpath, TSN_Enddevice_AppRef *app_ref)
{
    int rc = SR_ERR_OK;
    char *xpath_app_ref = NULL;

    _create_xpath(xpath, "/app-ref", &xpath_app_ref);
    rc = sr_set_item_str(session, xpath_app_ref, app_ref->app_ref, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_app_ref);
    return rc;
}

static int
_write_enddevice(char *xpath, TSN_Enddevice *enddevice)
{
    int rc = SR_ERR_OK;
    char *xpath_mac = NULL;
    char *xpath_has_app = NULL;
    //char *xpath_app_ref = NULL;
    char *xpath_apps = NULL;

    // Mac
    _create_xpath(xpath, "/mac", &xpath_mac);
    rc = sr_set_item_str(session, xpath_mac, enddevice->mac, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Has App
    _create_xpath(xpath, "/has-app", &xpath_has_app);
    sr_val_t val_has_app;
    val_has_app.type = SR_UINT8_T;
    val_has_app.data.uint8_val = enddevice->has_app;
    rc = sr_set_item(session, xpath_has_app, &val_has_app, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    /*
    // App Ref
    if (enddevice->has_app) {
        _create_xpath(xpath, "/app-ref", &xpath_app_ref);
        rc = sr_set_item_str(session, xpath_app_ref, enddevice->app_ref, NULL, 0);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }  
    }
    */

    // Apps
    if (enddevice->has_app) {
        _create_xpath(xpath, "/apps/app[app-ref='%s']", &xpath_apps);
        for (int i=0; i<enddevice->count_apps; ++i) {
            TSN_Enddevice_AppRef *ar = &(enddevice->apps[i]);
            char *xpath_entry = NULL;
            _create_xpath_key(xpath_apps, ar->app_ref, &xpath_entry);
            rc = _write_enddevice_app_ref(xpath_entry, ar);
            free(xpath_entry);
            if (rc != SR_ERR_OK) {
                goto cleanup;
            }
        }
    }

    // Apply the changes
    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    free(xpath_mac);
    free(xpath_has_app);
    //free(xpath_app_ref);
    free(xpath_apps);

    return rc;
}

static int
_write_switch(char *xpath, TSN_Switch *sw)
{
    int rc = SR_ERR_OK;
    char *xpath_mac = NULL;
    char *xpath_ports_count = NULL;

    // Mac
    _create_xpath(xpath, "/mac", &xpath_mac);
    rc = sr_set_item_str(session, xpath_mac, sw->mac, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Ports count
    _create_xpath(xpath, "/ports-count", &xpath_ports_count);
    sr_val_t val_ports_count;
    val_ports_count.type = SR_UINT8_T;
    val_ports_count.data.uint8_val = sw->ports_count;
    rc = sr_set_item(session, xpath_ports_count, &val_ports_count, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    free(xpath_mac);
    free(xpath_ports_count);

    return rc;
}

static int
_write_devices(char *xpath, TSN_Devices *devices)
{
    int rc = SR_ERR_OK;
    char *xpath_enddevices = NULL;
    char *xpath_switches = NULL;

    // Write Enddevices
    _create_xpath(xpath, "/enddevices/enddevice[mac='%s']", &xpath_enddevices);
    for (int i=0; i<devices->count_enddevices; ++i) {
        TSN_Enddevice *ed = &(devices->enddevices[i]);
        char *xpath_entry = NULL;
        _create_xpath_key(xpath_enddevices, ed->mac, &xpath_entry);
        rc = _write_enddevice(xpath_entry, ed);
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    // Write Switches
    _create_xpath(xpath, "/switches/switch[mac='%s']", &xpath_switches);
    for (int i=0; i<devices->count_switches; ++i) {
        TSN_Switch *sw = &(devices->switches[i]);
        char *xpath_entry = NULL;
        _create_xpath_key(xpath_switches, sw->mac, &xpath_entry);
        rc = _write_switch(xpath_entry, sw);
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    // Apply the changes
    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    free(xpath_enddevices);
    free(xpath_switches);

    return rc;
}

static int
_write_connection(char *xpath, TSN_Connection *connection)
{
    int rc = SR_ERR_OK;
    char *xpath_id = NULL;
    char *xpath_from_mac = NULL;
    char *xpath_from_port = NULL;
    char *xpath_to_mac = NULL;
    char *xpath_to_port = NULL;

    // ID
    _create_xpath(xpath, "/id", &xpath_id);
    sr_val_t val_id;
    val_id.type = SR_UINT16_T;
    val_id.data.uint16_val = connection->id;
    rc = sr_set_item(session, xpath_id, &val_id, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // From MAC
    _create_xpath(xpath, "/from-mac", &xpath_from_mac);
    rc = sr_set_item_str(session, xpath_from_mac, connection->from_mac, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // From Port
    _create_xpath(xpath, "/from-port", &xpath_from_port);
    sr_val_t val_from_port;
    val_from_port.type = SR_UINT8_T;
    val_from_port.data.uint8_val = connection->from_port;
    rc = sr_set_item(session, xpath_from_port, &val_from_port, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // To MAC
    _create_xpath(xpath, "/to-mac", &xpath_to_mac);
    rc = sr_set_item_str(session, xpath_to_mac, connection->to_mac, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // To Port
    _create_xpath(xpath, "/to-port", &xpath_to_port);
    sr_val_t val_to_port;
    val_to_port.type = SR_UINT8_T;
    val_to_port.data.uint8_val = connection->to_port;
    rc = sr_set_item(session, xpath_to_port, &val_to_port, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    free(xpath_id);
    free(xpath_from_mac);
    free(xpath_from_port);
    free(xpath_to_mac);
    free(xpath_to_port);

    return rc;
}

static int
_write_graph(char *xpath, TSN_Graph *graph)
{
    int rc = SR_ERR_OK;
    char *xpath_connections = NULL;

    // Write Connections
    _create_xpath(xpath, "/connections/connection[id='%d']", &xpath_connections);
    for (int i=0; i<graph->count_connections; ++i) {
        TSN_Connection *c = &(graph->connections[i]);
        char *xpath_entry = NULL;
        _create_xpath_id(xpath_connections, c->id, &xpath_entry);
        rc = _write_connection(xpath_entry, c);
        free(xpath_entry);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    // Apply the changes
    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    free(xpath_connections);

    return rc;
}

static int
_write_topology(char *xpath, TSN_Topology *topology)
{
    int rc = SR_ERR_OK;
    char *xpath_devices = NULL;
    char *xpath_graph = NULL;

    // Write Devices
    _create_xpath(xpath, "/devices", &xpath_devices);
    rc = _write_devices(xpath_devices, &(topology->devices));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write Graph
    _create_xpath(xpath, "/graph", &xpath_graph);
    rc = _write_graph(xpath_graph, &(topology->graph));
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_devices);
    free(xpath_graph);

    return rc;
}

static int
_remove_topology()
{
    int rc = SR_ERR_OK;

    rc = sr_delete_item(session, "/control-tsn-uni:tsn-uni/topology", 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    //rc = sr_apply_changes(session, 0, 1);
    //if (rc != SR_ERR_OK) {
    //    goto cleanup;
    //}

cleanup:
    return rc;
}

// -------------------------------
// Application
// -------------------------------
static int _remove_images()
{
    return sr_delete_item(session, "/control-tsn-uni:tsn-uni/application/images", 0);
}

static int _remove_apps()
{
    return sr_delete_item(session, "/control-tsn-uni:tsn-uni/application/apps", 0);
}

static int _write_image(char *xpath, TSN_Image *image)
{
    char *xpath_version = NULL;
    char *xpath_name = NULL;
    char *xpath_desc = NULL;
    char *xpath_id = NULL;
    int ret;

    // ID
    _create_xpath(xpath, "/id", &xpath_id);
    ret = sr_set_item_str(session, xpath_id, image->id, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Name
    _create_xpath(xpath, "/name", &xpath_name);
    ret = sr_set_item_str(session, xpath_name, image->name, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Description
    _create_xpath(xpath, "/desc", &xpath_desc);
    ret = sr_set_item_str(session, xpath_desc, image->description, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Version
    _create_xpath(xpath, "/version", &xpath_version);
    ret = sr_set_item_str(session, xpath_version, image->version, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

cleanup:
    free(xpath_id);
    free(xpath_name);
    free(xpath_desc);
    free(xpath_version);

    return ret;
}

static int
_write_parameter(char *xpath, TSN_App_Parameter *parameter)
{
    char *xpath_param_name = NULL;
    char *xpath_param_type = NULL;
    char *xpath_param_value = NULL;
    sr_val_t val_param_value;
    sr_val_t val_param_type;
    int ret = SR_ERR_OK;

    // Name
    _create_xpath(xpath, "/param-name", &xpath_param_name);
    ret = sr_set_item_str(session, xpath_param_name, parameter->name, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Type
    _create_xpath(xpath, "/param-type", &xpath_param_type);
    val_param_type.data.enum_val = data_type_to_string(parameter->type);
    val_param_type.type = SR_ENUM_T;
    ret = sr_set_item(session, xpath_param_type, &val_param_type, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Value
    _create_xpath(xpath, "/param-value", &xpath_param_value);
    val_param_value = data_value_to_sysrepo_value(parameter->value, parameter->type);
    ret = sr_set_item(session, xpath_param_value, &val_param_value, 0);

cleanup:
    free(xpath_param_name);
    free(xpath_param_type);
    free(xpath_param_value);

    return ret;
}

static int _write_app(char *xpath, TSN_App *app)
{
    char *xpath_parameters = NULL;
    char *xpath_has_image = NULL;
    char *xpath_version = NULL;
    char *xpath_image = NULL;
    char *xpath_name = NULL;
    char *xpath_desc = NULL;
    char *xpath_id = NULL;
    sr_val_t has_image;
    int ret, i;

    // ID
    _create_xpath(xpath, "/id", &xpath_id);
    ret = sr_set_item_str(session, xpath_id, app->id, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Name
    _create_xpath(xpath, "/name", &xpath_name);
    ret = sr_set_item_str(session, xpath_name, app->name, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Description
    _create_xpath(xpath, "/desc", &xpath_desc);
    ret = sr_set_item_str(session, xpath_desc, app->description, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Version
    _create_xpath(xpath, "/version", &xpath_version);
    ret = sr_set_item_str(session, xpath_version, app->version, NULL, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Has-Image
    _create_xpath(xpath, "/has-image", &xpath_has_image);
    has_image.type = SR_UINT8_T;
    has_image.data.uint8_val = app->has_image;
    ret = sr_set_item(session, xpath_has_image, &has_image, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Image
    if (app->has_image) {
        _create_xpath(xpath, "/image-ref", &xpath_image);
        ret = sr_set_item_str(session, xpath_image, app->image_ref, NULL, 0);
        if (ret != SR_ERR_OK)
            goto cleanup;
    }


    // Parameters
    _create_xpath(xpath, "/parameters/parameter[param-name='%s']", &xpath_parameters);
    for (i = 0; i < app->count_parameters; ++i) {
        TSN_App_Parameter *par = &app->parameters[i];
        char *xpath_entry = NULL;

        _create_xpath_key(xpath_parameters, par->name, &xpath_entry);

        ret = _write_parameter(xpath_entry, par);
        free(xpath_entry);

        if (ret != SR_ERR_OK)
            goto cleanup;
    }

cleanup:
    free(xpath_id);
    free(xpath_name);
    free(xpath_desc);
    free(xpath_version);
    free(xpath_image);
    free(xpath_has_image);
    free(xpath_parameters);

    return ret;
}

static int _write_images(char *xpath, TSN_Images *images)
{
    char *xpath_images = NULL;
    int ret = SR_ERR_OK;
    int i;

    _create_xpath(xpath, "/image[id='%s']", &xpath_images);

    for (i = 0; i < images->count_images; ++i) {
        TSN_Image *image = &images->images[i];
        char *xpath_entry = NULL;

        _create_xpath_key(xpath_images, image->id, &xpath_entry);

        ret = _write_image(xpath_entry, image);
        free(xpath_entry);

        if (ret != SR_ERR_OK)
            goto cleanup;
    }

cleanup:
    free(xpath_images);

    return ret;
}

static int _write_apps(char *xpath, TSN_Apps *apps)
{
    char *xpath_apps = NULL;
    int ret = SR_ERR_OK;
    int i;

    _create_xpath(xpath, "/app[id='%s']", &xpath_apps);

    for (i = 0; i < apps->count_apps; ++i) {
        TSN_App *app = &apps->apps[i];
        char *xpath_entry = NULL;

        _create_xpath_key(xpath_apps, app->id, &xpath_entry);

        ret = _write_app(xpath_entry, app);
        free(xpath_entry);

        if (ret != SR_ERR_OK)
            goto cleanup;
    }

cleanup:
    free(xpath_apps);

    return ret;
}

static int
_read_app_parameter(char *xpath, TSN_App_Parameter **parameter)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_param_name = NULL;
    sr_val_t *val_param_type = NULL;
    sr_val_t *val_param_value = NULL;
    char *xpath_param_name = NULL;
    char *xpath_param_type = NULL;
    char *xpath_param_value = NULL;

    // Name
    _create_xpath(xpath, "/param-name", &xpath_param_name);
    rc = sr_get_item(session, xpath_param_name, 0, &val_param_name);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*parameter)->name = strdup(val_param_name->data.string_val);

    // Type
    _create_xpath(xpath, "/param-type", &xpath_param_type);
    rc = sr_get_item(session, xpath_param_type, 0, &val_param_type);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*parameter)->type = string_to_data_type(val_param_type->data.enum_val);

    // Value
    _create_xpath(xpath, "/param-value", &xpath_param_value);
    rc = sr_get_item(session, xpath_param_value, 0, &val_param_value);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*parameter)->value = sysrepo_value_to_data_value(*val_param_value);
    (*parameter)->type = sysrepo_value_to_data_type(*val_param_value);

cleanup:
    sr_free_val(val_param_name);
    sr_free_val(val_param_type);
    sr_free_val(val_param_value);
    free(xpath_param_name);
    free(xpath_param_type);
    free(xpath_param_value);

    return rc;
}

static int
_read_app(char *xpath, TSN_App **app)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_id = NULL;
    sr_val_t *val_name = NULL;
    sr_val_t *val_desc = NULL;
    sr_val_t *val_has_mac = NULL;
    sr_val_t *val_mac = NULL;
    sr_val_t *val_version = NULL;
    sr_val_t *val_has_image = NULL;
    sr_val_t *val_image_ref = NULL;
    sr_val_t *val_parameters = NULL;
    char *xpath_id = NULL;
    char *xpath_name = NULL;
    char *xpath_desc = NULL;
    char *xpath_has_mac = NULL;
    char *xpath_mac = NULL;
    char *xpath_version = NULL;
    char *xpath_has_image = NULL;
    char *xpath_image_ref = NULL;
    char *xpath_parameters = NULL;

    // ID
    _create_xpath(xpath, "/id", &xpath_id);
    rc = sr_get_item(session, xpath_id, 0, &val_id);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*app)->id = strdup(val_id->data.string_val);

    // Name
    _create_xpath(xpath, "/name", &xpath_name);
    rc = sr_get_item(session, xpath_name, 0, &val_name);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*app)->name = strdup(val_name->data.string_val);

    // Description
    _create_xpath(xpath, "/desc", &xpath_desc);
    rc = sr_get_item(session, xpath_desc, 0, &val_desc);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*app)->description = strdup(val_desc->data.string_val);

    // Virtual MAC
    _create_xpath(xpath, "/has-mac", &xpath_has_mac);
    rc = sr_get_item(session, xpath_has_mac, 0, &val_has_mac);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*app)->has_mac = val_has_mac->data.uint8_val;

    if ((*app)->has_mac) {
        _create_xpath(xpath, "/mac", &xpath_mac);
        rc = sr_get_item(session, xpath_mac, 0, &val_mac);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*app)->mac = strdup(val_mac->data.string_val);
    }

    // Version
    _create_xpath(xpath, "/version", &xpath_version);
    rc = sr_get_item(session, xpath_version, 0, &val_version);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*app)->version = strdup(val_version->data.string_val);

    // Has Image
    _create_xpath(xpath, "/has-image", &xpath_has_image);
    rc = sr_get_item(session, xpath_has_image, 0, &val_has_image);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*app)->has_image = val_has_image->data.uint8_val;

    // Image Reference
    if ((*app)->has_image) {
        _create_xpath(xpath, "/image-ref", &xpath_image_ref);
        rc = sr_get_item(session, xpath_image_ref, 0, &val_image_ref);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*app)->image_ref = strdup(val_image_ref->data.string_val);
    }

    // Parameters
    _create_xpath(xpath, "/parameters/*", &xpath_parameters);
    size_t count_parameters = 0;
    rc = sr_get_items(session, xpath_parameters, 0, 0, &val_parameters, &count_parameters);
    (*app)->count_parameters = count_parameters;
    (*app)->parameters = (TSN_App_Parameter *) malloc(sizeof(TSN_App_Parameter) * count_parameters);
    for (int i=0; i<count_parameters; ++i) {
        TSN_App_Parameter *p = malloc(sizeof(TSN_App_Parameter));
        rc = _read_app_parameter((&val_parameters[i])->xpath, &p);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*app)->parameters[i] = *p;
        free(p);
    }

cleanup:
    sr_free_val(val_id);
    sr_free_val(val_name);
    sr_free_val(val_desc);
    sr_free_val(val_has_mac);
    sr_free_val(val_mac);
    sr_free_val(val_version);
    sr_free_val(val_has_image);
    sr_free_val(val_image_ref);
    sr_free_val(val_parameters);
    free(xpath_id);
    free(xpath_name);
    free(xpath_desc);
    free(xpath_has_mac);
    free(xpath_mac);
    free(xpath_version);
    free(xpath_has_image);
    free(xpath_image_ref);
    free(xpath_parameters);

    return rc;
}

static int
_read_image(char *xpath, TSN_Image **image)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_id = NULL;
    sr_val_t *val_name = NULL;
    sr_val_t *val_desc = NULL;
    sr_val_t *val_version = NULL;
    char *xpath_id = NULL;
    char *xpath_name = NULL;
    char *xpath_desc = NULL;
    char *xpath_version = NULL;

    // ID
    _create_xpath(xpath, "/id", &xpath_id);
    rc = sr_get_item(session, xpath_id, 0, &val_id);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*image)->id = strdup(val_id->data.string_val);

    // Name
    _create_xpath(xpath, "/name", &xpath_name);
    rc = sr_get_item(session, xpath_name, 0, &val_name);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*image)->name = strdup(val_name->data.string_val);

    // Description
    _create_xpath(xpath, "/desc", &xpath_desc);
    rc = sr_get_item(session, xpath_desc, 0, &val_desc);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*image)->description = strdup(val_desc->data.string_val);

    // Version
    _create_xpath(xpath, "/version", &xpath_version);
    rc = sr_get_item(session, xpath_version, 0, &val_version);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*image)->version = strdup(val_version->data.string_val);

cleanup:
    sr_free_val(val_id);
    sr_free_val(val_name);
    sr_free_val(val_desc);
    sr_free_val(val_version);
    free(xpath_id);
    free(xpath_name);
    free(xpath_desc);
    free(xpath_version);

    return rc;
}

static int
_read_apps(char *xpath, TSN_Apps **apps)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_apps = NULL;
    char *xpath_apps = NULL;

    // Apps
    _create_xpath(xpath, "/*", &xpath_apps);
    size_t count_apps = 0;
    rc = sr_get_items(session, xpath_apps, 0, 0, &val_apps, &count_apps);
    (*apps)->count_apps = count_apps;
    (*apps)->apps = (TSN_App *) malloc(sizeof(TSN_App) * count_apps);
    for (int i=0; i<count_apps; ++i) {
        TSN_App *a = malloc(sizeof(TSN_App));
        rc = _read_app((&val_apps[i])->xpath, &a);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*apps)->apps[i] = *a;
        free(a);
    }

cleanup:
    sr_free_val(val_apps);
    free(xpath_apps);

    return rc;
}

static int
_read_images(char *xpath, TSN_Images **images)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_images = NULL;
    char *xpath_images = NULL;

    // Images
    _create_xpath(xpath, "/*", &xpath_images);
    size_t count_images = 0;
    rc = sr_get_items(session, xpath_images, 0, 0, &val_images, &count_images);
    (*images)->count_images = count_images;
    (*images)->images = (TSN_Image *) malloc(sizeof(TSN_Image) * count_images);
    for (int i=0; i<count_images; ++i) {
        TSN_Image *im = malloc(sizeof(TSN_Image));
        rc = _read_image((&val_images[i])->xpath, &im);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        (*images)->images[i] = *im;
        free(im);
    }

cleanup:
    sr_free_val(val_images);
    free(xpath_images);

    return rc;
}

static int
_read_application(char *xpath, TSN_Application **application)
{
    int rc = SR_ERR_OK;
    sr_val_t *val_apps = NULL;
    sr_val_t *val_images = NULL;
    char *xpath_apps = NULL;
    char *xpath_images = NULL;

    // Apps
    _create_xpath(xpath, "/apps", &xpath_apps);
    TSN_Apps *apps = NULL;
    apps = malloc(sizeof(TSN_Apps));
    rc = _read_apps(xpath_apps, &apps);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*application)->apps = *apps;
    free(apps);

    // Images
    _create_xpath(xpath, "/images", &xpath_images);
    TSN_Images *images = NULL;
    images = malloc(sizeof(TSN_Images));
    rc = _read_images(xpath_images, &images);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*application)->images = *images;
    free(images);

cleanup:
    sr_free_val(val_apps);
    sr_free_val(val_images);
    free(xpath_apps);
    free(xpath_images);

    return rc;
}




// -------------------------------------------------------- //
//  Module handling
// -------------------------------------------------------- //
/*
int
sysrepo_add_or_get_module(TSN_Module **module)
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
        if ((strcmp((*module)->name, stored_modules->available_modules[i].name) == 0) &&
            (strcmp((*module)->path, stored_modules->available_modules[i].path) == 0)) {

            // We found a module with the same name and path
            printf("[SYSREPO] Module not added. Module with the same name and path already exists!\n");
            // Overwrite the passed struct with the stored information
            (*module)->id = stored_modules->available_modules[i].id;
            (*module)->description = stored_modules->available_modules[i].description;
            (*module)->subscribed_events_mask = stored_modules->available_modules[i].subscribed_events_mask;

            is_failure = 1;
            goto cleanup;
        }

        if (stored_modules->available_modules[i].id > highest_used_id) {
            highest_used_id = stored_modules->available_modules[i].id;
        }
    }

    // Otherwise we can add the new module
    (*module)->id = (highest_used_id + 1);
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']", (*module)->id, &xpath_mod);
    rc = _write_module(xpath_mod, (*module));   // &module
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Error adding a new module to the list!\n");
        is_failure = 1;
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }


cleanup:
    free(xpath_mod);
    free(stored_modules);

    return is_failure ? EXIT_FAILURE : EXIT_SUCCESS;
}
*/

int
sysrepo_add_module(TSN_Module **module)
{
    int is_failure = 0;
    char *xpath_module = NULL;

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
    for (int i=0; i<stored_modules->count_modules; ++i) {
        // Compare name and path
        if ((strcmp((*module)->name, stored_modules->modules[i].name) == 0 &&
            (strcmp((*module)->path, stored_modules->modules[i].path) == 0))) {

            // We found a module with the same name and path
            printf("[SYSREPO] Module not added. Module with the same name and path already exists!\n");
            is_failure = 1;
            goto cleanup;
        }

        if (stored_modules->modules[i].id > highest_used_id) {
            highest_used_id = stored_modules->modules[i].id;
        }
    }

    // Otherwise we can add the new module
    (*module)->id = (highest_used_id + 1);
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']", (*module)->id, &xpath_module);
    rc = _write_module(xpath_module, (*module));
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Error adding a new module to the list!\n");
        is_failure = 1;
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        is_failure = 1;
        goto cleanup;
    }

cleanup:
    free(xpath_module);
    free(stored_modules);

    return is_failure ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_register_module(int module_id)
{
    /*
    int is_failure = 0;
    sr_val_t *val_module_name_check = NULL;
    char *xpath_stored_module = NULL;
    //char *xpath_module_check = NULL;
    char *xpath_module_add = NULL;

    // Get the module from the datastore
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']", module_id, &xpath_stored_module);
    TSN_Module *stored_module = malloc(sizeof(TSN_Module));
    rc = _read_module(xpath_stored_module, &stored_module);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Error finding module with the ID %d in the list!\n", module_id);
        is_failure = 1;
        goto cleanup;
    }


    if (adjusted_subscribed_events_mask != -1) {
        stored_module->subscribed_events_mask = adjusted_subscribed_events_mask;
    }

    // Write module to list of registered modules
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']", module_id, &xpath_module_add);
    rc = _write_module(xpath_module_add, stored_module);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Error registering the Module!\n");
        is_failure = 1;
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_stored_module);
    free(stored_module);
    //free(xpath_module_check);
    sr_free_val(val_module_name_check);
    free(xpath_module_add);

    return is_failure ? EXIT_FAILURE : EXIT_SUCCESS;
    */

    char *xpath_registered = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/registered", module_id, &xpath_registered);
    sr_val_t val_registered;
    val_registered.type = SR_UINT8_T;
    val_registered.data.uint8_val = 1;
    rc = sr_set_item(session, xpath_registered, &val_registered, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_registered);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_unregister_module(int module_id)
{
    /*
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
    */

    char *xpath_registered = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/registered", module_id, &xpath_registered);
    sr_val_t val_registered;
    val_registered.type = SR_UINT8_T;
    val_registered.data.uint8_val = 0;
    rc = sr_set_item(session, xpath_registered, &val_registered, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_registered);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_delete_module(int module_id)
{
    /*
    // Unregister the module first
    char *xpath_module_unregister = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']", module_id, &xpath_module_unregister);
    rc = sr_delete_item(session, xpath_module_unregister, 0);

    // Delete the entry
    char *xpath_module_delete = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']", module_id, &xpath_module_delete);
    rc = sr_delete_item(session, xpath_module_delete, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    */

    char *xpath_module_delete = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']", module_id, &xpath_module_delete);
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
sysrepo_set_module_pid(int module_id, pid_t pid)
{
    char *xpath_module_pid = NULL;
    // _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/pid", module_id, &xpath_module_pid);
     _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/pid", module_id, &xpath_module_pid);

    // Set the PID
    sr_val_t val_pid;
    val_pid.type = SR_UINT32_T;
    val_pid.data.uint32_val = pid;
    rc = sr_set_item(session, xpath_module_pid, &val_pid, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module_pid);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*
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
sysrepo_get_module_from_available(int module_id, TSN_Module **module)
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
*/

int
sysrepo_get_all_modules(TSN_Modules **modules)
{
    rc = _read_modules("/control-tsn-uni:tsn-uni/modules", modules);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_module(int module_id, TSN_Module **module)
{
    char *xpath_module = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']", module_id, &xpath_module);
    rc = _read_module(xpath_module, module);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_module_data(int module_id, TSN_Module_Data **data)
{
    char *xpath_module_data = NULL;
    //_create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/data", module_id, &xpath_module_data);
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/data", module_id, &xpath_module_data);
    rc = _read_module_data(xpath_module_data, data);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module_data);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_update_module_data(int module_id, TSN_Module_Data data)
{
    char *xpath_module_data = NULL;
    //_create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/data", module_id, &xpath_module_data);
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/data", module_id, &xpath_module_data);
    // clean the data first
    rc = sr_delete_item(session, xpath_module_data, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    rc = _write_module_data(xpath_module_data, &data);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module_data);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*
int
sysrepo_set_module_pid(int module_id, pid_t pid)
{
    char *xpath_module_pid = NULL;
    _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/pid", module_id, &xpath_module_pid);

    // Set the PID
    sr_val_t val_pid;
    val_pid.type = SR_UINT32_T;
    val_pid.data.uint32_val = pid;
    rc = sr_set_item(session, xpath_module_pid, &val_pid, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_module_pid);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
*/

int
sysrepo_update_module_attributes(int module_id, const char *name, const char *description, const char *path, const uint32_t subscribed_events_mask)
{
    char *xpath_name = NULL;
    char *xpath_name_reg = NULL;
    char *xpath_description = NULL;
    char *xpath_description_reg = NULL;
    char *xpath_path = NULL;
    char *xpath_path_reg = NULL;
    char *xpath_subscribed_events_mask = NULL;
    char *xpath_subscribed_events_mask_reg = NULL;

    // Set name
    if (name != NULL) {
        //_create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']/name", module_id, &xpath_name);
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/name", module_id, &xpath_name);
        rc = sr_set_item_str(session, xpath_name, name, NULL, 0);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        /*
        // Update also registered modules if they exist
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/name", module_id, &xpath_name_reg);
        rc = sr_set_item_str(session, xpath_name_reg, name, NULL, SR_EDIT_NON_RECURSIVE);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        */
    }

    // Set description
    if (description != NULL) {
        //_create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']/desc", module_id, &xpath_description);
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/desc", module_id, &xpath_description);
        rc = sr_set_item_str(session, xpath_description, description, NULL, 0);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        /*
        // Update also registered modules if they exist
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/desc", module_id, &xpath_description_reg);
        rc = sr_set_item_str(session, xpath_description_reg, description, NULL, SR_EDIT_NON_RECURSIVE);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        */
    }

    // Set path
    if (path != NULL) {
        //_create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']/executable-path", module_id, &xpath_path);
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/executable-path", module_id, &xpath_path);
        rc = sr_set_item_str(session, xpath_path, path, NULL, 0);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        /*
        // Update also registered modules if they exist
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/executable-path", module_id, &xpath_path_reg);
        rc = sr_set_item_str(session, xpath_path_reg, path, NULL, SR_EDIT_NON_RECURSIVE);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        */
    }

    // Set subscribed events mask
    if (subscribed_events_mask >= 0) {
        //_create_xpath_id("/control-tsn-uni:tsn-uni/modules/available-modules/mod[id='%d']/subscribed-events-mask", module_id, &xpath_subscribed_events_mask);
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/mod[id='%d']/subscribed-events-mask", module_id, &xpath_subscribed_events_mask);
        sr_val_t val_mask;
        val_mask.type = SR_UINT32_T;
        val_mask.data.uint32_val = subscribed_events_mask;
        rc = sr_set_item(session, xpath_subscribed_events_mask, &val_mask, 0);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        /*
        // Update also registered modules if they exist
        _create_xpath_id("/control-tsn-uni:tsn-uni/modules/registered-modules/mod[id='%d']/subscribed-events-mask", module_id, &xpath_subscribed_events_mask_reg);
        rc = sr_set_item_str(session, xpath_subscribed_events_mask_reg, subscribed_events_mask, NULL, SR_EDIT_NON_RECURSIVE);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
        */
    }

    // Apply changes
    if (name != NULL || description != NULL || path != NULL || subscribed_events_mask >= -1) {
        rc = sr_apply_changes(session, 0, 1);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

cleanup:
    free(xpath_name);
    free(xpath_name_reg);
    free(xpath_description);
    free(xpath_description_reg);
    free(xpath_path);
    free(xpath_path_reg);
    free(xpath_subscribed_events_mask);
    free(xpath_subscribed_events_mask_reg);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}


// -------------------------------------------------------- //
//  Stream handling
// -------------------------------------------------------- //
int
sysrepo_get_all_streams(TSN_Streams **streams, uint8_t without_configured_ones)
{
    rc = _read_streams("/control-tsn-uni:tsn-uni/streams", streams, without_configured_ones);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_write_stream_request(TSN_Request *request, char **generated_stream_id)
{
    char *xpath_stream_root = NULL;
    char *xpath_stream_id = NULL;
    char *xpath_request = NULL;
    char *xpath_configured = NULL;

    // Generate a new stream id based on the talker mac and already stored streams from this talker
    char *new_stream_id = _generate_stream_id(*request);
    if (new_stream_id == NULL) {
        printf("[SYSREPO] Error generating new Stream ID!\n");
        return EXIT_FAILURE;
    }
    (*generated_stream_id) = strdup(new_stream_id);

    _create_xpath_key("/control-tsn-uni:tsn-uni/streams/stream[stream-id='%s']", new_stream_id, &xpath_stream_root);

    // Write stream id
    _create_xpath(xpath_stream_root, "/stream-id", &xpath_stream_id);
    rc = sr_set_item_str(session, xpath_stream_id, new_stream_id, NULL, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write request
    _create_xpath(xpath_stream_root, "/request", &xpath_request);
    rc = _write_request(xpath_request, request);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Set the 'configured' flag to false
    _create_xpath(xpath_stream_root, "/configured", &xpath_configured);
    sr_val_t val_configured;
    val_configured.type = SR_UINT8_T;
    val_configured.data.uint8_val = 0;
    rc = sr_set_item(session, xpath_configured, &val_configured, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_stream_root);
    free(xpath_stream_id);
    free(xpath_request);
    free(xpath_configured);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_write_stream_configuration(char *stream_id, TSN_Configuration *configuration)
{
    char *xpath_stream_root = NULL;
    char *xpath_configuration = NULL;
    char *xpath_configured = NULL;

    _create_xpath_key("/control-tsn-uni:tsn-uni/streams/stream[stream-id='%s']", stream_id, &xpath_stream_root);

    // Write configuration
    _create_xpath(xpath_stream_root, "/configuration", &xpath_configuration);
    rc = _write_configuration(xpath_configuration, configuration);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Set the 'configured' flag to true
    _create_xpath(xpath_stream_root, "/configured", &xpath_configured);
    sr_val_t val_configured;
    val_configured.type = SR_UINT8_T;
    val_configured.data.uint8_val = 1;
    rc = sr_set_item(session, xpath_configured, &val_configured, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Apply the changes
    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_stream_root);
    free(xpath_configuration);
    free(xpath_configured);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_stream(char *stream_id, TSN_Stream **stream)
{
    char *xpath_stream = NULL;

    _create_xpath_key("/control-tsn-uni:tsn-uni/streams/stream[stream-id='%s']", stream_id, &xpath_stream);
    rc = _read_stream(xpath_stream, stream);

    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_stream);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_delete_stream(char *stream_id)
{
    char *xpath_stream = NULL;
    _create_xpath_key("/control-tsn-uni:tsn-uni/streams/stream[stream-id='%s']", stream_id, &xpath_stream);

    rc = sr_delete_item(session, xpath_stream, 0);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    free(xpath_stream);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}


// -------------------------------------------------------- //
//  Topology handling
// -------------------------------------------------------- //
int
sysrepo_get_topology(TSN_Topology **topology)
{
    rc = _read_topology("/control-tsn-uni:tsn-uni/topology", topology);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_set_topology(TSN_Topology *topology)
{
    // Clear the whole topology first
    rc = _remove_topology();
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Write the topology
    if (topology) {
        rc = _write_topology("/control-tsn-uni:tsn-uni/topology", topology);
        if (rc != SR_ERR_OK) {
            goto cleanup;
        }
    }

    rc = sr_apply_changes(session, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_all_devices(TSN_Devices **devices)
{
    rc = _read_devices("/control-tsn-uni:tsn-uni/topology/devices", devices);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_topology_graph(TSN_Graph **graph)
{
    rc = _read_graph("/control-tsn-uni:tsn-uni/topology/graph", graph);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*      NOT NEEDED ANYMORE?! --> switched to generic notif in yang and a single (generic) send notif function
int
sysrepo_trigger_topology_discovery()
{
    int rc = SR_ERR_OK;
    sr_val_t *output = NULL;
    char *xpath_rpc_trigger_topology_discovery = NULL;

    // Send the RPC

    size_t ouput_count = 0;
    xpath_rpc_trigger_topology_discovery = strdup("/control-tsn-uni:rpc-trigger-topology-discovery");
    rc = sr_rpc_send(session, xpath_rpc_trigger_topology_discovery, NULL, 0, 0, &output, &ouput_count);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }



    // TEEEEEEEEEEEEEEST
    rc = sr_event_notif_send(session, "/control-tsn-uni:notif-topology-discovery-requested", NULL, 0);
    printf("[SYSREPO] Sending notification 'notif-topology-discovery-requested'... \n");
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }


cleanup:
    sr_free_val(output);
    free(xpath_rpc_trigger_topology_discovery);

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
*/

// -------------------------------------------------------- //
//  Application handling
// -------------------------------------------------------- //
int
sysrepo_get_application(TSN_Application **application)
{
    rc = _read_application("/control-tsn-uni:tsn-uni/application", application);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_application_apps(TSN_Apps **apps)
{
    rc = _read_apps("/control-tsn-uni:tsn-uni/application/apps", apps);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_application_images(TSN_Images **images)
{
    rc = _read_images("/control-tsn-uni:tsn-uni/application/images", images);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_set_application_images(TSN_Images *images)
{
    int ret;

    // Clear all images first
    ret = _remove_images();
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Write the images
    ret = _write_images("/control-tsn-uni:tsn-uni/application/images", images);
    if (rc != SR_ERR_OK)
        goto cleanup;

    ret = sr_apply_changes(session, 0, 1);

cleanup:
    return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_set_application_apps(TSN_Apps *apps)
{
    int ret;

    // Clear all apps first
    ret = _remove_apps();
    if (ret != SR_ERR_OK)
        goto cleanup;

    // Write the apps
    ret = _write_apps("/control-tsn-uni:tsn-uni/application/apps", apps);
    if (rc != SR_ERR_OK)
        goto cleanup;

    ret = sr_apply_changes(session, 0, 1);

cleanup:
    return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_set_application_app(TSN_App *app)
{
    TSN_Apps apps;
    int ret;

    // Write the app
    apps.count_apps = 1;
    apps.apps = app;
    ret = _write_apps("/control-tsn-uni:tsn-uni/application/apps", &apps);
    if (rc != SR_ERR_OK)
        goto cleanup;

    ret = sr_apply_changes(session, 0, 1);

cleanup:
    return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_get_application_app(const char *id, TSN_App **app)
{
    char *xpath_app = NULL;
    int ret;

    *app = malloc(sizeof(**app));
    if (!*app)
        return EXIT_FAILURE;
    memset(*app, '\0', sizeof(**app));

    _create_xpath_key("/control-tsn-uni:tsn-uni/application/apps/app[id='%s']", (char *)id, &xpath_app);
    ret = _read_app(xpath_app, app);
    if (ret != SR_ERR_OK)
        goto cleanup;

cleanup:
    free(xpath_app);

    return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
sysrepo_remove_application_app(const char *id)
{
    char *xpath_app = NULL;
    int ret;

    _create_xpath_key("/control-tsn-uni:tsn-uni/application/apps/app[id='%s']", (char *)id, &xpath_app);

    ret = sr_delete_item(session, xpath_app, 0);
    if (ret != SR_ERR_OK)
        goto cleanup;

    ret = sr_apply_changes(session, 0, 1);

cleanup:
    free(xpath_app);

    return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

