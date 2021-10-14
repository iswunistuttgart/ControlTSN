#include "logger.h"

// ----------------------------------------------
//      Helper
// ----------------------------------------------
static void
_print_enddevice(TSN_Enddevice enddevice)
{
    printf("MAC:     %s\n", enddevice.mac);
    printf("App-Ref: %s\n", enddevice.app_ref);
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
        char *val_str = data_value_to_string(data.entries[i]);
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
        _print_enddevice(topology.devices.enddevices[i]);
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