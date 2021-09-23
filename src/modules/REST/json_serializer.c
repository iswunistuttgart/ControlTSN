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

// Streams
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