/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 * 
 * This plugin monitors the changes in the sysrepo configuration for the ControlTSN module.
 * If certain changes occur, such as the request for a new stream, corresponding notifications are sent.
 */

#include <sysrepo.h>
#include <string.h>
#include <stdio.h>

#include "../../events_definitions.h"

// Subscriptions
sr_subscription_ctx_t *_subscriptions = NULL;


/*
static int
_test_print_mask(uint32_t mask) {
    if (mask & EVENT_STREAM_REQUESTED) {
        printf("EVENT_STREAM_REQUESTED | ");
    }
    if (mask & EVENT_STREAM_CONFIGURED) {
        printf("EVENT_STREAM_CONFIGURED | ");
    }
    if (mask & EVENT_MODULE_ADDED) {
        printf("EVENT_MODULE_ADDED | ");
    }
    if (mask & EVENT_MODULE_REGISTERED) {
        printf("EVENT_MODULE_REGISTERED | ");
    }
    if (mask & EVENT_MODULE_DATA_UPDATED) {
        printf("EVENT_MODULE_DATA_UPDATED | ");
    }
    if (mask & EVENT_MODULE_UNREGISTERED) {
        printf("EVENT_MODULE_UNREGISTERED | ");
    }
    if (mask & EVENT_MODULE_DELETED) {
        printf("EVENT_MODULE_DELETED | ");
    }
    if (mask & EVENT_TOPOLOGY_DISCOVERY_REQUESTED) {
        printf("EVENT_TOPOLOGY_DISCOVERY_REQUESTED | ");
    }
    if (mask & EVENT_TOPOLOGY_DISCOVERED) {
        printf("EVENT_TOPOLOGY_DISCOVERED | ");
    }

    printf("\n");
}
*/


static int
_send_notification(sr_session_ctx_t *session, uint32_t event_id, char *entry_id, char *msg) 
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

    int rc = sr_event_notif_send(session, "/control-tsn-uni:notif-generic", notif_values, 3);
    if (rc != SR_ERR_OK) {
        printf("[SYSREPO] Failure while sending notification for event id %d!\n", event_id);
    }

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;   
}

static char *
_extract_key(char *xpath, char *search_key) 
{
    char *ptr = NULL;
    char *search_term = NULL;

    search_term = malloc(strlen(search_key) + 2);
    sprintf(search_term, "%s=", search_key);
    ptr = strstr(xpath, search_term);
    free(search_term);

    if (ptr) {
        ptr = strtok(ptr, "'");
        ptr = strtok(NULL, "'");
    }

    return ptr;
}

static int
_save_to_startup(sr_session_ctx_t *session)
{
    // Switch to startup datastore
    int rc = sr_session_switch_ds(session, SR_DS_STARTUP);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Copy configuration from running to startup
    rc = sr_copy_config(session, "control-tsn-uni", SR_DS_RUNNING, 0, 1);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

    // Switch back to running
    // (TODO: is this even necessary?)
    rc = sr_session_switch_ds(session, SR_DS_RUNNING);
    if (rc != SR_ERR_OK) {
        goto cleanup;
    }

cleanup:
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

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
    sr_val_t *old_val = NULL;
    sr_val_t *new_val = NULL;
    sr_change_iter_t *iter = NULL;
    sr_change_oper_t op;

    uint32_t occured_mask = 0;
    uint32_t already_send_mask = 0;

    //TSN_Event_CB_Data notif_data;
    //char *notif_data_event_name = NULL;

    rc = sr_get_changes_iter(session, "//.", &iter);
    if (rc != SR_ERR_OK) {
        printf("[PLUGIN] Failed to get changes!\n");
        char err_code[2];
        sprintf(err_code, "%d", rc);
        _send_notification(session, EVENT_ERROR, err_code, "[PLUGIN] Failed to get changes!");
        goto cleanup;
    }

    // Monitor changes and send corresponding notifications
    
    while (sr_get_change_next(session, iter, &op, &old_val, &new_val) == SR_ERR_OK) {

        val = new_val ? new_val : old_val;

        // For each occured change associated with an event
        // we add this event to the mask.
        // We do not send the notification immediately 
        // because e.g. a added stream configuration to a stream
        // will result in multiple xpaths in this while loop

        // STREAM ---------------------------------------------------
        if (strstr(val->xpath, "/streams/") != NULL) {
            // Deleted
            if ((strstr(val->xpath, "/stream-id") != NULL) &&
                (op == SR_OP_DELETED)) {   
                if ((already_send_mask & EVENT_STREAM_DELETED) == 0) {
                    char *key = _extract_key(val->xpath, "stream-id");
                    rc = _send_notification(session, EVENT_STREAM_DELETED, key, "stream deleted");
                    if (rc == EXIT_FAILURE) {
                        printf("[PLUGIN] Failed to send notification 'EVENT_STREAM_DELETED'!\n");
                    } else {
                        already_send_mask |= EVENT_STREAM_DELETED;
                    }
                    
                }
            }

            // Requested
            if ((strstr(val->xpath, "/request/") != NULL) &&
                (op == SR_OP_CREATED)) {       
                if ((already_send_mask & EVENT_STREAM_REQUESTED) == 0) {
                    char *key = _extract_key(val->xpath, "stream-id");
                    rc = _send_notification(session, EVENT_STREAM_REQUESTED, key, "stream request added");
                    if (rc == EXIT_FAILURE) {
                        printf("[PLUGIN] Failed to send notification 'EVENT_STREAM_REQUESTED'!\n");
                    } else {
                        already_send_mask |= EVENT_STREAM_REQUESTED;
                    }
                    
                }
            }

            // Configured
            if ((!strcmp(val->xpath + strlen(val->xpath) - strlen("/configured"), "/configured")) 
                && (op == SR_OP_MODIFIED || op == SR_OP_CREATED) && (val->data.bool_val == 1)) {

                if ((already_send_mask & EVENT_STREAM_CONFIGURED) == 0) {
                    char *key = _extract_key(val->xpath, "stream-id");
                    rc = _send_notification(session, EVENT_STREAM_CONFIGURED, key, "stream configuration completed");
                    if (rc == EXIT_FAILURE) {
                        printf("[PLUGIN] Failed to send notification 'EVENT_STREAM_CONFIGURED'!\n");
                    } else {
                        already_send_mask |= EVENT_STREAM_CONFIGURED;
                    }
                    
                }
            }

            // Modified (currently not used)


        // MODULES ---------------------------------------------------
        } else if (strstr(val->xpath, "/modules/") != NULL) {
            // Added
            if ((strstr(val->xpath, "/id") != NULL)
                && (op = SR_OP_CREATED)) {

                if ((already_send_mask & EVENT_MODULE_ADDED) == 0) {
                    char *key = _extract_key(val->xpath, "id");
                    rc = _send_notification(session, EVENT_MODULE_ADDED, key, "module added");
                    if (rc == EXIT_FAILURE) {
                        printf("[PLUGIN] Failed to send notification 'EVENT_MODULE_ADDED'!\n");
                    } else {
                        already_send_mask |= EVENT_MODULE_ADDED;
                    }
                }
            }

            // Registered / Unregistered
            // Check if /registered is at the end of the xpath
            if ((!strcmp(val->xpath + strlen(val->xpath) - strlen("/registered"), "/registered")) 
                && (op == SR_OP_MODIFIED)) {
                
                char *key = _extract_key(val->xpath, "id");

                uint8_t registered = val->data.uint8_val;
                rc = _send_notification(session, registered ? EVENT_MODULE_REGISTERED : EVENT_MODULE_UNREGISTERED, key, registered ? "module registered" : "module unregistered");
                if (rc == EXIT_FAILURE) {
                    printf("[PLUGIN] Failed to send notification '%s'!\n", (registered ? "EVENT_MODULE_REGISTERED" : "EVENT_MODULE_UNREGISTERED"));
                } else {
                    already_send_mask |= (registered ? EVENT_MODULE_REGISTERED : EVENT_MODULE_UNREGISTERED);
                }

            } 

            // Data updated
            if ((strstr(val->xpath, "/data/entry") != NULL) 
                && ((op == SR_OP_CREATED) || (op == SR_OP_MODIFIED))) {
                if ((already_send_mask & EVENT_MODULE_DATA_UPDATED) == 0) {
                    char *key = _extract_key(val->xpath, "id");
                    rc = _send_notification(session, EVENT_MODULE_DATA_UPDATED, key, "module data updated");
                    if (rc == EXIT_FAILURE) {
                        printf("[PLUGIN] Failed to send notification 'EVENT_MODULE_DATA_UPDATED'!\n");
                    } else {
                        already_send_mask |= EVENT_MODULE_DATA_UPDATED;
                    }
                    
                }
            }

            // Deleted
            if ((strstr(val->xpath, "/id") != NULL) 
                && (op == SR_OP_DELETED)) {
                if ((already_send_mask & EVENT_MODULE_DELETED) == 0) {
                    char *key = _extract_key(val->xpath, "id");
                    rc = _send_notification(session, EVENT_MODULE_DELETED, key, "module deleted");
                    if (rc == EXIT_FAILURE) {
                        printf("[PLUGIN] Failed to send notification 'EVENT_MODULE_DELETED'!\n");
                    } else {
                        already_send_mask |= EVENT_MODULE_DELETED;
                    }
                    
                }
            }
        

        // TOPOLOGY ---------------------------------------------------
        } else if (strstr(val->xpath, "/topology/") != NULL) {
            // Discovery requested is send directly and not reflected in changes in the datastore 

            // Discovered
            if ((already_send_mask & EVENT_TOPOLOGY_DISCOVERED) == 0) {
                rc = _send_notification(session, EVENT_TOPOLOGY_DISCOVERED, NULL, "topology discovered");
                already_send_mask |= EVENT_TOPOLOGY_DISCOVERED;
                if (rc == EXIT_FAILURE) {
                    printf("[PLUGIN] Failed to send notification 'EVENT_TOPOLOGY_DISCOVERED'!\n");
                } else {
                    already_send_mask |= EVENT_TOPOLOGY_DISCOVERED;
                }
                
                //notif_data.event_id = EVENT_TOPOLOGY_DISCOVERED;
                //notif_data.entry_id = NULL;
                //notif_data.msg = NULL;
                //notif_data_event_name = strdup("EVENT_TOPOLOGY_DISCOVERED");
                //already_send_mask |= EVENT_TOPOLOGY_DISCOVERED;
                
            }

        // APPLICATION ---------------------------------------------------
        } else if (strstr(val->xpath, "/application/") != NULL) {
            // EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED   (directly from sysrepo)
            // EVENT_APPLICATION_LIST_OF_APPS_REQUESTED     (currently not used)
            // EVENT_APPLICATION_APP_START_REQUESTED        (directly from sysrepo)
            // EVENT_APPLICATION_APP_STOP_REQUESTED         (directly from sysrepo)
        }
    }

    //rc = _send_notification(session, notif_data.event_id, notif_data.entry_id, notif_data.msg);
    //if (rc == EXIT_FAILURE) {
    //    printf("[PLUGIN] Failed to send notification '%s'!\n", notif_data_event_name);
    //}
    


    
    

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
    sr_val_t *val_plugin_running = NULL;

    (void) private_data;

    /*
    // Check if the plugin is already running
    rc = sr_get_item(session, "/control-tsn-uni:tsn-uni/plugin-running", 0, &val_plugin_running);
    if (rc != SR_ERR_OK) {
        goto error;
    }
    
    if (!val_plugin_running->data.bool_val) {
        // If not running then set flag to true
        sr_val_t val_running;
        val_running.type = SR_BOOL_T;
        val_running.data.bool_val = 1;
        rc = sr_set_item(session, "/control-tsn-uni:tsn-uni/plugin-running", &val_running, 0);
        if (rc != SR_ERR_OK) {
            goto error;
        }
        rc = sr_apply_changes(session, 0, 1);
        if (rc != SR_ERR_OK) {
            goto error;
        }

    } else {
        printf("[PLUGIN] A instance of the plugin is already running!\n");
        return SR_ERR_OPERATION_FAILED;
    }
    */
    

    // Subscribe for module changes (also causes startup data to be copied into running and enabling the module)
    rc = sr_module_change_subscribe(session, "control-tsn-uni", NULL, _module_change_cb, NULL, 0, SR_SUBSCR_DONE_ONLY, &_subscriptions);
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
    sr_free_val(val_plugin_running);
    printf("[PLUGIN] Error! Plugin couldn't be initialized: %s\n", sr_strerror(rc));
    sr_unsubscribe(_subscriptions);
    return rc;
}

int
sr_plugin_cleanup_cb(sr_session_ctx_t *session, void *private_data)
{
    (void) session;
    (void) private_data;

    int rc;

    // Reset the 'plugin-running' flag
    sr_val_t val_running;
    val_running.type = SR_BOOL_T;
    val_running.data.bool_val = 0;
    rc = sr_set_item(session, "/control-tsn-uni:tsn-uni/plugin-running", &val_running, 0);
    rc = sr_apply_changes(session, 0, 1);


    // Saving the current configuration by writing the data to the startup datastore
    rc = _save_to_startup(session);

    // Freeing the subscriptions
    rc = sr_unsubscribe(_subscriptions);
    printf("[PLUGIN] Plugin cleanup finished.\n");

    return rc;
}