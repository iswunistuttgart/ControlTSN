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
_json_to_data_value(const json_t *val, TSN_Module_Data_Entry_Type type)
{
    TSN_Module_Data_Entry_Value value;

    if (type == BINARY) {
        value.binary_val = (char *) json_string_value(val);
    }
    else if (type == BOOLEAN) {
        value.boolean_val = json_boolean_value(val);
    }
    else if (type == DECIMAL64) {
        value.decimal64_val = json_real_value(val);
    }
    else if (type == INSTANCE_IDENTIFIER) {
        value.instance_identifier_val = (char *) json_string_value(val);
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
        value.string_val = (char *) json_string_value(val);
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
    json_object_set_new(root, "data", serialize_module_data(&module->data));

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

    uint16_t count_entries = json_number_value(json_object_get(obj, "count_entries"));
    module_data->count_entries = count_entries;
    module_data->entries = (TSN_Module_Data_Entry *) malloc(sizeof(TSN_Module_Data_Entry) * count_entries);
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
serialize_stream_id(char *stream_id)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "stream_id", json_string(stream_id));

    return root;
}

json_t *
serialize_status_info(IEEE_StatusInfo *si)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "talker_status", json_string(si->talker_status));
    json_object_set_new(root, "listener_status", json_string(si->listener_status));
    json_object_set_new(root, "failure_code", json_integer(si->failure_code));

    return root;
}

json_t *
serialize_interface_id(IEEE_InterfaceId *ii)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "mac_address", json_string(ii->mac_address));
    json_object_set_new(root, "interface_name", json_string(ii->interface_name));

    return root;
}

json_t *
serialize_status_stream(IEEE_StatusStream *ss)
{
    json_t *root = NULL;
    root = json_object();

    json_t *status_info = NULL;
    status_info = serialize_status_info(&(ss->status_info));
    json_object_set_new(root, "status_info", status_info);

    json_object_set_new(root, "count_failed_interfaces", json_integer(ss->count_failed_interfaces));
    json_t *array_failed_interfaces = NULL;
    array_failed_interfaces = json_array();
    for (int i=0; i<ss->count_failed_interfaces; ++i) {
        json_t *x = NULL;
        x = serialize_interface_id(&(ss->failed_interfaces[i]));
        json_array_append_new(array_failed_interfaces, x);
    }
    json_object_set_new(root, "failed_interfaces", array_failed_interfaces);

    return root;
}

json_t *
serialize_mac_addresses(IEEE_MacAddresses *ma)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "destination_mac_address", json_string(ma->destination_mac_address));
    json_object_set_new(root, "source_mac_address", json_string(ma->source_mac_address));

    return root;
}

json_t *
serialize_vlan_tag(IEEE_VlanTag *vt)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "priority_code_point", json_integer(vt->priority_code_point));
    json_object_set_new(root, "vlan_id", json_integer(vt->vlan_id));

    return root;
}

json_t *
serialize_ipv4_tuple(IEEE_IPv4Tuple *ipv4)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "source_ip_address", json_string(ipv4->source_ip_address));
    json_object_set_new(root, "destination_ip_address", json_string(ipv4->destination_ip_address));
    json_object_set_new(root, "dscp", json_integer(ipv4->dscp));
    json_object_set_new(root, "protocol", json_integer(ipv4->protocol));
    json_object_set_new(root, "source_port", json_integer(ipv4->source_port));
    json_object_set_new(root, "destination_port", json_integer(ipv4->destination_port));

    return root;
}

json_t *
serialize_ipv6_tuple(IEEE_IPv6Tuple *ipv6)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "source_ip_address", json_string(ipv6->source_ip_address));
    json_object_set_new(root, "destination_ip_address", json_string(ipv6->destination_ip_address));
    json_object_set_new(root, "dscp", json_integer(ipv6->dscp));
    json_object_set_new(root, "protocol", json_integer(ipv6->protocol));
    json_object_set_new(root, "source_port", json_integer(ipv6->source_port));
    json_object_set_new(root, "destination_port", json_integer(ipv6->destination_port));

    return root;
}

json_t *
serialize_config_list(IEEE_ConfigList *cl)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "index", json_integer(cl->index));
    
    json_object_set_new(root, "field_type", json_integer(cl->field_type));
    json_t *field = NULL;
    if (cl->field_type == CONFIG_LIST_MAC_ADDRESSES) {
        field = serialize_mac_addresses(&(cl->config_value.ieee802_mac_addresses));
    } else if (cl->field_type == CONFIG_LIST_VLAN_TAG) {
        field = serialize_vlan_tag(&(cl->config_value.ieee802_vlan_tag));
    } else if (cl->field_type == CONFIG_LIST_IPV4_TUPLE) {
        field = serialize_ipv4_tuple(&(cl->config_value.ipv4_tuple));
    } else if (cl->field_type == CONFIG_LIST_IPV6_TUPLE) {
        field = serialize_ipv6_tuple(&(cl->config_value.ipv6_tuple));
    } else if (cl->field_type == CONFIG_LIST_TIME_AWARE_OFFSET) {
        field = json_integer(cl->config_value.time_aware_offset);
    }
    json_object_set_new(root, "config_value", field);

    return root;
}

json_t *
serialize_interface_list(IEEE_InterfaceList *il)
{
    json_t *root = NULL;
    root = json_object();

    json_t *interface_id = NULL;
    interface_id = serialize_interface_id(&(il->interface_id));
    json_object_set_new(root, "interface_id", interface_id);

    json_object_set_new(root, "count_config_list_entries", json_integer(il->count_config_list_entries));
    json_t *array_config_list = NULL;
    array_config_list = json_array();
    for (int i=0; i<il->count_config_list_entries; ++i) {
        json_t *x = NULL;
        x = serialize_config_list(&(il->config_list[i]));
        json_array_append_new(array_config_list, x);
    }
    json_object_set_new(root, "config_list", array_config_list);

    return root;
}

json_t *
serialize_interface_configuration(IEEE_InterfaceConfiguration *ic)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_interface_list_entries", json_integer(ic->count_interface_list_entries));
    json_t *array_interface_list_entries = NULL;
    array_interface_list_entries = json_array();
    for (int i=0; i<ic->count_interface_list_entries; ++i) {
        json_t *x = NULL;
        x = serialize_interface_list(&(ic->interface_list[i]));
        json_array_append_new(array_interface_list_entries, x);
    }
    json_object_set_new(root, "interface_list", array_interface_list_entries);

    return root;
}

json_t *
serialize_stream_rank(IEEE_StreamRank *sr)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "rank", json_integer(sr->rank));

    return root;
}

json_t *
serialize_data_frame_specification(IEEE_DataFrameSpecification *dfs)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "index", json_integer(dfs->index));
    
    json_object_set_new(root, "field_type", json_integer(dfs->field_type));
    json_t *field = NULL;
    if (dfs->field_type == DATA_FRAME_SPECIFICATION_MAC_ADDRESSES) {
        field = serialize_mac_addresses(&(dfs->field.ieee802_mac_addresses));
    } else if (dfs->field_type == DATA_FRAME_SPECIFICATION_VLAN_TAG) {
        field = serialize_vlan_tag(&(dfs->field.ieee802_vlan_tag));
    } else if (dfs->field_type == DATA_FRAME_SPECIFICATION_IPV4_TUPLE) {
        field = serialize_ipv4_tuple(&(dfs->field.ipv4_tuple));
    } else if (dfs->field_type == DATA_FRAME_SPECIFICATION_IPV6_TUPLE) {
        field = serialize_ipv6_tuple(&(dfs->field.ipv6_tuple));
    }
    json_object_set_new(root, "field", field);

    return root;
}

json_t *
serialize_interval(IEEE_Interval *i)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "numerator", json_integer(i->numerator));
    json_object_set_new(root, "denominator", json_integer(i->denominator));

    return root;
}

json_t *
serialize_time_aware(IEEE_TimeAware *ta)
{
    json_t *root = NULL;
    root = json_object();
    
    json_object_set_new(root, "earliest_transmit_offset", json_integer(ta->earliest_transmit_offset));
    json_object_set_new(root, "latest_transmit_offset", json_integer(ta->latest_transmit_offset));
    json_object_set_new(root, "jitter", json_integer(ta->jitter));

    return root;
}

json_t *
serialize_traffic_specification(IEEE_TrafficSpecification *ts)
{
    json_t *root = NULL;
    root = json_object();

    json_t *interval = NULL;
    interval = serialize_interval(&(ts->interval));
    json_object_set_new(root, "interval", interval);
    
    json_object_set_new(root, "max_frames_per_interval", json_integer(ts->max_frames_per_interval));
    json_object_set_new(root, "max_frame_size", json_integer(ts->max_frame_size));
    json_object_set_new(root, "transmission_selection", json_integer(ts->transmission_selection));
    
    json_t *time_aware = NULL;
    time_aware = serialize_time_aware(&(ts->time_aware));
    json_object_set_new(root, "time_aware", time_aware);

    return root;
}

json_t *
serialize_user_to_network_requirements(IEEE_UserToNetworkRequirements *utnr)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "num_seamless_trees", json_integer(utnr->num_seamless_trees));
    json_object_set_new(root, "max_latency", json_integer(utnr->max_latency));

    return root;
}

json_t *
serialize_interface_capabilities(IEEE_InterfaceCapabilities *ic)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "vlan_tag_capable", json_boolean(ic->vlan_tag_capable));
    
    json_object_set_new(root, "count_cb_stream_iden_types", json_integer(ic->count_cb_stream_iden_types));
    json_t *array_cb_stream_iden_type_list = NULL;
    array_cb_stream_iden_type_list = json_array();
    for (int i=0; i<ic->count_cb_stream_iden_types; ++i) {
        json_t *c = NULL;
        c = json_integer(ic->cb_stream_iden_type_list[i]);
        json_array_append_new(array_cb_stream_iden_type_list, c);
    }
    json_object_set_new(root, "cb_stream_iden_type_list", array_cb_stream_iden_type_list);

    json_object_set_new(root, "count_cb_sequence_types", json_integer(ic->count_cb_sequence_types));
    json_t *array_cb_sequence_type_list = NULL;
    array_cb_sequence_type_list = json_array();
    for (int i=0; i<ic->count_cb_sequence_types; ++i) {
        json_t *c = NULL;
        c = json_integer(ic->cb_sequence_type_list[i]);
        json_array_append_new(array_cb_sequence_type_list, c);
    }
    json_object_set_new(root, "cb_sequence_type_list", array_cb_sequence_type_list);

    return root;
}

json_t *
serialize_talker(TSN_Talker *talker)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_end_station_interfaces", json_integer(talker->count_end_station_interfaces));
    json_object_set_new(root, "count_data_frame_specifications", json_integer(talker->count_data_frame_specifications));

    json_t *stream_rank = NULL;
    stream_rank = serialize_stream_rank(&(talker->stream_rank));
    json_object_set_new(root, "stream_rank", stream_rank);

    json_t *array_end_station_interfaces = NULL;
    array_end_station_interfaces = json_array();
    for (int i=0; i<talker->count_end_station_interfaces; ++i) {
        json_t *e = NULL;
        e = serialize_interface_id(&(talker->end_station_interfaces[i]));
        json_array_append_new(array_end_station_interfaces, e);
    }
    json_object_set_new(root, "end_station_interfaces", array_end_station_interfaces);

    json_t *array_data_frame_specification = NULL;
    array_data_frame_specification = json_array();
    for (int i=0; i<talker->count_data_frame_specifications; ++i) {
        json_t *d = NULL;
        d = serialize_data_frame_specification(&(talker->data_frame_specification[i]));
        json_array_append_new(array_data_frame_specification, d);
    }
    json_object_set_new(root, "data_frame_specification", array_data_frame_specification);

    json_t *traffic_specification = NULL;
    traffic_specification = serialize_traffic_specification(&(talker->traffic_specification));
    json_object_set_new(root, "traffic_specification", traffic_specification);

    json_t *user_to_network_requirement = NULL;
    user_to_network_requirement = serialize_user_to_network_requirements(&(talker->user_to_network_requirements));
    json_object_set_new(root, "user_to_network_requirement", user_to_network_requirement);

    json_t *interface_capabilities = NULL;
    interface_capabilities = serialize_interface_capabilities(&(talker->interface_capabilities));
    json_object_set_new(root, "interface_capabilities", interface_capabilities);

    return root;
}

json_t *
serialize_listener(TSN_Listener *listener)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "index", json_integer(listener->index));

    json_object_set_new(root, "count_end_station_interfaces", json_integer(listener->count_end_station_interfaces));
    json_t *array_end_station_interfaces = NULL;
    array_end_station_interfaces = json_array();
    for (int i=0; i<listener->count_end_station_interfaces; ++i) {
        json_t *x = NULL;
        x = serialize_interface_id(&(listener->end_station_interfaces[i]));
        json_array_append_new(array_end_station_interfaces, x);
    }
    json_object_set_new(root, "end_station_interfaces", array_end_station_interfaces);

    json_t *user_to_network_requirements = NULL;
    user_to_network_requirements = serialize_user_to_network_requirements(&(listener->user_to_network_requirements));
    json_object_set_new(root, "user_to_network_requirements", user_to_network_requirements);

    json_t *interface_capabilities = NULL;
    interface_capabilities = serialize_interface_capabilities(&(listener->interface_capabilities));
    json_object_set_new(root, "interface_capabilities", interface_capabilities);

    return root;
}

json_t *
serialize_stream_request(TSN_Request *request)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_listeners", json_integer(request->count_listeners));

    json_t *talker = NULL;
    talker = serialize_talker(&(request->talker));
    json_object_set_new(root, "talker", talker);

    json_t *array_listeners = NULL;
    array_listeners = json_array();
    for (int i=0; i<request->count_listeners; ++i) {
        json_t *l = NULL;
        l = serialize_listener(&(request->listener_list[i]));
        json_array_append_new(array_listeners, l);
    }
    json_object_set_new(root, "listener_list", array_listeners);

    return root;
}

json_t *
serialize_status_talker(TSN_StatusTalker *st)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "accumulated_latency", json_integer(st->accumulated_latency));

    json_t *interface_configuration = NULL;
    interface_configuration = serialize_interface_configuration(&(st->interface_configuration));
    json_object_set_new(root, "interface_configuration", interface_configuration);

    return root;
}

json_t *
serialize_status_listener(TSN_StatusListener *sl)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "index", json_integer(sl->index));
    json_object_set_new(root, "accumulated_latency", json_integer(sl->accumulated_latency));

    json_t *interface_configuration = NULL;
    interface_configuration = serialize_interface_configuration(&(sl->interface_configuration));
    json_object_set_new(root, "interface_configuration", interface_configuration);

    return root;
}

json_t *
serialize_stream_configuration(TSN_Configuration *configuration)
{
    json_t *root = NULL;
    root = json_object();

    json_object_set_new(root, "count_listeners", json_integer(configuration->count_listeners));
    json_t *array_listener_list = NULL;
    array_listener_list = json_array();
    for (int i=0; i<configuration->count_listeners; ++i) {
        json_t *x = NULL;
        x = serialize_status_listener(&(configuration->listener_list[i]));
        json_array_append_new(array_listener_list, x);
    }
    json_object_set_new(root, "listener_list", array_listener_list);

    json_t *talker = NULL;
    talker = serialize_status_talker(&(configuration->talker));
    json_object_set_new(root, "talker", talker);

    json_t *status_stream = NULL;
    status_stream = serialize_status_stream(&(configuration->status_stream));
    json_object_set_new(root, "status_stream", status_stream);

    return root;
}

json_t *
serialize_stream(TSN_Stream *stream)
{
    json_t *root = NULL;
    root = json_object();

    // Stream ID
    json_object_set_new(root, "stream_id", json_string(stream->stream_id));

    // Request
   json_t *request = NULL;
   request = serialize_stream_request(&(stream->request));
   json_object_set_new(root, "request", request);

    // Configuration
    json_t *configuration = NULL;
    configuration = serialize_stream_configuration(stream->configuration);
    json_object_set_new(root, "configuration", configuration);

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

IEEE_StatusInfo *
deserialize_status_info(json_t *obj)
{
    IEEE_StatusInfo *si = malloc(sizeof(IEEE_StatusInfo));

    si->talker_status = strdup(json_string_value(json_object_get(obj, "talker_status")));
    si->listener_status = strdup(json_string_value(json_object_get(obj, "listener_status")));
    si->failure_code = json_integer_value(json_object_get(obj, "failure_code"));

    return si;
}

IEEE_InterfaceId *
deserialize_interface_id(json_t *obj)
{
    IEEE_InterfaceId *ii = malloc(sizeof(IEEE_InterfaceId));

    ii->mac_address = strdup(json_string_value(json_object_get(obj, "mac_address")));
    ii->interface_name = strdup(json_string_value(json_object_get(obj, "interface_name")));

    return ii;
}

IEEE_StatusStream *
deserialize_status_stream(json_t *obj)
{
    IEEE_StatusStream *x = malloc(sizeof(IEEE_StatusStream));

    x->status_info = *(deserialize_status_info(json_object_get(obj, "status_info")));


    json_t *failed_interfaces = json_object_get(obj, "failed_interfaces");
    x->count_failed_interfaces = json_array_size(failed_interfaces);
    x->failed_interfaces = (IEEE_InterfaceId *) malloc(sizeof(IEEE_InterfaceId) * x->count_failed_interfaces);
    for (int i=0; i<x->count_failed_interfaces; ++i) {
        json_t *e = json_array_get(failed_interfaces, i);
        x->failed_interfaces[i] = *(deserialize_interface_id(e));
    }

    return x;
}

IEEE_MacAddresses *
deserialize_mac_addresses(json_t *obj)
{
    IEEE_MacAddresses *x = malloc(sizeof(IEEE_MacAddresses));

    x->destination_mac_address = strdup(json_string_value(json_object_get(obj, "destination_mac_address")));
    x->source_mac_address = strdup(json_string_value(json_object_get(obj, "source_mac_address")));

    return x;
}

IEEE_VlanTag *
deserialize_vlan_tag(json_t *obj)
{
    IEEE_VlanTag *x = malloc(sizeof(IEEE_VlanTag));

    x->priority_code_point = json_integer_value(json_object_get(obj, "priority_code_point"));
    x->vlan_id = json_integer_value(json_object_get(obj, "vlan_id"));

    return x;
}

IEEE_IPv4Tuple *
deserialize_ipv4_tuple(json_t *obj) 
{
    IEEE_IPv4Tuple *x = malloc(sizeof(IEEE_IPv4Tuple));

    x->source_ip_address = strdup(json_string_value(json_object_get(obj, "source_ip_address")));
    x->destination_ip_address = strdup(json_string_value(json_object_get(obj, "destination_ip_address")));
    x->dscp = json_integer_value(json_object_get(obj, "dscp"));
    x->protocol = json_integer_value(json_object_get(obj, "protocol"));
    x->source_port = json_integer_value(json_object_get(obj, "source_port"));
    x->destination_port = json_integer_value(json_object_get(obj, "destination_port"));

    return x;
}

IEEE_IPv6Tuple *
deserialize_ipv6_tuple(json_t *obj)
{
    IEEE_IPv6Tuple *x = malloc(sizeof(IEEE_IPv6Tuple));

    x->source_ip_address = strdup(json_string_value(json_object_get(obj, "source_ip_address")));
    x->destination_ip_address = strdup(json_string_value(json_object_get(obj, "destination_ip_address")));
    x->dscp = json_integer_value(json_object_get(obj, "dscp"));
    x->protocol = json_integer_value(json_object_get(obj, "protocol"));
    x->source_port = json_integer_value(json_object_get(obj, "source_port"));
    x->destination_port = json_integer_value(json_object_get(obj, "destination_port"));

    return x;
}

IEEE_ConfigList *
deserialize_config_list(json_t *obj)
{
    IEEE_ConfigList *x = malloc(sizeof(IEEE_ConfigList));

    x->index = json_integer_value(json_object_get(obj, "index"));
    x->field_type = json_integer_value(json_object_get(obj, "field_type"));

    json_t *config_value = NULL;
    config_value = json_object_get(obj, "config_value");
    
    if (x->field_type == CONFIG_LIST_MAC_ADDRESSES) {
        x->config_value.ieee802_mac_addresses = *(deserialize_mac_addresses(json_object_get(config_value, "ieee802_mac_addresses")));
    } else if (x->field_type == CONFIG_LIST_VLAN_TAG) {
        x->config_value.ieee802_vlan_tag = *(deserialize_vlan_tag(json_object_get(config_value, "ieee802_vlan_tag")));
    } else if (x->field_type == CONFIG_LIST_IPV4_TUPLE) {
        x->config_value.ipv4_tuple = *(deserialize_ipv4_tuple(json_object_get(config_value, "ipv4_tuple")));
    } else if (x->field_type == CONFIG_LIST_IPV6_TUPLE) {
        x->config_value.ipv6_tuple = *(deserialize_ipv6_tuple(json_object_get(config_value, "ipv6_tuple")));
    } else if (x->field_type == CONFIG_LIST_TIME_AWARE_OFFSET) {
        x->config_value.time_aware_offset = json_integer_value(json_object_get(config_value, "time_aware_offset"));
    }

    return x;
}

IEEE_InterfaceList *
deserialize_interface_list(json_t *obj)
{
    IEEE_InterfaceList *x = malloc(sizeof(IEEE_InterfaceList));

    x->interface_id = *(deserialize_interface_id(json_object_get(obj, "interface_id")));

    json_t *config_list = json_object_get(obj, "config_list");
    x->count_config_list_entries = json_array_size(config_list);
    x->config_list = (IEEE_ConfigList *) malloc(sizeof(IEEE_ConfigList) * x->count_config_list_entries);
    for (int i=0; i<x->count_config_list_entries; ++i) {
        json_t *e = json_array_get(config_list, i);
        x->config_list[i] = *(deserialize_config_list(e));
    }

    return x;
}

IEEE_InterfaceConfiguration *
deserialize_interface_configuration(json_t *obj)
{
    IEEE_InterfaceConfiguration *x = malloc(sizeof(IEEE_InterfaceConfiguration));

    json_t *interface_list = json_object_get(obj, "interface_list");
    x->count_interface_list_entries = json_array_size(interface_list);
    x->interface_list = (IEEE_InterfaceList *) malloc(sizeof(IEEE_InterfaceList) * x->count_interface_list_entries);
    for (int i=0; i<x->count_interface_list_entries; ++i) {
        json_t *e = json_array_get(interface_list, i);
        x->interface_list[i] = *(deserialize_interface_list(e));
    }

    return x;
}

IEEE_StreamRank *
deserialize_stream_rank(json_t *obj)
{
    IEEE_StreamRank *x = malloc(sizeof(IEEE_StreamRank));

    x->rank = json_integer_value(json_object_get(obj, "rank"));

    return x;
}

IEEE_DataFrameSpecification *
deserialize_data_frame_specification(json_t *obj)
{
    IEEE_DataFrameSpecification *x = malloc(sizeof(IEEE_DataFrameSpecification));

    x->index = json_integer_value(json_object_get(obj, "index"));
    x->field_type = json_integer_value(json_object_get(obj, "field_type"));

    json_t *field = NULL;
    field = json_object_get(obj, "field");
    
    if (x->field_type == DATA_FRAME_SPECIFICATION_MAC_ADDRESSES) {
        x->field.ieee802_mac_addresses = *(deserialize_mac_addresses(json_object_get(field, "ieee802_mac_addresses")));
    } else if (x->field_type == DATA_FRAME_SPECIFICATION_VLAN_TAG) {
        x->field.ieee802_vlan_tag = *(deserialize_vlan_tag(json_object_get(field, "ieee802_vlan_tag")));
    } else if (x->field_type == DATA_FRAME_SPECIFICATION_IPV4_TUPLE) {
        x->field.ipv4_tuple = *(deserialize_ipv4_tuple(json_object_get(field, "ipv4_tuple")));
    } else if (x->field_type == DATA_FRAME_SPECIFICATION_IPV6_TUPLE) {
        x->field.ipv6_tuple = *(deserialize_ipv6_tuple(json_object_get(field, "ipv6_tuple")));
    }

    return x;    
}

IEEE_Interval *
deserialize_interval(json_t *obj)
{
    IEEE_Interval *x = malloc(sizeof(IEEE_Interval));

    x->numerator = json_integer_value(json_object_get(obj, "numerator"));
    x->denominator = json_integer_value(json_object_get(obj, "denominator"));

    return x;
}

IEEE_TimeAware *
deserialize_time_aware(json_t *obj)
{
    IEEE_TimeAware *x = malloc(sizeof(IEEE_TimeAware));

    x->earliest_transmit_offset = json_integer_value(json_object_get(obj, "earliest_transmit_offset"));
    x->latest_transmit_offset = json_integer_value(json_object_get(obj, "latest_transmit_offset"));
    x->jitter = json_integer_value(json_object_get(obj, "jitter"));

    return x;
}

IEEE_TrafficSpecification *
deserialize_traffic_specification(json_t *obj)
{
    IEEE_TrafficSpecification *x = malloc(sizeof(IEEE_TrafficSpecification));

    x->interval = *(deserialize_interval(json_object_get(obj, "interval")));
    x->max_frames_per_interval = json_integer_value(json_object_get(obj, "max_frames_per_interval"));
    x->max_frame_size = json_integer_value(json_object_get(obj, "max_frame_size"));
    x->transmission_selection = json_integer_value(json_object_get(obj, "transmission_selection"));
    x->time_aware = *(deserialize_time_aware(json_object_get(obj, "time_aware")));

    return x;
}

IEEE_UserToNetworkRequirements *
deserialize_user_to_network_requirements(json_t *obj)
{
    IEEE_UserToNetworkRequirements *x = malloc(sizeof(IEEE_UserToNetworkRequirements));

    x->num_seamless_trees = json_integer_value(json_object_get(obj, "num_seamless_trees"));
    x->max_latency = json_integer_value(json_object_get(obj, "max_latency"));

    return x;
}

IEEE_InterfaceCapabilities *
deserialize_interface_capabilities(json_t *obj)
{
    IEEE_InterfaceCapabilities *x = malloc(sizeof(IEEE_InterfaceCapabilities));

    x->vlan_tag_capable = json_boolean_value(json_object_get(obj, "vlan_tag_capable"));
    
    json_t *cb_stream_iden_type_list = json_object_get(obj, "cb_stream_iden_type_list");
    x->count_cb_stream_iden_types = json_array_size(cb_stream_iden_type_list);
    x->cb_stream_iden_type_list = (uint32_t *) malloc(sizeof(uint32_t) * x->count_cb_stream_iden_types);
    for (int i=0; i<x->count_cb_stream_iden_types; ++i) {
        json_t *e = json_array_get(cb_stream_iden_type_list, i);
        x->cb_stream_iden_type_list[i] = json_integer_value(e);
    }

    json_t *cb_sequence_type_list = json_object_get(obj, "cb_sequence_type_list");
    x->count_cb_sequence_types = json_array_size(cb_sequence_type_list);
    x->cb_sequence_type_list = (uint32_t *) malloc(sizeof(uint32_t) * x->count_cb_sequence_types);
    for (int i=0; i<x->count_cb_sequence_types; ++i) {
        json_t *e = json_array_get(cb_sequence_type_list, i);
        x->cb_sequence_type_list[i] = json_integer_value(e);
    }

    return x;
}

TSN_Talker *
deserialize_talker(json_t *obj)
{
    TSN_Talker *x = malloc(sizeof(TSN_Talker));

    x->stream_rank = *(deserialize_stream_rank(json_object_get(obj, "stream_rank")));
    json_t *end_station_interfaces = json_object_get(obj, "end_station_interfaces");
    x->count_end_station_interfaces = json_array_size(end_station_interfaces);
    x->end_station_interfaces = (IEEE_InterfaceId *) malloc(sizeof(IEEE_InterfaceId) * x->count_end_station_interfaces);
    for (int i=0; i<x->count_end_station_interfaces; ++i) {
        json_t *e = json_array_get(end_station_interfaces, i);
        x->end_station_interfaces[i] = *(deserialize_interface_id(e));
    }

    json_t *data_frame_specification = json_object_get(obj, "data_frame_specification");
    x->count_data_frame_specifications = json_array_size(data_frame_specification);
    x->data_frame_specification = (IEEE_DataFrameSpecification *) malloc(sizeof(IEEE_DataFrameSpecification) * x->count_data_frame_specifications);
    for (int i=0; i<x->count_data_frame_specifications; ++i) {
        json_t *e = json_array_get(data_frame_specification, i);
        x->data_frame_specification[i] = *(deserialize_data_frame_specification(e));
    }

    x->traffic_specification = *(deserialize_traffic_specification(json_object_get(obj, "traffic_specification")));
    x->user_to_network_requirements = *(deserialize_user_to_network_requirements(json_object_get(obj, "user_to_network_requirements")));
    x->interface_capabilities = *(deserialize_interface_capabilities(json_object_get(obj, "interface_capabilities")));

    return x;
}

TSN_Listener *
deserialize_listener(json_t *obj)
{
    TSN_Listener *x = malloc(sizeof(TSN_Listener));

    x->index = json_integer_value(json_object_get(obj, "index"));
    
    json_t *end_station_interfaces = json_object_get(obj, "end_station_interfaces");
    x->count_end_station_interfaces = json_array_size(end_station_interfaces);
    x->end_station_interfaces = (IEEE_InterfaceId *) malloc(sizeof(IEEE_InterfaceId) * x->count_end_station_interfaces);
    for (int i=0; i<x->count_end_station_interfaces; ++i) {
        json_t *e = json_array_get(end_station_interfaces, i);
        x->end_station_interfaces[i] = *(deserialize_interface_id(e));
    }

    x->user_to_network_requirements = *(deserialize_user_to_network_requirements(json_object_get(obj, "user_to_network_requirements")));
    x->interface_capabilities = *(deserialize_interface_capabilities(json_object_get(obj, "interface_capabilities")));

    return x;
}

TSN_StatusTalker *
deserialize_status_talker(json_t *obj)
{
    TSN_StatusTalker *x = malloc(sizeof(TSN_StatusTalker));

    x->accumulated_latency = json_integer_value(json_object_get(obj, "accumulated_latency"));
    x->interface_configuration = *(deserialize_interface_configuration(json_object_get(obj, "interface_configuration")));

    return x;
}

TSN_StatusListener *
deserialize_status_listener(json_t *obj)
{
    TSN_StatusListener *x = malloc(sizeof(TSN_StatusListener));

    x->index = json_integer_value(json_object_get(obj, "index"));
    x->accumulated_latency = json_integer_value(json_object_get(obj, "accumulated_latency"));
    x->interface_configuration = *(deserialize_interface_configuration(json_object_get(obj, "interface_configuration")));

    return x;
}

TSN_Request *
deserialize_stream_request(json_t *obj)
{
    TSN_Request *x = malloc(sizeof(TSN_Request));

    x->talker = *(deserialize_talker(json_object_get(obj, "talker")));

    json_t *listener_list = json_object_get(obj, "listener_list");
    x->count_listeners = json_array_size(listener_list);
    x->listener_list = (TSN_Listener *) malloc(sizeof(TSN_Listener) * x->count_listeners);
    for (int i=0; i<x->count_listeners; ++i) {
        json_t *e = json_array_get(listener_list, i);
        x->listener_list[i] = *(deserialize_listener(e));
    }

    return x;
}

TSN_Configuration *
deserialize_stream_configuration(json_t *obj)
{
    TSN_Configuration *x = malloc(sizeof(TSN_Configuration));

    x->talker = *(deserialize_status_talker(json_object_get(obj, "talker")));

    json_t *listener_list = json_object_get(obj, "listener_list");
    x->count_listeners = json_array_size(listener_list);
    x->listener_list = (TSN_StatusListener *) malloc(sizeof(TSN_StatusListener) * x->count_listeners);
    for (int i=0; i<x->count_listeners; ++i) {
        json_t *e = json_array_get(listener_list, i);
        x->listener_list[i] = *(deserialize_status_listener(e));
    }

    x->status_stream = *(deserialize_status_stream(json_object_get(obj, "status_stream")));

    return x;
}

TSN_Stream *
deserialize_stream(json_t *obj)
{
    TSN_Stream *stream = malloc(sizeof(TSN_Stream));

    const json_t *stream_id = json_object_get(obj, "stream_id");
    stream->stream_id = strdup(json_string_value(stream_id));

    // Request
    stream->request = *(deserialize_stream_request(json_object_get(obj, "request")));

    // Configuration
    stream->configuration = deserialize_stream_configuration(json_object_get(obj, "configuration"));

    return stream;
}

TSN_Streams *
deserialize_streams(json_t *obj)
{
    TSN_Streams *streams = malloc(sizeof(TSN_Streams));

    uint16_t count_streams = json_number_value(json_object_get(obj, "count_streams"));
    streams->count_streams = count_streams;
    streams->streams = (TSN_Stream *) malloc(sizeof(TSN_Stream) * count_streams);
    json_t *streams_json = json_object_get(obj, "streams");
    for (int i=0; i<count_streams; ++i) {
        json_t *stream = json_array_get(streams_json, i);
        TSN_Stream *s = deserialize_stream(stream);
        streams->streams[i] = *s;
    }

    return streams;
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
    json_object_set_new(root, "has_app", json_integer(enddevice->has_app));
    if (enddevice->has_app) {
        json_object_set_new(root, "app_ref", json_string(enddevice->app_ref));
    }

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
    json_t *has_app;
    json_t *app_ref;

    mac = json_object_get(obj, "mac");
    enddevice->mac = strdup(json_string_value(mac));

    has_app = json_object_get(obj, "has_app");
    enddevice->has_app = json_integer_value(has_app);

    if (enddevice->has_app) {
        app_ref = json_object_get(obj, "app_ref");
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
    json_object_set_new(root, "has_image", json_integer(app->has_image));
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


// ------------------------------------
// Other
// ------------------------------------
json_t *
serialize_event_cb_data(TSN_Event_CB_Data *data)
{
    json_t *root = NULL;
    root = json_object();

    // Event ID
    json_object_set_new(root, "event_id", json_integer(data->event_id));

    // Entry ID
    json_object_set_new(root, "entry_id", json_string(data->entry_id));

    // Message
    json_object_set_new(root, "msg", json_string(data->msg));

    return root;
}

json_t *
serialize_websocket_message(Websocket_Message *ws_msg)
{
    json_t *root = NULL;
    root = json_object();

    // Timestamp
    json_object_set_new(root, "timestamp", json_integer(ws_msg->timestamp));

    // Message
    json_object_set_new(root, "msg", json_string(ws_msg->msg));

    return root;
}
