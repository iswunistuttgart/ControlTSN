#include "logger.h"

// ----------------------------------------------
//      Helper
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


// ----------------------------------------------
//      Printing
// ----------------------------------------------
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
    printf("Events Mask:    %d\n", module.subscribed_events_mask);
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
    printf("----- Stream Request -----\n");
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
}
