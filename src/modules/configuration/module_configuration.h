/*
 * Copyright (C) 2022 Linutronix GmbH
 * Author Kurt Kanzenbach <kurt@linutronix.de>
 */

#ifndef _MODULE_CONFIGURATION_H_
#define _MODULE_CONFIGURATION_H_

#include <stdint.h>

//
// The execution parameters are fixed for all real time applications. It
// includes the cycle times, scheduling parameters and so on.
//
struct execution_parameter {
    const char *current_status;
    const char *commanded_status;
    uint64_t cycle_time;
    uint64_t base_time;
    uint64_t wakeup_latency;
    int32_t scheduling_priority;
    int32_t socket_priority;
};

//
// The following struct stores the application specific parameters.
//
struct application_parameter {
    const char **names;
    const char **values;
    size_t num_parameters;
};

struct configuration_parameter {
    const char *opcua_configuration_uri;
    const char *app_id;
    struct execution_parameter exec;
    struct application_parameter app;
};

static const uint64_t CONFIGURATION_DEFAULT_CYCLE_TIME_NS       = 1000000;
static const uint64_t CONFIGURATION_DEFAULT_BASE_TIME_NS        = 0;
static const uint64_t CONFIGURATION_DEFAULT_WAKEUP_LATENCY_NS   = 500000;
static const int      CONFIGURATION_DEFAULT_SCHEDULING_PRIORITY = 98;
static const int      CONFIGURATION_DEFAULT_SOCKET_PRIORITY     = 0;

#endif /* _MODULE_CONFIGURATION_H_ */
