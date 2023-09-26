/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#include "logger.h"

// ----------------------------------------------
//      Helper
// ----------------------------------------------



// ----------------------------------------------
//      Printing
// ----------------------------------------------
void
print_enddevice(TSN_Enddevice enddevice)
{
    printf("MAC:           %s\n", enddevice.mac);
    printf("Interface URI: %s\n", enddevice.interface_uri);
    //printf("App-Ref: %s\n", enddevice.app_ref);
    printf("Apps:\n");
    for (int i=0; i<enddevice.count_apps; ++i) {
        printf("   %s\n", enddevice.apps[i].app_ref);
    }
}

static void
_print_switch(TSN_Switch sw)
{
    printf("MAC:         %s\n", sw.mac);
    printf("Ports Count: %d\n", sw.ports_count);
}

static void
_print_connection(TSN_Connection connection)
{
    printf("ID:        %d\n", connection.id);
    printf("From MAC:  %s\n", connection.from_mac);
    printf("From Port: %d\n", connection.from_port);
    printf("To MAC:    %s\n", connection.to_mac);
    printf("To Port:   %d\n", connection.to_port);
}

void
print_module_data(TSN_Module_Data data)
{
    for (int i=0; i<data.count_entries; ++i) {
        char *type_str = data_type_to_string(data.entries[i].type);
        char *val_str = data_value_to_string(data.entries[i].type, data.entries[i].value);
        printf("   %s (%s): %s\n", data.entries[i].name, type_str, val_str);
    }
}

void
print_module(TSN_Module module) {
    printf("----- MODULE -----\n");
    printf("Name:           %s\n", module.name);
    printf("Description:    %s\n", module.description);
    printf("ID:             %d\n", module.id);
    printf("P_ID:           %d\n", module.p_id);
    printf("Path:           %s\n", module.path);
    printf("Events Mask:    %ld\n", module.subscribed_events_mask);
    printf("Data:\n");
    print_module_data(module.data);
    printf("\n");
}

void
print_topology(TSN_Topology topology)
{
    printf("----- Topology -----\n");
    printf("Devices:\n");
    printf("Enddevices (%d):\n", topology.devices.count_enddevices);
    for (int i=0; i<topology.devices.count_enddevices; ++i) {
        print_enddevice(topology.devices.enddevices[i]);
    }
    printf("Switches (%d):\n", topology.devices.count_switches);
    for (int i=0; i<topology.devices.count_switches; ++i) {
        _print_switch(topology.devices.switches[i]);
    }
    printf("Graph:\n");
    printf("Connections (%d):\n", topology.graph.count_connections);
    for (int i=0; i<topology.graph.count_connections; ++i) {
        _print_connection(topology.graph.connections[i]);
    }
}

void
print_stream_request(TSN_Request request)
{
    printf("----- Request -----\n");
    printf("Talker:\n");
    for (int i=0; i<request.talker.count_end_station_interfaces; ++i) {
        printf("   End station interface #%02d: %s '%s'\n", i+1, request.talker.end_station_interfaces[i].mac_address, request.talker.end_station_interfaces[i].interface_name);
    }
    printf("   Traffic specification:\n");
    printf("      Interval: %d / %d\n", request.talker.traffic_specification.interval.numerator, request.talker.traffic_specification.interval.denominator);
    printf("      Max frames per interval: %d\n", request.talker.traffic_specification.max_frames_per_interval);
    printf("      Max frame size: %d\n", request.talker.traffic_specification.max_frame_size);
    printf("      Time aware:\n");
    printf("         Earliest transmit offset: %d\n", request.talker.traffic_specification.time_aware.earliest_transmit_offset);
    printf("         Latest transmit offset:   %d\n", request.talker.traffic_specification.time_aware.latest_transmit_offset);
    printf("   User to network requirements:\n");
    printf("      Max latency: %d\n", request.talker.user_to_network_requirements.max_latency);

    printf("Listeners (%d):\n", request.count_listeners);
    for (int i=0; i<request.count_listeners; ++i) {
        printf("   Listener #%d\n", request.listener_list[i].index);
        for (int j=0; j<request.listener_list[i].count_end_station_interfaces; ++j) {
            printf("      End station interface #%02d: %s '%s'\n", i+1, request.listener_list[i].end_station_interfaces[j].mac_address, request.listener_list[i].end_station_interfaces[j].interface_name);
        }
        printf("      User to network requirements:\n");
        printf("         Max latency: %d\n", request.listener_list[i].user_to_network_requirements.max_latency);
    }
}

void
print_stream_configuration(TSN_Configuration config)
{
    printf("----- Configuration -----\n");
    printf("   StatusInfo:\n");
    printf("      Talker:       %d\n", config.status_info.talker_status);
    printf("      Listener:     %d\n", config.status_info.listener_status);
    printf("      Failure Code: %d\n", config.status_info.failure_code);
    printf("   Failed Interfaces:\n");
    for (int i=0; i<config.count_failed_interfaces; ++i) {
        printf("      Failed interface #%02d: %s '%s'\n", i+1, config.failed_interfaces[i].mac_address, config.failed_interfaces[i].interface_name);
    }
    printf("   Talker:\n");
    printf("      Accumulated Latency: %d\n", config.talker.accumulated_latency);
    printf("      Interface Configuration:\n");
    for (int i=0; i<config.talker.interface_configuration.count_interface_list_entries; ++i) {
        printf("         %02d: ", i+1);
        printf("   MAC:       %s\n", config.talker.interface_configuration.interface_list[i].mac_address);
        printf("                Interface: %s\n", config.talker.interface_configuration.interface_list[i].interface_name);
        printf("                Config List:\n");
        for(int j=0; j<config.talker.interface_configuration.interface_list[i].count_config_list_entries; ++j) {
            printf("                Index: %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].index);
            if (config.talker.interface_configuration.interface_list[i].config_list[j].field_type == CONFIG_LIST_MAC_ADDRESSES) {
                printf("                      Destination MAC: %s\n", config.talker.interface_configuration.interface_list[i].config_list[j].ieee802_mac_addresses->destination_mac_address);
                printf("                      Source MAC:      %s\n", config.talker.interface_configuration.interface_list[i].config_list[j].ieee802_mac_addresses->source_mac_address);
            } else if (config.talker.interface_configuration.interface_list[i].config_list[j].field_type == CONFIG_LIST_VLAN_TAG) {
                printf("                      PCP:     %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ieee802_vlan_tag->priority_code_point);
                printf("                      VLAN ID: %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ieee802_vlan_tag->vlan_id);
            } else if (config.talker.interface_configuration.interface_list[i].config_list[j].field_type == CONFIG_LIST_IPV4_TUPLE) {
                printf("                      Source IP:        %s\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv4_tuple->source_ip_address);
                printf("                      Destination IP:   %s\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv4_tuple->destination_ip_address);
                printf("                      DSCP:             %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv4_tuple->dscp);
                printf("                      Protocol:         %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv4_tuple->protocol);
                printf("                      Source Port:      %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv4_tuple->source_port);
                printf("                      Destination Port: %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv4_tuple->destination_port);
            } else if (config.talker.interface_configuration.interface_list[i].config_list[j].field_type == CONFIG_LIST_IPV6_TUPLE) {
                printf("                      Source IP:        %s\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv6_tuple->source_ip_address);
                printf("                      Destination IP:   %s\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv6_tuple->destination_ip_address);
                printf("                      DSCP:             %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv6_tuple->dscp);
                printf("                      Protocol:         %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv6_tuple->protocol);
                printf("                      Source Port:      %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv6_tuple->source_port);
                printf("                      Destination Port: %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].ipv6_tuple->destination_port);
            } else if (config.talker.interface_configuration.interface_list[i].config_list[j].field_type == CONFIG_LIST_TIME_AWARE_OFFSET) {
                printf("                      Time Aware Offset: %d\n", config.talker.interface_configuration.interface_list[i].config_list[j].time_aware_offset);
            }
        }
    }
    
    printf("   Listeners:\n");
    for (int i=0; i<config.count_listeners; ++i) {
        printf("      Listener #%d\n", i);
        printf("         Accumulated Latency: %d\n", config.listener_list[i].accumulated_latency);
        printf("         Interface Configuration:\n");
        for (int k=0; k<config.listener_list[i].interface_configuration.count_interface_list_entries; ++k) {
            printf("            %02d: ", i+1);
            printf("   MAC:          %s\n", config.listener_list[i].interface_configuration.interface_list[k].mac_address);
            printf("                   Interface: %s\n", config.listener_list[i].interface_configuration.interface_list[k].interface_name);
            printf("                   Config List:\n");
            for(int j=0; j<config.listener_list[i].interface_configuration.interface_list[k].count_config_list_entries; ++j) {
                printf("                   Index: %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].index);
                if (config.listener_list[i].interface_configuration.interface_list[k].config_list[j].field_type == CONFIG_LIST_MAC_ADDRESSES) {
                    printf("                      Destination MAC: %s\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ieee802_mac_addresses->destination_mac_address);
                    printf("                      Source MAC:      %s\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ieee802_mac_addresses->source_mac_address);
                } else if (config.listener_list[i].interface_configuration.interface_list[k].config_list[j].field_type == CONFIG_LIST_VLAN_TAG) {
                    printf("                      PCP:     %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ieee802_vlan_tag->priority_code_point);
                    printf("                      VLAN ID: %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ieee802_vlan_tag->vlan_id);
                } else if (config.listener_list[i].interface_configuration.interface_list[k].config_list[j].field_type == CONFIG_LIST_IPV4_TUPLE) {
                    printf("                      Source IP:        %s\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv4_tuple->source_ip_address);
                    printf("                      Destination IP:   %s\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv4_tuple->destination_ip_address);
                    printf("                      DSCP:             %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv4_tuple->dscp);
                    printf("                      Protocol:         %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv4_tuple->protocol);
                    printf("                      Source Port:      %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv4_tuple->source_port);
                    printf("                      Destination Port: %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv4_tuple->destination_port);
                } else if (config.listener_list[i].interface_configuration.interface_list[k].config_list[j].field_type == CONFIG_LIST_IPV6_TUPLE) {
                    printf("                      Source IP:        %s\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv6_tuple->source_ip_address);
                    printf("                      Destination IP:   %s\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv6_tuple->destination_ip_address);
                    printf("                      DSCP:             %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv6_tuple->dscp);
                    printf("                      Protocol:         %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv6_tuple->protocol);
                    printf("                      Source Port:      %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv6_tuple->source_port);
                    printf("                      Destination Port: %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].ipv6_tuple->destination_port);
                } else if (config.listener_list[i].interface_configuration.interface_list[k].config_list[j].field_type == CONFIG_LIST_TIME_AWARE_OFFSET) {
                    printf("                      Time Aware Offset: %d\n", config.listener_list[i].interface_configuration.interface_list[k].config_list[j].time_aware_offset);
                }
            }
        }
    }
}

void 
print_stream(TSN_Stream stream)
{
    printf("----- Stream -----\n");
    printf("StreamID: %s\n", stream.stream_id);
    print_stream_request(stream.request);
    print_stream_configuration(*stream.configuration);
}

void 
print_app(TSN_App app)
{
    printf("----- App -----\n");
    printf("ID: %s\n", app.id);
    printf("Name: %s\n", app.name);
    printf("Version: %s\n", app.version);
    printf("Description: %s\n", app.description);
    printf("MAC: %s\n", app.has_mac ? app.mac : "-");
    printf("Image: %s\n", app.has_image ? app.image_ref : "-");
    printf("Parameters (%d):\n", app.count_parameters);
    for (int i=0; i<app.count_parameters; ++i) {
        char *type_str = data_type_to_string(app.parameters[i].type);
        char *val_str = data_value_to_string(app.parameters[i].type, app.parameters[i].value);
        printf("   %s (%s): %s | %s\n", app.parameters[i].name, type_str, val_str, app.parameters[i].description);
    }
    printf("Associated Streams:\n");
    printf("   Egress (%d):\n", app.stream_mapping.count_egress);
    for (int i=0; i<app.stream_mapping.count_egress; ++i) {
        printf("      %s\n", app.stream_mapping.egress[i]);
    }
    printf("   Ingress (%d):\n", app.stream_mapping.count_ingress);
    for (int i=0; i<app.stream_mapping.count_ingress; ++i) {
        printf("      %s\n", app.stream_mapping.ingress[i]);
    }
}
