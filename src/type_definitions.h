#ifndef __TYPE_DEFINITIONS_H__
#define __TYPE_DEFINITIONS_H__

#include <stdint.h>
#include "ieee802_tsn_types.h"


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

typedef struct TSN_Uni {
    uint16_t count_streams;
    
    TSN_Stream *stream_list;
} TSN_Uni;


#endif // __TYPE_DEFINITIONS_H__