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
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }
    (*req)->count_listeners = count_listeners;
    (*req)->listener_list = (TSN_Listener *) malloc(sizeof(TSN_Listener) * count_listeners);
    for (int i=0; i<count_listeners; ++i) {
        TSN_Listener *l = malloc(sizeof(TSN_Listener));
        // TODO _read_listener)
    }

cleanup:
    sr_free_val(val_listener_list);
    free(xpath_talker);
    free(xpath_listener_list);

    return rc;
}
