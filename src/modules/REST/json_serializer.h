#ifndef __JSON_SERIALIZER_H__
#define __JSON_SERIALIZER_H__

#include <jansson.h>
#include "../../type_definitions.h"

// Modules
json_t *serialize_module_data(TSN_Module_Data *module_data);
json_t *serialize_module(TSN_Module *module);
json_t *serialize_modules(TSN_Modules *modules);

// Streams
json_t *serialize_stream(TSN_Stream *stream);
json_t *serialize_streams(TSN_Streams *streams);

#endif // __JSON_SERIALIZER_H__