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
static const char *kubernetes_url;

//--------------------------------------
// API specific REST endpoints
//--------------------------------------
static const char *API_DOCKER_LIST_IMAGES = "v2/_catalog";
static const char *API_KUBERNETES_PODS = "api/v1/namespaces/default/pods";

//--------------------------------------
// Container Module Utility functions
//--------------------------------------
static volatile sig_atomic_t is_running = 1;

static void signal_handler(int signum)
{
    (void)signum;
    is_running = 0;
}

static const char *container_docker_host(void)
{
    if (!strncmp(registry_url, "http://", strlen("http://")))
        return registry_url + strlen("http://");
    if (!strncmp(registry_url, "https://", strlen("https://")))
        return registry_url + strlen("https://");

    return NULL;
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

static char *x_strdup(const char *str)
{
    char *p = strdup(str);
    if (!p) {
        log("Error allocating memory!\n");
        exit(EXIT_FAILURE);
    }

    return p;
}

static void container_init_app_param(struct application_parameter *parameter)
{
    memset(parameter, '\0', sizeof(*parameter));
    parameter->capabilities    = x_strdup(APPLICATION_DEFAULT_CAPABILITIES);
    parameter->resource_cpus   = APPLICATION_DEFAULT_RESOURCE_CPUS;
    parameter->resource_ram_mb = APPLICATION_DEFAULT_RESOURCE_RAM_MB;
}

static void container_fill_app_param(struct application_parameter *parameter,
                                     const TSN_App *app)
{
    char *cpus = NULL, *ram = NULL;
    int i;

    parameter->name = app->name;
    parameter->application_image = app->image_ref;

    for (i = 0; i < app->count_parameters; ++i) {
        TSN_App_Parameter *par = &app->parameters[i];

        if (!strcmp(par->name, "command"))
            parameter->command = parameter_data_value_to_string(par);

        if (!strcmp(par->name, "command_line"))
            parameter->command_line = parameter_data_value_to_string(par);

        if (!strcmp(par->name, "node"))
            parameter->node_selector = parameter_data_value_to_string(par);

        if (!strcmp(par->name, "capabilities"))
            parameter->capabilities = parameter_data_value_to_string(par);

        if (!strcmp(par->name, "cpus")) {
            cpus = parameter_data_value_to_string(par);
            parameter->resource_cpus = atoi(cpus);
            if (parameter->resource_cpus <= 0)
                parameter->resource_cpus = APPLICATION_DEFAULT_RESOURCE_CPUS;
            free(cpus);
        }

        if (!strcmp(par->name, "ram")) {
            ram = parameter_data_value_to_string(par);
            parameter->resource_ram_mb = atoi(ram);
            if (parameter->resource_ram_mb <= 0)
                parameter->resource_ram_mb = APPLICATION_DEFAULT_RESOURCE_RAM_MB;
            free(ram);
        }
    }
}

static void container_find_app_node(struct application_parameter *parameter,
                                    const TSN_App *app, const TSN_Devices *devices)
{
    int i, j;

    for (i = 0; i < devices->count_enddevices; ++i) {
        TSN_Enddevice *device = &devices->enddevices[i];

        if (device->has_app) {
            for (j = 0; j < device->count_apps; ++j) {
                if (!strcmp(app->id, device->apps[j].app_ref)) {
                    parameter->node_selector = device->mac;
                    break;
                }
            }
        }
    }
}

static void container_free_app_param(struct application_parameter *parameter)
{
    if (!parameter)
        return;

    free((void *)parameter->command);
    free((void *)parameter->command_line);
    free((void *)parameter->node_selector);
    free((void *)parameter->capabilities);
}

//--------------------------------------
// Request list of Images from Registry
//--------------------------------------
static void container_discover_images(void)
{
    struct _u_response response;
    struct _u_request request;
    char url[1024] = { 0 };
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
    TSN_Images *discovered_images = deserialize_images(json_body, container_docker_host());
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

//------------------------------------------
// Request list of Apps/Pods from Kubernetes
//------------------------------------------
static void container_discover_apps(void)
{
    struct _u_response response;
    struct _u_request request;
    char url[1024] = { 0 };
    json_t *json_body;
    int ret;

    if (!kubernetes_url) {
        log("No kubernetes URL specified!");
        return;
    }

    // Make HTTP Request to the Kubernetes cluster
    ulfius_init_request(&request);
    ulfius_init_response(&response);

    snprintf(url, sizeof(url) - 1, "%s/%s", kubernetes_url, API_KUBERNETES_PODS);
    request.http_url  = strdup(url);
    request.http_verb = strdup("GET");
    if (!request.http_url || !request.http_verb)
        goto out;

    ret = ulfius_send_http_request(&request, &response);
    if (ret != U_OK) {
        log("Failure sending request to Kubernetes Cluster at '%s'", url);
        goto out;
    }

    // Get JSON body containing the "items" aka Pods/Applications
    json_body = ulfius_get_json_body_response(&response, NULL);

    // Deserialize the apps
    TSN_Apps *discovered_apps = deserialize_apps(json_body);
    if (!discovered_apps) {
        log("Failed to deserialize images!");
        goto out;
    }

    // Write the apps to sysrepo
    ret = sysrepo_set_application_apps(discovered_apps);
    if (ret != EXIT_SUCCESS)
        log("Error writing list of images to the datastore!");

    free(discovered_apps);

out:
    ulfius_clean_response(&response);
    ulfius_clean_request(&request);
}

//------------------------------------------
// Start App/Pod in Kubernetes Cluster
//------------------------------------------
static void container_start_app(const struct application_parameter *parameter)
{
    struct _u_response response;
    struct _u_request request;
    struct _u_map req_headers;
    char pod[4096] = { 0 };
    char url[1024] = { 0 };
    int ret, len;

    if (!kubernetes_url) {
        log("No kubernetes URL specified!");
        return;
    }

    if (!parameter) {
        log("No application/pod parameter specified!");
        return;
    }

    // Build Kubernetes pod description
    len = snprintf(pod, sizeof(pod) - 1,
                   "apiVersion: v1\n"
                   "kind: Pod\n"
                   "metadata:\n"
                   "  name: %s\n"
                   "spec:\n"
                   "  hostIPC: true\n"
                   "  containers:\n"
                   "  - name: rt-container\n"
                   "    image: %s\n"
                   "    imagePullPolicy: IfNotPresent\n"
                   "    command: [ \"%s\" ]\n"
                   "    args: [ %s ]\n"
                   "    securityContext:\n"
                   "      capabilities:\n"
                   "        add: [ %s ]\n"
                   "    resources:\n"
                   "      requests:\n"
                   "        cpu: %d\n"
                   "        memory: \"%dM\"\n"
                   "      limits:\n"
                   "        cpu: %d\n"
                   "        memory: \"%dM\"\n"
                   "    volumeMounts:\n"
                   "    - mountPath: /etc/sysrepo\n"
                   "      name: sysrepo-volume\n"
                   "  volumes:\n"
                   "  - name: sysrepo-volume\n"
                   "    hostPath:\n"
                   "      path: /etc/sysrepo\n"
                   "      type: DirectoryOrCreate\n",
                   parameter->name,
                   parameter->application_image,
                   parameter->command,
                   parameter->command_line,
                   parameter->capabilities,
                   parameter->resource_cpus,
                   parameter->resource_ram_mb,
                   parameter->resource_cpus,
                   parameter->resource_ram_mb);

    if (parameter->node_selector)
        len = snprintf(pod + len, sizeof(pod) - len - 1,
                       "  nodeSelector:\n"
                       "    mac: %s\n",
                       parameter->node_selector);

    // Make HTTP Request to the Kubernetes cluster
    ulfius_init_request(&request);
    ulfius_init_response(&response);

    snprintf(url, sizeof(url) - 1, "%s/%s", kubernetes_url, API_KUBERNETES_PODS);

    u_map_init(&req_headers);
    u_map_put(&req_headers, "Content-Type", "application/yaml");
    request.http_verb = strdup("POST");
    request.http_url = strdup(url);
    u_map_copy_into(request.map_header, &req_headers);
    request.binary_body = strdup(pod);
    request.binary_body_length = strlen(pod);

    if (!request.http_verb || !request.http_url || !request.binary_body)
        goto out;

    ret = ulfius_send_http_request(&request, &response);
    if (ret != U_OK) {
        log("Failure sending request to Kubernetes Cluster at '%s'", url);
        goto out;
    }

    if (response.status != 200 && response.status != 201)
        log("Failed to create pod. Response code=%ld", response.status);

out:
    ulfius_clean_response(&response);
    ulfius_clean_request(&request);
}

//------------------------------------------
// Start App/Pod in Kubernetes Cluster
//------------------------------------------
static void container_stop_app(const struct application_parameter *parameter)
{
    struct _u_response response;
    struct _u_request request;
    char url[1024] = { 0 };
    int ret;

    if (!kubernetes_url) {
        log("No kubernetes URL specified!");
        return;
    }

    if (!parameter) {
        log("No application/pod parameter specified!");
        return;
    }

    // Make HTTP Request to the Kubernetes cluster
    ulfius_init_request(&request);
    ulfius_init_response(&response);

    snprintf(url, sizeof(url) - 1, "%s/%s/%s", kubernetes_url,
             API_KUBERNETES_PODS, parameter->name);

    request.http_verb = strdup("DELETE");
    request.http_url = strdup(url);

    if (!request.http_verb || !request.http_url)
        goto out;

    ret = ulfius_send_http_request(&request, &response);
    if (ret != U_OK) {
        log("Failure sending request to Kubernetes Cluster at '%s'", url);
        goto out;
    }

    if (response.status != 200 && response.status != 201)
        log("Failed to delete pod. Response code=%ld", response.status);

out:
    ulfius_clean_response(&response);
    ulfius_clean_request(&request);
}

// ------------------------------------
// Callback handler
// ------------------------------------
static void _cb_event(TSN_Event_CB_Data data)
{
    struct application_parameter param;
    const char *event_name = NULL;
    TSN_Devices *devices = NULL;
    TSN_App *app = NULL;

    // Initialize with default values
    container_init_app_param(&param);

    if (data.event_id & EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED) {
        event_name = "EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED";
        container_discover_images();
    } else if (data.event_id & EVENT_APPLICATION_LIST_OF_APPS_REQUESTED) {
        event_name = "EVENT_APPLICATION_LIST_OF_APPS_REQUESTED";
        container_discover_apps();
    } else if (data.event_id & EVENT_APPLICATION_APP_START_REQUESTED) {
        int ret;

        event_name = "EVENT_APPLICATION_APP_START_REQUESTED";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto cleanup;

        // Enrich application parameters from sysrepo data
        container_fill_app_param(&param, app);

        // Get enddevices
        devices = malloc(sizeof(TSN_Devices));
        if (!devices)
            goto cleanup;

        ret = topology_get_devices(&devices);
        if (ret != EXIT_SUCCESS)
            goto cleanup;

        container_find_app_node(&param, app, devices);

        container_start_app(&param);
    } else if (data.event_id & EVENT_APPLICATION_APP_STOP_REQUESTED) {
        int ret;

        event_name = "EVENT_APPLICATION_APP_STOP_REQUESTED";

        // Get app from sysrepo
        ret = sysrepo_get_application_app(data.msg, &app);
        if (ret != EXIT_SUCCESS)
            goto cleanup;

        // Enrich application parameters from sysrepo data
        container_fill_app_param(&param, app);

        container_stop_app(&param);
    }

    log("Event '%s' (%s, %s)", event_name, data.entry_id, data.msg);

cleanup:
    container_free_app_param(&param);
    application_app_put(app);
    topology_put_devices(devices);
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

    // Find Kubernetes url
    TSN_Module_Data_Entry *kub_entry = module_get_data_entry(module_data, MODULE_DATA_IDENTIFIER_KUBERNETES);
    if (kub_entry) {
        kubernetes_url = strdup(kub_entry->value.string_val);
        if (!kubernetes_url)
            goto cleanup;
        log("Kubernetes URL: '%s'", kubernetes_url);
    }

    // Start kubectl proxy for REST communication
    rc = system("kubectl proxy &");
    if (rc) {
        log("Failed to launch kubectl proxy");
        goto cleanup;
    }

    log("Container module successfully started and running");

    // Keep running
    while (is_running)
        sleep(1);

    log("Stopping the module...");

cleanup:
    module_shutdown();
    return rc;
}
