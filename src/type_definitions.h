#ifndef __TYPE_DEFINITIONS_H__
#define __TYPE_DEFINITIONS_H__

#include <stdint.h>
#include "ieee802_tsn_types.h"
#include "sysrepo.h"

// ----------------------------------------------
// Event handling
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

/**
 * @brief Data for the event of an occured error
 */
typedef struct TSN_Event_Data_Error
{
    int error_code;
    char *error_msg;
} TSN_Event_Data_Error;

/**
 * @brief Union containing the data for the various events
 */
typedef union TSN_Event_CB_Data
{
    TSN_Event_Data_Stream_Requested stream_requested;
    TSN_Event_Data_Stream_Configured stream_configured;
    TSN_Event_Data_Error error;
} TSN_Event_CB_Data;

// ---------------------------------------
// Stream definitions
// ---------------------------------------
typedef struct TSN_Talker {
    uint16_t count_end_station_interfaces;
    uint16_t count_data_frame_specifications;

    // ieee802-dot1q-tsn-types -> group-talker
    IEEE_StreamRank stream_rank;
    IEEE_InterfaceId *end_station_interfaces;
    IEEE_DataFrameSpecification *data_frame_specification;
    IEEE_TrafficSpecification traffic_specification;
    IEEE_UserToNetworkRequirements user_to_network_requirements;
    IEEE_InterfaceCapabilities interface_capabilities;
} TSN_Talker;

typedef struct TSN_Listener {
    uint16_t index;
    uint16_t count_end_station_interfaces;

    // ieee802-dot1q-tsn-types -> group-listener
    IEEE_InterfaceId *end_station_interfaces;
    IEEE_UserToNetworkRequirements user_to_network_requirements;
    IEEE_InterfaceCapabilities interface_capabilities;
} TSN_Listener;

typedef struct TSN_StatusTalker {
    // ieee802-dot1q-tsn-types -> group-status-talker-listener
    uint32_t accumulated_latency;
    IEEE_InterfaceConfiguration interface_configuration;
} TSN_StatusTalker;

typedef struct TSN_StatusListener {
    uint16_t index;
    
    // ieee802-dot1q-tsn-types -> group-status-talker-listener
    uint32_t accumulated_latency;
    IEEE_InterfaceConfiguration interface_configuration;
} TSN_StatusListener;

typedef struct TSN_Configuration {
    uint16_t count_listeners;
    uint16_t count_failed_interfaces;

    TSN_StatusTalker talker;
    TSN_StatusListener *listener_list;

    // ieee802-dot1q-tsn-types -> group-status-stream
    IEEE_StatusInfo status_info;
    IEEE_InterfaceId *failed_interfaces;
} TSN_Configuration;

typedef struct TSN_Request {
    uint16_t count_listeners;

    TSN_Talker talker;
    TSN_Listener *listener_list;
} TSN_Request;

typedef struct TSN_Stream {
    // The unique id of the stream in the format XX-XX-XX-XX-XX-XX:YY-YY,
    // where XX is the MAC of the Talker and YY a unique number
    char *stream_id;   
    TSN_Request request;
    TSN_Configuration configuration;
} TSN_Stream;


// ---------------------------------------
// Module definitions
// ---------------------------------------
typedef struct TSN_Module_Data_Entry
{
    char *name;         // The name of the variable
    sr_type_t type;     // The type of the variable
    sr_data_t value;    // The value of the variable
} TSN_Module_Data_Entry;

typedef struct TSN_Module_Data 
{
    uint16_t count_entries;
    TSN_Module_Data_Entry *entries;
} TSN_Module_Data;


typedef struct TSN_Module
{
    uint16_t id;             // Module ID
    int p_id;           // Module Process ID
    char *path;         // Path to the modules executable       (???)
    char *name;         // Module Name
    char *description;  // Module Description
    uint16_t subscribed_events_mask;     // Mask describing the relevant events for this module
    void (*cb_event)(int event_id, TSN_Event_CB_Data data);     // Generic callback method for events
} TSN_Module;

typedef struct TSN_Modules {
    uint16_t count_available_modules;
    TSN_Module *available_modules;

    uint16_t count_registered_modules;
    TSN_Module *registered_modules;
} TSN_Modules;


// ----------------------------------------------
// Root
// ----------------------------------------------
typedef struct TSN_Uni {
    uint16_t count_streams;
    TSN_Stream *stream_list;

    TSN_Modules modules;
} TSN_Uni;


#endif // __TYPE_DEFINITIONS_H__