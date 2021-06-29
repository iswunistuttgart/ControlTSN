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
int 
sysrepo_get_root(TSN_Uni **root)
{

}

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
