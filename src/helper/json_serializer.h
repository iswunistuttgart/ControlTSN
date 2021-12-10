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
json_t *serialize_stream(TSN_Stream *stream);
json_t *serialize_streams(TSN_Streams *streams);

// ------------------------------------
// Topology
// ------------------------------------
json_t *serialize_enddevice(TSN_Enddevice *enddevice);
json_t *serialize_switch(TSN_Switch *sw);
json_t *serialize_devices(TSN_Devices *devices);
json_t *serialize_connection(TSN_Connection *connection);
json_t *serialize_graph(TSN_Graph *graph);
json_t *serialize_topology(TSN_Topology *topology);

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

#endif // __JSON_SERIALIZER_H__