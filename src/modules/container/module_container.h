/*
 * Copyright (C) 2022 Linutronix GmbH
 * Author Kurt Kanzenbach <kurt@linutronix.de>
 */

#ifndef _MODULE_CONTAINER_H_
#define _MODULE_CONTAINER_H_

//------------------------------------
// Module Data Identifiers
//------------------------------------
static const char *MODULE_DATA_IDENTIFIER_REGISTRY = "registry_url";
static const char *MODULE_DATA_IDENTIFIER_KUBERNETES = "kubernetes_url";

//------------------------------------
// Application parameter structure
//------------------------------------
struct application_parameter {
    const char *name;              /**< Name of the application/pod */
    const char *application_image; /**< Docker/podman container image */
    const char *command;           /**< Command/binary to be executed e.g., cyclictest */
    const char *command_line;      /**< Command line arguments for the binary */
    const char *node_selector;     /**< Optional: Hint for Kubernetes scheduler where to put (host/node) the application */
    const char *capabilities;      /**< Optional: Capabilities for container which might be required for RT applications.
                                                  If not specified, default RT capabilities will be assigned. */
    int resource_cpus;             /**< Optional: Amount of requested CPU cores. Default: 1. */
    int resource_ram_mb;           /**< Optional: Amount of requested RAM in MiB. Default: 512 MiB. */
    bool privileged;               /**< Optional: Specify privileged for container instead of using @capabilities. */
};

static const char *APPLICATION_DEFAULT_CAPABILITIES = "\"SYS_NICE\", \"NET_ADMIN\", \"SYS_TIME\", \"SYS_RAWIO\", \"IPC_LOCK\"";
static const int APPLICATION_DEFAULT_RESOURCE_CPUS = 1;
static const int APPLICATION_DEFAULT_RESOURCE_RAM_MB = 512;

#endif /* _MODULE_CONTAINER_H_ */
