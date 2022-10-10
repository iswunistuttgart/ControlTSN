/*
 * Copyright (C) 2022 Linutronix GmbH
 * Author Kurt Kanzenbach <kurt@linutronix.de>
 */

#ifndef _MODULE_CONFIGURATION_H_
#define _MODULE_CONFIGURATION_H_

#include <stdint.h>
#include <stdbool.h>

//
// The engineering parameters are fixed for all real time applications. It
// includes the cycle times, scheduling parameters and so on.
//
// Keep in sync with [endpoint]/nodeset/tsnendpointmodel.xml
//
struct engineering_parameter {
    const char *current_status;
    const char *commanded_status;
    const char *destination_mac;
    const char *subscribed_mac;
    const char *interface;
    bool sendreceive_enabled;
    uint64_t cycle_time;
    uint64_t base_time;
    int32_t qbv_offset;
    uint64_t wakeup_latency;
    uint32_t wcet;
    int32_t scheduling_priority;
    int32_t socket_priority;
};

//
// The following struct stores the application specific parameters.
//
struct application_parameter {
    const char **names;
    const char **values;
    const char **types;
    size_t num_parameters;
};

struct configuration_parameter {
    const char *opcua_configuration_uri;
    const char *app_id;
    struct engineering_parameter eng;
    struct application_parameter app;
};

static const bool     CONFIGURATION_DEFAULT_SENDRECEIVE_ENABLED = true;
static const uint64_t CONFIGURATION_DEFAULT_CYCLE_TIME_NS       = 1000000;
static const uint64_t CONFIGURATION_DEFAULT_BASE_TIME_NS        = 0;
static const int32_t  CONFIGURATION_DEFAULT_QBV_OFFSET_NS       = 0;
static const uint64_t CONFIGURATION_DEFAULT_WAKEUP_LATENCY_NS   = 500000;
static const uint32_t CONFIGURATION_DEFAULT_WCET                = 0;
static const int32_t  CONFIGURATION_DEFAULT_SCHEDULING_PRIORITY = 98;
static const int32_t  CONFIGURATION_DEFAULT_SOCKET_PRIORITY     = 0;

#endif /* _MODULE_CONFIGURATION_H_ */
