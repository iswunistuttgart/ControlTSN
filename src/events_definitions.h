/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#ifndef __EVENTS_DEFINITIONS_H__
#define __EVENTS_DEFINITIONS_H__

#include <stdint.h>

// ----------------------------------------------
//      EVENTS
// ----------------------------------------------
#define EVENT_ERROR                 0x00000001

// Stream related
#define EVENT_STREAM_REQUESTED              0x00000002
#define EVENT_STREAM_CONFIGURED             0x00000004
#define EVENT_STREAM_DELETED                0x00000008
#define EVENT_STREAM_MODIFIED               0x00000010
#define EVENT_STREAM_COMPUTATION_REQUESTED  0x00000020
#define EVENT_STREAM_DEPLOY                 0x00000040
#define EVENT_STREAM_DEPLOYED               0x00000080

// Modules related
#define EVENT_MODULE_ADDED          0x00000100
#define EVENT_MODULE_REGISTERED     0x00000200
#define EVENT_MODULE_DATA_UPDATED   0x00000400
#define EVENT_MODULE_UNREGISTERED   0x00000800
#define EVENT_MODULE_DELETED        0x00001000
//#define ...                       0x00002000
//#define ...                       0x00004000
#define EVENT_MODULE_EXPORT_DATA    0x00008000

// Topology related
#define EVENT_TOPOLOGY_DISCOVERY_REQUESTED  0x00010000
#define EVENT_TOPOLOGY_DISCOVERED           0x00020000
//#define ...                               0x00040000
//#define ...                               0x00080000
//#define ...                               0x00100000
//#define ...                               0x00200000
//#define ...                               0x00400000
//#define ...                               0x00800000

// Application related
#define EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED	0x01000000  // 16777216
#define EVENT_APPLICATION_LIST_OF_APPS_REQUESTED	0x02000000  // 33554432
#define EVENT_APPLICATION_APP_START_REQUESTED		0x04000000  // 67108864
#define EVENT_APPLICATION_APP_STOP_REQUESTED		0x08000000  // 134217728

// Configuration related
#define EVENT_CONFIGURATION_DEPLOY	                    0x10000000  // 268435456
#define EVENT_CONFIGURATION_CHANGED	                    0x20000000  // 536870912
//#define EVENT_CONFIGURATION_REQUEST_RUN_STATE           0x40000000  // 1073741824
//#define EVENT_CONFIGURATION_TOGGLE_APP_SEND_RECEIVE     0x80000000  // 2147483648
#define EVENT_CONFIGURATION_ENABLE_STREAM_SEND_RECEIVE  0x40000000  // 1073741824
#define EVENT_CONFIGURATION_DISABLE_STREAM_SEND_RECEIVE 0x80000000  // 2147483648


typedef struct TSN_Event_CB_Data {
    //uint32_t event_id;  // The ID of the occured event
    uint64_t event_id;  // The ID of the occured event
    char *entry_id;     // Might contain the sysrepo key of the corresponding element (e.g. the id)
    char *msg;          // Might contain a event specific message
} TSN_Event_CB_Data;


#endif // __EVENTS_DEFINITIONS_H__
