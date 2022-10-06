#ifndef __IEEE802_IEEE_TYPES_H__
#define __IEEE802_IEEE_TYPES_H__

#include <stdint.h>
#include <stdbool.h>


typedef struct IEEE_StatusInfo {
    //IEEE_TalkerStatus talker_status;
    //IEEE_ListenerStatus listener_status;
    uint8_t talker_status;
    uint8_t listener_status;
    uint8_t failure_code;
} IEEE_StatusInfo;

typedef struct IEEE_InterfaceId {
    uint8_t index;
    char *mac_address;
    char *interface_name;
} IEEE_InterfaceId;

/*
typedef struct IEEE_StatusStream
{
    IEEE_StatusInfo status_info;
    IEEE_InterfaceId *failed_interfaces;

    uint16_t count_failed_interfaces;
} IEEE_StatusStream;
*/

typedef struct IEEE_MacAddresses {
    char *destination_mac_address;
    char *source_mac_address;
} IEEE_MacAddresses;

typedef struct IEEE_VlanTag {
    uint8_t priority_code_point;
    uint16_t vlan_id;
} IEEE_VlanTag;

typedef struct IEEE_IPv4Tuple {
    char *source_ip_address;
    char *destination_ip_address;
    uint8_t dscp;
    uint16_t protocol;
    uint16_t source_port;
    uint16_t destination_port;
} IEEE_IPv4Tuple;

typedef struct IEEE_IPv6Tuple {
    char *source_ip_address;
    char *destination_ip_address;
    uint8_t dscp;
    uint16_t protocol;
    uint16_t source_port;
    uint16_t destination_port;
} IEEE_IPv6Tuple;

typedef enum ConfigList_FieldType {
    CONFIG_LIST_MAC_ADDRESSES = 0,
    CONFIG_LIST_VLAN_TAG = 1,
    CONFIG_LIST_IPV4_TUPLE = 2,
    CONFIG_LIST_IPV6_TUPLE = 3,
    CONFIG_LIST_TIME_AWARE_OFFSET = 4,
} ConfigList_FieldType;

typedef struct IEEE_ConfigList {
    uint8_t index;
    ConfigList_FieldType field_type;                    // Added by xoe (should be included in the official yang in my opinion)
    /*
    union config_value
    {
        IEEE_MacAddresses ieee802_mac_addresses;
        IEEE_VlanTag ieee802_vlan_tag;
        IEEE_IPv4Tuple ipv4_tuple;
        IEEE_IPv6Tuple ipv6_tuple;
        uint32_t time_aware_offset;
    } config_value;
    */
    IEEE_MacAddresses *ieee802_mac_addresses;
    IEEE_VlanTag *ieee802_vlan_tag;
    IEEE_IPv4Tuple *ipv4_tuple;
    IEEE_IPv6Tuple *ipv6_tuple;
    uint32_t time_aware_offset;
} IEEE_ConfigList;

typedef struct IEEE_InterfaceList {
    char *mac_address;
    char *interface_name;
    //IEEE_InterfaceId interface_id;
    IEEE_ConfigList *config_list;

    uint16_t count_config_list_entries;
} IEEE_InterfaceList;

typedef struct IEEE_InterfaceConfiguration
{
    IEEE_InterfaceList *interface_list;

    uint16_t count_interface_list_entries;
} IEEE_InterfaceConfiguration;

typedef struct IEEE_StreamRank {
    uint8_t rank;
} IEEE_StreamRank;

typedef enum DataFrameSpecification_FieldType {
    DATA_FRAME_SPECIFICATION_MAC_ADDRESSES = 0,
    DATA_FRAME_SPECIFICATION_VLAN_TAG = 1,
    DATA_FRAME_SPECIFICATION_IPV4_TUPLE = 2,
    DATA_FRAME_SPECIFICATION_IPV6_TUPLE = 3
} DataFrameSpecification_FieldType;

typedef struct IEEE_DataFrameSpecification {
    uint8_t index;
    DataFrameSpecification_FieldType field_type;        // Added by xoe (should be included in the official yang in my opinion)
    /*
    union field {
        IEEE_MacAddresses ieee802_mac_addresses;
        IEEE_VlanTag ieee802_vlan_tag;
        IEEE_IPv4Tuple ipv4_tuple;
        IEEE_IPv6Tuple ipv6_tuple;
    } field;
    */
    IEEE_MacAddresses *ieee802_mac_addresses;
    IEEE_VlanTag *ieee802_vlan_tag;
    IEEE_IPv4Tuple *ipv4_tuple;
    IEEE_IPv6Tuple *ipv6_tuple;
} IEEE_DataFrameSpecification;

typedef struct IEEE_Interval {
    uint32_t numerator;
    uint32_t denominator;
} IEEE_Interval;

typedef struct IEEE_TimeAware {
    uint32_t earliest_transmit_offset;
    uint32_t latest_transmit_offset;
    uint32_t jitter;
} IEEE_TimeAware;

typedef struct IEEE_TrafficSpecification {
    IEEE_Interval interval;
    uint16_t max_frames_per_interval;
    uint16_t max_frame_size;
    uint8_t transmission_selection;
    IEEE_TimeAware time_aware;
} IEEE_TrafficSpecification;

typedef struct IEEE_UserToNetworkRequirements {
    uint8_t num_seamless_trees;
    uint32_t max_latency;
} IEEE_UserToNetworkRequirements;

typedef struct IEEE_InterfaceCapabilities {
    bool vlan_tag_capable;
    uint32_t *cb_stream_iden_type_list;
    uint32_t *cb_sequence_type_list;

    uint16_t count_cb_stream_iden_types;
    uint16_t count_cb_sequence_types;
} IEEE_InterfaceCapabilities;


#endif // __IEEE802_IEEE_TYPES_H__