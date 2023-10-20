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
#define EVENT_ERROR                                     0x0000000000000001 // 1

// Stream related
#define EVENT_STREAM_REQUESTED                          0x0000000000000002 // 2
#define EVENT_STREAM_CONFIGURED                         0x0000000000000004 // 4
#define EVENT_STREAM_DELETED                            0x0000000000000008 // 8
#define EVENT_STREAM_MODIFIED                           0x0000000000000010 // 16
#define EVENT_STREAM_COMPUTATION_REQUESTED              0x0000000000000020 // 32
#define EVENT_STREAM_DEPLOY                             0x0000000000000040 // 64
#define EVENT_STREAM_DEPLOYED                           0x0000000000000080 // 128
#define EVENT_STREAM_LISTENER_JOINED                    0x0000000000000100 // 256
#define EVENT_STREAM_LISTENER_LEFT                      0x0000000000000200 // 512
//#define ...                                           0x0000000000000400 // 1024
//#define ...                                           0x0000000000000800 // 2048

// Modules related
#define EVENT_MODULE_ADDED                              0x0000000000001000 // 4096
#define EVENT_MODULE_REGISTERED                         0x0000000000002000 // 8192
#define EVENT_MODULE_DATA_UPDATED                       0x0000000000004000 // 16384
#define EVENT_MODULE_UNREGISTERED                       0x0000000000008000 // 32768
#define EVENT_MODULE_DELETED                            0x0000000000010000 // 65536
//#define ...                                           0x0000000000020000 // 131072
//#define ...                                           0x0000000000040000 // 262144
#define EVENT_MODULE_EXPORT_DATA                        0x0000000000080000 // 524288
//#define ...                                           0x0000000000100000 // 1048576
//#define ...                                           0x0000000000200000 // 2097152
//#define ...                                           0x0000000000400000 // 4194304
//#define ...                                           0x0000000000800000 // 8388608

// Topology related
#define EVENT_TOPOLOGY_DISCOVERY_REQUESTED              0x0000000001000000 // 16777216
#define EVENT_TOPOLOGY_DISCOVERED                       0x0000000002000000 // 33554432
//#define ...                                           0x0000000004000000 // 67108864
//#define ...                                           0x0000000008000000 // 134217728
//#define ...                                           0x0000000010000000 // 268435456
//#define ...                                           0x0000000020000000 // 536870912
//#define ...                                           0x0000000040000000 // 1073741824
//#define ...                                           0x0000000080000000 // 2147483648
//#define ...                                           0x0000000100000000 // 4294967296
//#define ...                                           0x0000000200000000 // 8589934592
//#define ...                                           0x0000000400000000 // 17179869184
//#define ...                                           0x0000000800000000 // 34359738368

// Application related
#define EVENT_APPLICATION_LIST_OF_IMAGES_REQUESTED	    0x0000001000000000 // 68719476736
#define EVENT_APPLICATION_LIST_OF_APPS_REQUESTED	    0x0000002000000000 // 137438953472
#define EVENT_APPLICATION_APP_START_REQUESTED		    0x0000004000000000 // 274877906944
#define EVENT_APPLICATION_APP_STOP_REQUESTED		    0x0000008000000000 // 549755813888
//#define ...                                           0x0000010000000000 // 1099511627776
//#define ...                                           0x0000020000000000 // 2199023255552
//#define ...                                           0x0000040000000000 // 4398046511104
//#define ...                                           0x0000080000000000 // 8796093022208
//#define ...                                           0x0000100000000000 // 17592186044416
//#define ...                                           0x0000200000000000 // 35184372088832
//#define ...                                           0x0000400000000000 // 70368744177664
//#define ...                                           0x0000800000000000 // 140737488355328

// Configuration related
#define EVENT_CONFIGURATION_DEPLOY	                    0x0001000000000000 // 281474976710656
#define EVENT_CONFIGURATION_CHANGED	                    0x0002000000000000 // 562949953421312
#define EVENT_CONFIGURATION_ENABLE_STREAM_SEND_RECEIVE  0x0004000000000000 // 1125899906842624
#define EVENT_CONFIGURATION_DISABLE_STREAM_SEND_RECEIVE 0x0008000000000000 // 2251799813685248
//#define ...                                           0x0010000000000000 // 4503599627370496
//#define ...                                           0x0020000000000000 // 9007199254740992
//#define ...                                           0x0040000000000000 // 18014398509481984
//#define ...                                           0x0080000000000000 // 36028797018963970
//#define ...                                           0x0100000000000000 // 72057594037927940
//#define ...                                           0x0200000000000000 // 144115188075855870
//#define ...                                           0x0400000000000000 // 288230376151711740
//#define ...                                           0x0800000000000000 // 576460752303423500


typedef struct TSN_Event_CB_Data {
    //uint32_t event_id;  // The ID of the occured event
    uint64_t event_id;  // The ID of the occured event
    char *entry_id;     // Might contain the sysrepo key of the corresponding element (e.g. the id)
    char *msg;          // Might contain a event specific message
} TSN_Event_CB_Data;


#endif // __EVENTS_DEFINITIONS_H__
