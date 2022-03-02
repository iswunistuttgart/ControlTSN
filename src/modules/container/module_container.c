/*
 * Copyright (C) 2022 Linutronix GmbH
 * Author Kurt Kanzenbach <kurt@linutronix.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <ulfius.h>

#include "../../common.h"
#include "../../events_definitions.h"
#include "../../helper/json_serializer.h"
#include "module_container.h"

//--------------------------------------
// Global Module Parameter
//   Defined in src/sysrepo/initial2.xml
//--------------------------------------
static const char *registry_url;

//--------------------------------------
// API specific REST endpoints
//--------------------------------------
static const char *API_DOCKER_LIST_IMAGES = "v2/_catalog";

//--------------------------------------
// Container Module Utility functions
//--------------------------------------
static volatile sig_atomic_t is_running = 1;

static void signal_handler(int signum)
{
    is_running = 0;
}

#define log(...)                                                        \
    do {                                                                \
        container_log(__func__, __VA_ARGS__);                           \
    } while (0)

static void container_log(const char * restrict func, const char * restrict fmt, ...)
{
    FILE *out = stdout;
    va_list args;

    va_start(args, fmt);

    fprintf(out, "[%s][%s]: ", "Container", func);
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    fflush(out);

    va_end(args);
}

//--------------------------------------
// Request list of Images from Registry
//--------------------------------------
static void container_discover_images(void)
{
    struct _u_response response;
    struct _u_request request;
    char url[1024] = { };
    json_t *json_body;
    int ret;

    if (!registry_url) {
        log("No container registry URL specified!");
        return;
    }

    // Make HTTP Request to the Docker registry
    ulfius_init_request(&request);
    ulfius_init_response(&response);

    snprintf(url, sizeof(url) - 1, "%s/%s", registry_url, API_DOCKER_LIST_IMAGES);
    request.http_url  = strdup(url);
    request.http_verb = strdup("GET");
    if (!request.http_url || !request.http_verb)
        goto out;

    ret = ulfius_send_http_request(&request, &response);
    if (ret != U_OK) {
        log("Failure sending request to Docker Registry at '%s'", url);
        goto out;
    }

    // Get JSON body containing the "repositories" aka Images
    json_body = ulfius_get_json_body_response(&response, NULL);

    // Deserialize the images
    TSN_Images *discovered_images = deserialize_images(json_body);
    if (!discovered_images) {
        log("Failed to deserialize images!");
        goto out;
    }

    // Write the images to sysrepo
    ret = sysrepo_set_application_images(discovered_images);
    if (ret != EXIT_SUCCESS)
        log("Error writing list of images to the datastore!");

    free(discovered_images);

out:
    ulfius_clean_response(&response);
    ulfius_clean_request(&request);
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void _cb_event(TSN_Event_CB_Data data)
{
    const char *event_name = NULL;

    if (data.event_id & EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED) {
        event_name = "EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED";
        container_discover_images();
    } else if (data.event_id & EVENT_APPLICATION_LIST_OF_APPS_REQUESTED) {
        event_name = "EVENT_APPLICATION_LIST_OF_APPS_REQUESTED";
    } else if (data.event_id & EVENT_APPLICATION_APP_START_REQUESTED) {
        event_name = "EVENT_APPLICATION_APP_START_REQUESTED";
    } else if (data.event_id & EVENT_APPLICATION_APP_STOP_REQUESTED) {
        event_name = "EVENT_APPLICATION_APP_STOP_REQUESTED";
    }

    log("Event '%s' (%s, %s)", event_name, data.entry_id, data.msg);

    return;
}


// ------------------------------------
// MAIN
// ------------------------------------
int main(void)
{
    TSN_Module *this_module = NULL;
    int rc;

    // Signal handling
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);

    // Init this module
    this_module = malloc(sizeof(TSN_Module));
    if (!this_module)
        return EXIT_FAILURE;

    rc = module_init("Container", &this_module,
                     (EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED |
                      EVENT_APPLICATION_LIST_OF_APPS_REQUESTED |
                      EVENT_APPLICATION_APP_START_REQUESTED |
                      EVENT_APPLICATION_APP_STOP_REQUESTED),
                     _cb_event);
    if (rc == EXIT_FAILURE) {
        log("Error initializing module!");
        goto cleanup;
    }

    // Get saved module data
    TSN_Module_Data *module_data = malloc(sizeof(TSN_Module_Data));
    if (!module_data)
        goto cleanup;

    rc = module_get_data(this_module->id, &module_data);
    if (rc != EXIT_SUCCESS)
        goto cleanup;

    // Find Container registry url
    TSN_Module_Data_Entry *reg_entry = module_get_data_entry(module_data, MODULE_DATA_IDENTIFIER_REGISTRY);
    if (reg_entry) {
        registry_url = strdup(reg_entry->value.string_val);
        if (!registry_url)
            goto cleanup;
        log("Container registry URL: '%s'", registry_url);
    }

    log("Container module successfully started and running");

    // Keep running
    while (is_running)
        sleep(1);

    log("Stopping the module...");

cleanup:
    module_shutdown();
    free(this_module);
    return rc;
}
