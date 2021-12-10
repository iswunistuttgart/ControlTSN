#include <string.h>

#include "json_serializer.h"

json_t *
_data_value_to_json(TSN_Module_Data_Entry_Type type, TSN_Module_Data_Entry_Value value)
{
    json_t *root = NULL;
    
    if (type == BINARY) {
        root = json_string(value.binary_val);
    }
    else if (type == BOOLEAN) {
        root = json_boolean(value.boolean_val);
    }
    else if (type == DECIMAL64) {
        root = json_real(value.decimal64_val);
    }
    else if (type == INSTANCE_IDENTIFIER) {
        root = json_string(value.instance_identifier_val);
    }
    else if (type == INT8) {
        root = json_integer(value.int8_val);
    }
    else if (type == INT16) {
        root = json_integer(value.int16_val);
    }
    else if (type == INT32) {
        root = json_integer(value.int32_val);
    }
    else if (type == INT64) {
        root = json_integer(value.int64_val);
    }
    else if (type == STRING) {
        root = json_string(value.string_val);
    }
    else if (type == UINT8) {
        root = json_integer(value.uint8_val);
    }
    else if (type == UINT16) {
        root = json_integer(value.uint16_val);
    }
    else if (type == UINT32) {
        root = json_integer(value.uint32_val);
    }
    else if (type == UINT64) {
        root = json_integer(value.uint64_val);
    }

    return root;
}

TSN_Module_Data_Entry_Value 
_json_to_data_value(json_t *val, TSN_Module_Data_Entry_Type type)
{
    TSN_Module_Data_Entry_Value value;

    if (type == BINARY) {
        value.binary_val = json_string_value(val);
    }
    else if (type == BOOLEAN) {
        value.boolean_val = json_boolean_value(val);
    }
    else if (type == DECIMAL64) {
        value.decimal64_val = json_real_value(val);
    }
    else if (type == INSTANCE_IDENTIFIER) {
        value.instance_identifier_val = json_string_value(val);
    }
    else if (type == INT8) {
        value.int8_val = json_integer_value(val);
    }
    else if (type == INT16) {
        value.int16_val = json_integer_value(val);
    }
    else if (type == INT32) {
        value.int32_val = json_integer_value(val);
    }
    else if (type == INT64) {
        value.int64_val = json_integer_value(val);
    }
    else if (type == STRING) {
        value.string_val = json_string_value(val);
    }
    else if (type == UINT8) {
        value.uint8_val = json_integer_value(val);
    }
    else if (type == UINT16) {
        value.uint16_val = json_integer_value(val);
    }
    else if (type == UINT32) {
        value.uint32_val = json_integer_value(val);
    }
    else if (type == UINT64) {
        value.uint64_val = json_integer_value(val);
    }

    return value;
}


// ------------------------------------
// Module 
// ------------------------------------
json_t *
serialize_module_data(TSN_Module_Data *module_data)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_entries", json_integer(module_data->count_entries));

    json_t *array_data = NULL;
    array_data = json_array();
    for (int i=0; i<module_data->count_entries; ++i) {
        json_t *data_entry = NULL;
        data_entry = json_object();

        json_object_set_new(data_entry, "name", json_string(module_data->entries[i].name));
        json_object_set_new(data_entry, "type", json_string(data_type_to_string(module_data->entries[i].type)));
        json_object_set_new(data_entry, "value", _data_value_to_json(module_data->entries[i].type, module_data->entries[i].value));

        json_array_append_new(array_data, data_entry);
    }
    json_object_set_new(root, "entries", array_data);

    return root;
}

json_t *
serialize_module(TSN_Module *module)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "id", json_integer(module->id));
    json_object_set_new(root, "p_id", json_integer(module->p_id));
    json_object_set_new(root, "path", json_string(module->path));
    json_object_set_new(root, "name", json_string(module->name));
    json_object_set_new(root, "registered", json_integer(module->registered));
    json_object_set_new(root, "description", json_string(module->description));
    json_object_set_new(root, "subscribed_events_mask", json_integer(module->subscribed_events_mask));

    return root;
}

json_t * 
serialize_modules(TSN_Modules *modules)
{
    /*
    json_t *root = NULL;
    root = json_object();
    json_object_set_new(root, "count_available_modules", json_integer(modules->count_available_modules));
    json_object_set_new(root, "count_registered_modules", json_integer(modules->count_registered_modules));

    json_t *array_available = NULL;
    json_t *array_registered = NULL;
    array_available = json_array();
    array_registered = json_array();

    // Available modules
    for (int i=0; i<modules->count_available_modules; ++i) {
        json_t *mod = NULL;
        mod = serialize_module(&modules->available_modules[i]);
        json_array_append_new(array_available, mod);
    }
    json_object_set_new(root, "available_modules", array_available);

    // Registered modules
    for (int i=0; i<modules->count_registered_modules; ++i) {
        json_t *mod = NULL;
        mod = serialize_module(&modules->registered_modules[i]);
        json_array_append_new(array_registered, mod);
    }
    json_object_set_new(root, "registered_modules", array_registered);

    return root;
    */

    json_t *root = NULL;
    root = json_object();
    json_object_set_new(root, "count_modules", json_integer(modules->count_modules));

    json_t *array_modules = NULL;
    array_modules = json_array();

    for (int i=0; i<modules->count_modules; ++i) {
        json_t *mod = NULL;
        mod = serialize_module(&modules->modules[i]);
        json_array_append_new(array_modules, mod);
    }
    json_object_set_new(root, "modules", array_modules);

    return root;
}

TSN_Module_Data_Entry *
deserialize_module_data_entry(json_t *obj) 
{
    TSN_Module_Data_Entry *entry = malloc(sizeof(TSN_Module_Data_Entry));

    const json_t *name = json_object_get(obj, "name");
    entry->name = strdup(json_string_value(name));

    const json_t *type = json_object_get(obj, "type");
    entry->type = string_to_data_type(json_string_value(type));

    const json_t *value = json_object_get(obj, "value");
    entry->value = _json_to_data_value(value, entry->type);

    return entry;
}

TSN_Module_Data *
deserialize_module_data(json_t *obj)
{
    TSN_Module_Data *module_data = malloc(sizeof(TSN_Module_Data));

    uint16_t count_entries;
    TSN_Module_Data_Entry *entries;

    count_entries = json_number_value(json_object_get(obj, "count_entries"));
    module_data->count_entries = count_entries;
    entries = (TSN_Module_Data_Entry *) malloc(sizeof(TSN_Module_Data_Entry) * count_entries);
    json_t *entries_json = json_object_get(obj, "entries");
    for (int i=0; i<count_entries; ++i) {
        json_t *entry = json_array_get(entries_json, i);
        TSN_Module_Data_Entry *e = deserialize_module_data_entry(entry);
        module_data->entries[i] = *e;
    }

    return module_data;
}


// ------------------------------------
// Streams
// ------------------------------------
json_t *
serialize_stream(TSN_Stream *stream)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "stream_id", json_string(stream->stream_id));

    // Request          TODO

    // Configuration    TODO

    return root;
}

json_t *
serialize_streams(TSN_Streams *streams)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_streams", json_integer(streams->count_streams));

    json_t *array_streams = NULL;
    array_streams = json_array();
    for (int i=0; i<streams->count_streams; ++i) {
        json_t *s = NULL;
        s = serialize_stream(&streams->streams[i]);
        json_array_append_new(array_streams, s);
    }
    json_object_set_new(root, "streams", array_streams);

    return root;
}

// ------------------------------------
// Topology
// ------------------------------------
json_t *
serialize_enddevice(TSN_Enddevice *enddevice)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "mac", json_string(enddevice->mac));
    json_object_set_new(root, "app_ref", json_string(enddevice->app_ref));

    return root;
}

json_t *
serialize_switch(TSN_Switch *sw)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "mac", json_string(sw->mac));
    json_object_set_new(root, "ports_count", json_integer(sw->ports_count));

    return root;
}

json_t *
serialize_devices(TSN_Devices *devices)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_enddevices", json_integer(devices->count_enddevices));
    json_object_set_new(root, "count_switches", json_integer(devices->count_switches));

    // Enddevices
    json_t *array_enddevices = NULL;
    array_enddevices = json_array();
    for (int i=0; i<devices->count_enddevices; ++i) {
        json_t *ed = NULL;
        ed = serialize_enddevice(&devices->enddevices[i]);
        json_array_append_new(array_enddevices, ed);
    }
    json_object_set_new(root, "enddevices", array_enddevices);

    // Switches
    json_t *array_switches = NULL;
    array_switches = json_array();
    for (int i=0; i<devices->count_switches; ++i) {
        json_t *sw = NULL;
        sw = serialize_switch(&devices->switches[i]);
        json_array_append_new(array_switches, sw);
    }
    json_object_set_new(root, "switches", array_switches);

    return root;
}

json_t *
serialize_connection(TSN_Connection *connection)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "id", json_integer(connection->id));
    json_object_set_new(root, "from_mac", json_string(connection->from_mac));
    json_object_set_new(root, "from_port", json_integer(connection->from_port));
    json_object_set_new(root, "to_mac", json_string(connection->to_mac));
    json_object_set_new(root, "to_port", json_integer(connection->to_port));

    return root;
}

json_t *
serialize_graph(TSN_Graph *graph) 
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_connections", json_integer(graph->count_connections));

    // Connections
    json_t *array_connections = NULL;
    array_connections = json_array();
    for (int i=0; i<graph->count_connections; ++i) {
        json_t *c = NULL;
        c = serialize_connection(&graph->connections[i]);
        json_array_append_new(array_connections, c);
    }
    json_object_set_new(root, "connections", array_connections);

    return root;
}

json_t *
serialize_topology(TSN_Topology *topology)
{
    json_t *root = NULL;
    root = json_object();

    // Devices
    json_t *devices = NULL;
    devices = serialize_devices(&topology->devices);
    json_object_set_new(root, "devices", devices);

    // Graph
    json_t *graph = NULL;
    graph = serialize_graph(&topology->graph);
    json_object_set_new(root, "graph", graph);

    return root;
}

TSN_Enddevice *
deserialize_enddevice(json_t *obj)
{
    TSN_Enddevice *enddevice = malloc(sizeof(TSN_Enddevice));
    json_t *mac;
    json_t *app_ref;

    mac = json_object_get(obj, "mac");
    enddevice->mac = strdup(json_string_value(mac));

    app_ref = json_object_get(obj, "app_ref");
    if (app_ref != NULL) {
        enddevice->app_ref = strdup(json_string_value(app_ref));
    }

    return enddevice;
}

TSN_Switch 
*deserialize_switch(json_t *obj)
{
    TSN_Switch *sw = malloc(sizeof(TSN_Switch));
    json_t *mac;
    json_t *ports_count;

    mac = json_object_get(obj, "mac");
    sw->mac = strdup(json_string_value(mac));

    ports_count = json_object_get(obj, "ports_count");
    sw->ports_count = json_integer_value(ports_count);

    return sw;
}

TSN_Devices 
*deserialize_devices(json_t *obj)
{
    TSN_Devices *devices = malloc(sizeof(TSN_Devices));
    json_t *enddevices;
    json_t *switches;

    enddevices = json_object_get(obj, "enddevices");
    devices->count_enddevices = json_array_size(enddevices);
    devices->enddevices = (TSN_Enddevice *) malloc(sizeof(TSN_Enddevice) * devices->count_enddevices);
    for (int i=0; i<devices->count_enddevices; ++i) {
        json_t *entry = json_array_get(enddevices, i);
        TSN_Enddevice *x = deserialize_enddevice(entry);
        devices->enddevices[i] = *x;
    }

    switches = json_object_get(obj, "switches");
    devices->count_switches = json_array_size(switches);
    devices->switches = (TSN_Switch *) malloc(sizeof(TSN_Switch) * devices->count_switches);
    for (int i=0; i<devices->count_switches; ++i) {
        json_t *entry = json_array_get(switches, i);
        TSN_Switch *x = deserialize_switch(entry);
        devices->switches[i] = *x;
    }    

    return devices;
}

TSN_Connection 
*deserialize_connection(json_t *obj)
{
    TSN_Connection *connection = malloc(sizeof(TSN_Connection));
    json_t *id;
    json_t *from_mac;
    json_t *from_port;
    json_t *to_mac;
    json_t *to_port;

    id = json_object_get(obj, "id");
    connection->id = json_integer_value(id);

    from_mac = json_object_get(obj, "from_mac");
    connection->from_mac = strdup(json_string_value(from_mac));

    from_port = json_object_get(obj, "from_port");
    connection->from_port = json_integer_value(from_port);

    to_mac = json_object_get(obj, "to_mac");
    connection->to_mac = strdup(json_string_value(to_mac));

    to_port = json_object_get(obj, "to_port");
    connection->to_port = json_integer_value(to_port);

    return connection;
}

TSN_Graph *deserialize_graph(json_t *obj)
{
    TSN_Graph *graph = malloc(sizeof(TSN_Graph));
    json_t *connections;
    connections = json_object_get(obj, "connections");
    graph->count_connections = json_array_size(connections);
    graph->connections = (TSN_Connection *) malloc(sizeof(TSN_Connection) * graph->count_connections);
    for (int i=0; i<graph->count_connections; ++i) {
        json_t *entry = json_array_get(connections, i);
        TSN_Connection *x = deserialize_connection(entry);
        graph->connections[i] = *x;
    }

    return graph;
}

TSN_Topology *deserialize_topology(json_t *obj)
{
    TSN_Topology *topology = malloc(sizeof(TSN_Topology));
    json_t *devices;
    json_t *graph;

    devices = json_object_get(obj, "devices");
    TSN_Devices *d = deserialize_devices(devices);
    topology->devices = *d;

    graph = json_object_get(obj, "graph");
    TSN_Graph *g = deserialize_graph(graph);
    topology->graph = *g;

    return topology;
}

// ------------------------------------
// Application
// ------------------------------------
json_t *
serialize_app_parameter(TSN_App_Parameter *param)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "name", json_string(param->name));
    json_object_set_new(root, "description", json_string(param->description));
    json_object_set_new(root, "type", json_string(data_type_to_string(param->type)));
    json_object_set_new(root, "value", _data_value_to_json(param->type, param->value));

    return root;
}

json_t *
serialize_app(TSN_App *app)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "id", json_string(app->id));
    json_object_set_new(root, "name", json_string(app->name));
    json_object_set_new(root, "description", json_string(app->description));
    json_object_set_new(root, "version", json_string(app->version));
    json_object_set_new(root, "image_ref", json_string(app->image_ref));

    json_object_set_new(root, "count_parameters", json_integer(app->count_parameters));
    // Parameters
    json_t *array_parameters = NULL;
    array_parameters = json_array();
    for (int i=0; i<app->count_parameters; ++i) {
        json_t *param = NULL;
        param = serialize_app_parameter(&app->parameters[i]);
        json_array_append_new(array_parameters, param);
    }
    json_object_set_new(root, "parameters", array_parameters);

    return root;
}

json_t *
serialize_apps(TSN_Apps *apps)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_apps", json_integer(apps->count_apps));

    // Apps
    json_t *array_apps = NULL;
    array_apps = json_array();
    for (int i=0; i<apps->count_apps; ++i) {
        json_t *a = NULL;
        a = serialize_app(&apps->apps[i]);
        json_array_append_new(array_apps, a);
    }
    json_object_set_new(root, "apps", array_apps);

    return root;
}

json_t *
serialize_image(TSN_Image *image)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "id", json_string(image->id));
    json_object_set_new(root, "name", json_string(image->name));
    json_object_set_new(root, "description", json_string(image->description));
    json_object_set_new(root, "version", json_string(image->version));

    return root;
}

json_t *
serialize_images(TSN_Images *images)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_images", json_integer(images->count_images));

    // Images
    json_t *array_images = NULL;
    array_images = json_array();
    for (int i=0; i<images->count_images; ++i) {
        json_t *c = NULL;
        c = serialize_image(&images->images[i]);
        json_array_append_new(array_images, c);
    }
    json_object_set_new(root, "images", array_images);

    return root;
}

json_t *
serialize_application(TSN_Application *application)
{
    json_t *root = NULL;
    root = json_object();

    // Apps
    json_t *apps = NULL;
    apps = serialize_apps(&application->apps);
    json_object_set_new(root, "apps", apps);

    // Images
    json_t *images = NULL;
    images = serialize_images(&application->images);
    json_object_set_new(root, "images", images);

    return root;
}
