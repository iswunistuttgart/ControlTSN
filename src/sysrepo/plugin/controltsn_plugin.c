/**
 * @file controltsn.c
 * @author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 * @brief ControlTSN Sysrepo plugin. 
 * 
 * This plugin monitors the changes in the sysrepo configuration for the ControlTSN module.
 * If certain changes occur, such as the request for a new stream, corresponding notifications are sent.
 */

#include <sysrepo.h>


// Session
//sr_session_ctx_t *session;
// Subscriptions
sr_subscription_ctx_t *subscriptions;


// -------------------------------------------------------- //
//  Module Change Callback
// -------------------------------------------------------- //
static int
_module_change_cb(sr_session_ctx_t *session, const char *module_name, const char *xpath, sr_event_t event,
                  uint32_t request_id, void *private_data)
{
    (void) module_name;
    (void) request_id;
    (void) private_data;

    int rc = SR_ERR_OK;

    sr_val_t *val = NULL;
    sr_change_iter_t *iter = NULL;
    sr_change_oper_t op;

    // Monitor changes and send corresponding notifications
    printf("[PLUGIN] %s\n", xpath);

cleanup:
    return rc;
}

// -------------------------------------------------------- //
//  RPC Callbacks
// -------------------------------------------------------- //
/*
static int
_rpc_trigger_topology_discovery_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *path, const sr_val_t *input, const size_t input_cnt,
                                   sr_event_t event, uint32_t request_id, sr_val_t **output, size_t *output_cnt, void *private_data)
{
    (void) session;
    (void) sub_id;
    (void) path;
    (void) input;
    (void) input_cnt;
    (void) event;
    (void) request_id;
    (void) output;
    (void) output_cnt;
    (void) private_data;

    // TODO Send Notification 


    return SR_ERR_OK;
}
*/


// -------------------------------------------------------- //
//  Necessary Plugin Functions
// -------------------------------------------------------- //
int 
sr_plugin_init_cb(sr_session_ctx_t *session, void **private_data)
{
    int rc;

    (void) private_data;

    // Subscribe for module changes (also causes startup data to be copied into running and enabling the module)
    rc = sr_module_change_subscribe(session, "control-tsn-uni", NULL, _module_change_cb, NULL, 0, SR_SUBSCR_DONE_ONLY, &subscriptions);
    if (rc != SR_ERR_OK) {
        printf("[PLUGIN] Error subscribing for module changes!\n");
        goto error;
    }

    /*
    // Subscribe for RPC: trigger topology discovery
    rc = sr_rpc_subscribe(session, "/control-tsn-uni:rpc-trigger-topology-discovery", _rpc_trigger_topology_discovery_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, &subscriptions);
    if (rc != SR_ERR_OK) {
        printf("[PLUGIN] Error subscribing for RPC 'trigger-topology-discovery'!\n");
        goto error;
    }
    */

    printf("[PLUGIN] Plugin successfully initialized!\n");
    return SR_ERR_OK;

error:
    printf("[PLUGIN] Error! Plugin couldn't be initialized: %s\n", sr_strerror(rc));
    sr_unsubscribe(subscriptions);
    return rc;
}

int
sr_plugin_cleanup_cb(sr_session_ctx_t *session, void *private_data)
{
    (void) session;
    (void) private_data;

    // Freeing the subscriptions
    sr_unsubscribe(subscriptions);
    printf("[PLUGIN] Plugin cleanup finished.\n");
}