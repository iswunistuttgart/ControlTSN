#ifndef __JSON_SERIALIZER_H__
#define __JSON_SERIALIZER_H__

#include <jansson.h>
#include "../type_definitions.h"

// ------------------------------------
// Modules
// ------------------------------------
json_t *serialize_module_data(TSN_Module_Data *module_data);
json_t *serialize_module(TSN_Module *module);
json_t *serialize_modules(TSN_Modules *modules);

TSN_Module_Data *deserialize_module_data(json_t *obj);
TSN_Module_Data_Entry *deserialize_module_data_entry(json_t *obj);

// ------------------------------------
// Streams
// ------------------------------------
json_t *serialize_stream_id(char *stream_id);
json_t *serialize_status_info(IEEE_StatusInfo *si);
json_t *serialize_interface_id(IEEE_InterfaceId *ii);
//json_t *serialize_status_stream(IEEE_StatusStream *ss);
json_t *serialize_mac_addresses(IEEE_MacAddresses *ma);
json_t *serialize_vlan_tag(IEEE_VlanTag *vt);
json_t *serialize_ipv4_tuple(IEEE_IPv4Tuple *ipv4);
json_t *serialize_ipv6_tuple(IEEE_IPv6Tuple *ipv6);
json_t *serialize_config_list(IEEE_ConfigList *cl);
json_t *serialize_interface_list(IEEE_InterfaceList *il);
json_t *serialize_interface_configuration(IEEE_InterfaceConfiguration *ic);
json_t *serialize_stream_rank(IEEE_StreamRank *stream_rank);
json_t *serialize_data_frame_specification(IEEE_DataFrameSpecification *dfs);
json_t *serialize_interval(IEEE_Interval *i);
json_t *serialize_time_aware(IEEE_TimeAware *ta);
json_t *serialize_traffic_specification(IEEE_TrafficSpecification *ts);
json_t *serialize_user_to_network_requirements(IEEE_UserToNetworkRequirements *utnr);
json_t *serialize_interface_capabilities(IEEE_InterfaceCapabilities *ic);
json_t *serialize_talker(TSN_Talker *talker);
json_t *serialize_listener(TSN_Listener *listener);
json_t *serialize_stream_request(TSN_Request *request);
json_t *serialize_status_talker(TSN_StatusTalker *st);
json_t *serialize_status_listener(TSN_StatusListener *sl);
json_t *serialize_stream_configuration(TSN_Configuration *configuration);
json_t *serialize_stream(TSN_Stream *stream);
json_t *serialize_streams(TSN_Streams *streams);


/**
 * This function generates the correct output the CNC from the OpenCNC projects expects.
 * TODO: Whicht format/data structure is correct? Is this one in the standard yet?
 */
json_t *serialize_cnc_request(TSN_Streams *streams);


IEEE_StatusInfo *deserialize_status_info(json_t *obj);
IEEE_InterfaceId *deserialize_interface_id(json_t *obj);
//IEEE_StatusStream *deserialize_status_stream(json_t *obj);
IEEE_MacAddresses *deserialize_mac_addresses(json_t *obj);
IEEE_VlanTag *deserialize_vlan_tag(json_t *obj);
IEEE_IPv4Tuple *deserialize_ipv4_tuple(json_t *obj);
IEEE_IPv6Tuple *deserialize_ipv6_tuple(json_t *obj);
IEEE_ConfigList *deserialize_config_list(json_t *obj);
IEEE_InterfaceList *deserialize_interface_list(json_t *obj);
IEEE_InterfaceConfiguration *deserialize_interface_configuration(json_t *obj);
IEEE_StreamRank *deserialize_stream_rank(json_t *obj);
IEEE_DataFrameSpecification *deserialize_data_frame_specification(json_t *obj);
IEEE_Interval *deserialize_interval(json_t *obj);
IEEE_TimeAware *deserialize_time_aware(json_t *obj);
IEEE_TrafficSpecification *deserialize_traffic_specification(json_t *obj);
IEEE_UserToNetworkRequirements *deserialize_user_to_network_requirements(json_t *obj);
IEEE_InterfaceCapabilities *deserialize_interface_capabilities(json_t *obj);
TSN_Talker *deserialize_talker(json_t *obj);
TSN_Listener *deserialize_listener(json_t *obj);
TSN_StatusTalker *deserialize_status_talker(json_t *obj);
TSN_StatusListener *deserialize_status_listener(json_t *obj);
TSN_Request *deserialize_stream_request(json_t *obj);
TSN_Configuration *deserialize_stream_configuration(json_t *obj);
TSN_Stream *deserialize_stream(json_t *obj);
TSN_Streams *deserialize_streams(json_t *obj);

// ------------------------------------
// Topology
// ------------------------------------
json_t *serialize_enddevice_app_ref(TSN_Enddevice_AppRef *app_ref);
json_t *serialize_enddevice(TSN_Enddevice *enddevice);
json_t *serialize_switch(TSN_Switch *sw);
json_t *serialize_devices(TSN_Devices *devices);
json_t *serialize_connection(TSN_Connection *connection);
json_t *serialize_graph(TSN_Graph *graph);
json_t *serialize_topology(TSN_Topology *topology);

TSN_Enddevice_AppRef *deserialize_enddevice_app_ref(json_t *obj);
TSN_Enddevice *deserialize_enddevice(json_t *obj);
TSN_Switch *deserialize_switch(json_t *obj);
TSN_Devices *deserialize_devices(json_t *obj);
TSN_Connection *deserialize_connection(json_t *obj);
TSN_Graph *deserialize_graph(json_t *obj);
TSN_Topology *deserialize_topology(json_t *obj);

// ------------------------------------
// Application
// ------------------------------------
json_t *serialize_app_parameter(TSN_App_Parameter *param);
json_t *serialize_app(TSN_App *app);
json_t *serialize_apps(TSN_Apps *apps);
json_t *serialize_image(TSN_Image *image);
json_t *serialize_images(TSN_Images *images);
json_t *serialize_application(TSN_Application *application);

TSN_Images *deserialize_images(json_t *obj, const char *docker_host);
TSN_Apps *deserialize_apps(json_t *obj);
TSN_App *deserialize_app(json_t *obj);

// ------------------------------------
// Other
// ------------------------------------
json_t *serialize_event_cb_data(TSN_Event_CB_Data *data);
json_t *serialize_websocket_message(Websocket_Message *ws_msg);


#endif // __JSON_SERIALIZER_H__
