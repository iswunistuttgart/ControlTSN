/*
 * Copyright (C) 2022 Linutronix GmbH
 * Author Kurt Kanzenbach <kurt@linutronix.de>
 */

#ifndef _MODULE_CONFIGURATION_H_
#define _MODULE_CONFIGURATION_H_

#include <stdint.h>

/* FIXME: Adapt to correct vPLC information model */
struct vplc_parameter {
    int32_t axes;
    double speed;
    double length;
    const char *current_status;
    const char *commanded_status;
};

struct execution_parameter {
    uint64_t cycle_time;
    uint64_t base_time;
    uint64_t wakeup_latency;
    int32_t scheduling_priority;
    int32_t socket_priority;
};

struct configuration_parameter {
    const char *opcua_configuration_uri;
    const char *app_id;
    struct execution_parameter exec;
    struct vplc_parameter vplc;
};

static const uint64_t CONFIGURATION_DEFAULT_CYCLE_TIME_NS       = 1000000;
static const uint64_t CONFIGURATION_DEFAULT_BASE_TIME_NS        = 0;
static const uint64_t CONFIGURATION_DEFAULT_WAKEUP_LATENCY_NS   = 500000;
static const int      CONFIGURATION_DEFAULT_SCHEDULING_PRIORITY = 1;
static const int      CONFIGURATION_DEFAULT_SOCKET_PRIORITY     = 0;

#endif /* _MODULE_CONFIGURATION_H_ */
