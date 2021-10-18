#ifndef __EVENTS_DEFINITIONS_H__
#define __EVENTS_DEFINITIONS_H__

// ----------------------------------------------
//      EVENTS
// ----------------------------------------------
#define EVENT_ERROR                 0x00000001

// Stream related   
#define EVENT_STREAM_REQUESTED      0x00000002
#define EVENT_STREAM_CONFIGURED     0x00000004
#define EVENT_STREAM_DELETED        0x00000008
//#define ...                       0x00000010
//#define ...                       0x00000020
//#define ...                       0x00000040
//#define ...                       0x00000080

// Modules related  
#define EVENT_MODULE_ADDED          0x00000100
#define EVENT_MODULE_REGISTERED     0x00000200
#define EVENT_MODULE_DATA_UPDATED   0x00000400
#define EVENT_MODULE_UNREGISTERED   0x00000800
#define EVENT_MODULE_DELETED        0x00001000
//#define ...                       0x00002000
//#define ...                       0x00004000
//#define ...                       0x00008000

// Topology related   
#define EVENT_TOPOLOGY_DISCOVERY_REQUESTED  0x00010000
#define EVENT_TOPOLOGY_DISCOVERED           0x00020000
//#define ...                               0x00040000
//#define ...                               0x00080000
//#define ...                               0x00100000
//#define ...                               0x00200000
//#define ...                               0x00400000
//#define ...                               0x00800000


// ----------------------------------------------
// Error
// ----------------------------------------------
/**
 * @brief Data for the event of an occured error
 */
typedef struct TSN_Event_Data_Error
{
    int error_code;
    char *error_msg;
} TSN_Event_Data_Error;

// ----------------------------------------------
// Streams
// ----------------------------------------------
/**
 * @brief Data for the event of a requested stream
 */
typedef struct TSN_Event_Data_Stream_Requested
{
    char *stream_id;
} TSN_Event_Data_Stream_Requested;

/**
 * @brief Data for the event of a configured stream
 */
typedef struct TSN_Event_Data_Stream_Configured
{
    char *stream_id;
} TSN_Event_Data_Stream_Configured;

// ----------------------------------------------
// Modules
// ----------------------------------------------

// ----------------------------------------------
// Topology
// ----------------------------------------------

// ----------------------------------------------
// Application
// ----------------------------------------------

/**
 * @brief Union containing the data for the various events
 */
/*
typedef union TSN_Event_CB_Data
{
    // Error
    TSN_Event_Data_Error error;
    // Stream
    TSN_Event_Data_Stream_Requested stream_requested;
    TSN_Event_Data_Stream_Configured stream_configured;
    // Modules
    // Topology
    // Application
} TSN_Event_CB_Data;
*/

typedef struct TSN_Event_CB_Data {
    uint32_t event_id;
    char *entry_id;
    char *msg;
} TSN_Event_CB_Data;


#endif // __EVENTS_DEFINITIONS_H__