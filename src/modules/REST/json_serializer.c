#include "json_serializer.h"

json_t *
_data_value_to_json(TSN_Module_Data_Entry *entry)
{
    json_t *root = NULL;
    
    if (entry->type == BINARY) {
        root = json_string(entry->value.binary_val);
    }
    else if (entry->type == BOOLEAN) {
        root = json_boolean(entry->value.boolean_val);
    }
    else if (entry->type == DECIMAL64) {
        root = json_real(entry->value.decimal64_val);
    }
    else if (entry->type == INSTANCE_IDENTIFIER) {
        root = json_string(entry->value.instance_identifier_val);
    }
    else if (entry->type == INT8) {
        root = json_integer(entry->value.int8_val);
    }
    else if (entry->type == INT16) {
        root = json_integer(entry->value.int16_val);
    }
    else if (entry->type == INT32) {
        root = json_integer(entry->value.int32_val);
    }
    else if (entry->type == INT64) {
        root = json_integer(entry->value.int64_val);
    }
    else if (entry->type == STRING) {
        root = json_string(entry->value.string_val);
    }
    else if (entry->type == UINT8) {
        root = json_integer(entry->value.uint8_val);
    }
    else if (entry->type == UINT16) {
        root = json_integer(entry->value.uint16_val);
    }
    else if (entry->type == UINT32) {
        root = json_integer(entry->value.uint32_val);
    }
    else if (entry->type == UINT64) {
        root = json_integer(entry->value.uint64_val);
    }

    return root;
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
        json_object_set_new(data_entry, "value", _data_value_to_json(&module_data->entries[i]));

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
    json_object_set_new(root, "description", json_string(module->description));
    json_object_set_new(root, "subscribed_events_mask", json_integer(module->subscribed_events_mask));

    return root;
}

json_t * 
serialize_modules(TSN_Modules *modules)
{
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
    json_object_set_new(root, "value", _data_value_to_json(&param->value));

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
