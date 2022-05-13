#ifndef __TYPE_DEFINITIONS_H__
#define __TYPE_DEFINITIONS_H__

#include <stdint.h>
#include "ieee802_tsn_types.h"
#include "events_definitions.h"
#include "sysrepo.h"


// ---------------------------------------
// Module definitions
// ---------------------------------------
typedef enum TSN_Module_Data_Entry_Type {
    BINARY,
    BOOLEAN,
    DECIMAL64,
    INSTANCE_IDENTIFIER,
    INT8,
    INT16,
    INT32,
    INT64,
    STRING,
    UINT8,
    UINT16,
    UINT32,
    UINT64
} TSN_Module_Data_Entry_Type;

/**
 * @brief Converts the string from the sysrepo type enum to the module data type enum.
 * @param enum_string The string to convert
 * @return The module data type enum
 */
TSN_Module_Data_Entry_Type string_to_data_type(const char *enum_string);
/**
 * @brief Converts the module data type to the corresponding sysrepo enum string.
 * @param type The module data type enum
 * @return The converted string for writing to sysrepo
 */
char *data_type_to_string(TSN_Module_Data_Entry_Type type);

typedef union TSN_Module_Data_Entry_Value {
    char *binary_val;
    bool boolean_val;
    double decimal64_val;
    char *instance_identifier_val;
    int8_t int8_val;
    int16_t int16_val;
    int32_t int32_val;
    int64_t int64_val;
    char *string_val;
    uint8_t uint8_val;
    uint16_t uint16_val;
    uint32_t uint32_val;
    uint64_t uint64_val;
} TSN_Module_Data_Entry_Value;

/**
 * @brief Converts a sysrepo data struct to the corresponding module data value.
 * @param data The data struct from sysrepo
 * @param type The assigned type of this module data
 * @return The converted module data value struct
 */
TSN_Module_Data_Entry_Value sysrepo_data_to_data_value(sr_data_t data, TSN_Module_Data_Entry_Type type);
/**
 * @brief Converts a sysrepo value struct to the corresponding module data value.
 * @param data The sr_val_t struct from sysrepo
 * @return The converted module data value struct.
 */
TSN_Module_Data_Entry_Value sysrepo_value_to_data_value(sr_val_t value);
/**
 * @brief Converts a sysrepo value struct to the corresponding module data type.
 * @param data The sr_val_t struct from sysrepo
 * @return The converted module data type struct.
 */
TSN_Module_Data_Entry_Type sysrepo_value_to_data_type(sr_val_t value);
/**
 * @brief Converts a module data value to the corresponding sysrepo value.
 * @param value The module data value
 * @param type The assigned type of this module data
 * @return The converted sysrepo value struct
 */
sr_val_t data_value_to_sysrepo_value(TSN_Module_Data_Entry_Value value, TSN_Module_Data_Entry_Type type);

typedef struct TSN_Module_Data_Entry
{
    char *name;                         // The name of the variable
    TSN_Module_Data_Entry_Type type;    // The type of the variable
    TSN_Module_Data_Entry_Value value;  // The value of the variable
} TSN_Module_Data_Entry;

/**
 * @brief Converts a module data value to a string (e.g. for printing).
 * @param entry The module data entry
 * @return The value represented as a string
 */
char *data_value_to_string(TSN_Module_Data_Entry entry);

typedef struct TSN_Module_Data
{
    uint16_t count_entries;
    TSN_Module_Data_Entry *entries;
} TSN_Module_Data;

typedef struct TSN_Module_Reg {
    uint16_t id;
    uint32_t pid;
} TSN_Module_Reg;

typedef struct TSN_Module
{
    uint16_t id;        // Module ID
    int p_id;           // Module Process ID
    char *path;         // Path to the modules executable       (???)
    char *name;         // Module Name
    char *description;  // Module Description
    uint32_t subscribed_events_mask;     // Mask describing the relevant events for this module
    uint8_t registered;
    void (*cb_event)(TSN_Event_CB_Data data);     // Generic callback method for events
    TSN_Module_Data data;
} TSN_Module;

/*
typedef struct TSN_Modules {
    uint16_t count_available_modules;
    TSN_Module *available_modules;

    uint16_t count_registered_modules;
    TSN_Module *registered_modules;
} TSN_Modules;
*/

typedef struct TSN_Modules {
    uint16_t count_modules;
    TSN_Module *modules;
} TSN_Modules;


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
    //uint16_t count_failed_interfaces;

    TSN_StatusTalker talker;
    TSN_StatusListener *listener_list;

    // ieee802-dot1q-tsn-types -> group-status-stream
    IEEE_StatusInfo status_info;
    IEEE_InterfaceId *failed_interfaces;
    uint16_t count_failed_interfaces;
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
    uint8_t configured;
    TSN_Configuration *configuration;
} TSN_Stream;

typedef struct TSN_Streams {
    uint16_t count_streams;
    TSN_Stream *streams;
} TSN_Streams;


// ---------------------------------------
// Topology definitions
// ---------------------------------------
typedef struct TSN_Enddevice_AppRef {
    char *app_ref;
} TSN_Enddevice_AppRef;

typedef struct TSN_Enddevice {
    char *mac;
    uint8_t has_app;
    //char *app_ref;
    TSN_Enddevice_AppRef *apps;
    uint16_t count_apps;
} TSN_Enddevice;

typedef struct TSN_Switch {
    char *mac;
    uint8_t ports_count;
} TSN_Switch;

typedef struct TSN_Devices {
    uint16_t count_enddevices;
    TSN_Enddevice *enddevices;
    uint16_t count_switches;
    TSN_Switch *switches;
} TSN_Devices;

typedef struct TSN_Connection {
    uint16_t id;        // simple counter
    char *from_mac;
    uint8_t from_port;
    char *to_mac;
    uint8_t to_port;
} TSN_Connection;

typedef struct TSN_Graph {
    uint16_t count_connections;
    TSN_Connection *connections;
} TSN_Graph;

typedef struct TSN_Topology {
    TSN_Devices devices;
    TSN_Graph graph;
} TSN_Topology;

// ---------------------------------------
// Application definitions
// ---------------------------------------
typedef struct TSN_App_Parameter {
    char *name;
    char *description;
    TSN_Module_Data_Entry_Type type;
    TSN_Module_Data_Entry_Value value;
} TSN_App_Parameter;

/**
 * @brief Converts a parameter data value to a string (e.g. for printing).
 * @param parameter The parameter data entry
 * @return The value represented as a string
 */
char *
parameter_data_value_to_string(TSN_App_Parameter *parameter);

typedef struct TSN_App {
    char *id;           // {name}_{version}
    char *name;
    char *description;
    uint8_t has_mac;
    char *mac;          // (Virtual) MAC
    char *version;      // {X}.{Y}.{Z}
    uint8_t has_image;
    char *image_ref;
    uint8_t count_parameters;
    TSN_App_Parameter *parameters;
} TSN_App;

typedef struct TSN_Image {
    char *id;           // {name}_{version}
    char *name;
    char *description;
    char *version;      // {X}.{Y}.{Z}
} TSN_Image;

typedef struct TSN_Apps {
    uint16_t count_apps;
    TSN_App *apps;
} TSN_Apps;

typedef struct TSN_Images {
    uint16_t count_images;
    TSN_Image *images;
} TSN_Images;

typedef struct TSN_Application {
    TSN_Apps apps;
    TSN_Images images;
} TSN_Application;

// ----------------------------------------------
// Root
// ----------------------------------------------
typedef struct TSN_Uni {
    TSN_Streams streams;
    TSN_Modules modules;
    TSN_Topology topology;
    TSN_Application application;
} TSN_Uni;


// ----------------------------------------------
// Other
// ----------------------------------------------
typedef struct Websocket_Message {
    time_t timestamp;
    char *msg;
} Websocket_Message;


#endif // __TYPE_DEFINITIONS_H__
