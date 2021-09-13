#include <stdio.h>
#include <string.h>
#include "type_definitions.h"

TSN_Module_Data_Entry_Type 
sr_enum_to_data_type(char *enum_string)
{
    if (strcmp("binary", enum_string) == 0) {
        return BINARY;
    }
    else if (strcmp("boolean", enum_string) == 0) {
        return BOOLEAN;
    }
    else if (strcmp("decimal64", enum_string) == 0) {
        return DECIMAL64;
    }
    else if (strcmp("instance-identifier", enum_string) == 0) {
        return INSTANCE_IDENTIFIER;
    }
    else if (strcmp("int8", enum_string) == 0) {
        return INT8;
    }
    else if (strcmp("int16", enum_string) == 0) {
        return INT16;
    }
    else if (strcmp("int32", enum_string) == 0) {
        return INT32;
    }
    else if (strcmp("int64", enum_string) == 0) {
        return INT64;
    }
    else if (strcmp("string", enum_string) == 0) {
        return STRING;
    }
    else if (strcmp("uint8", enum_string) == 0) {
        return UINT8;
    }
    else if (strcmp("uint16", enum_string) == 0) {
        return UINT16;
    }
    else if (strcmp("uint32", enum_string) == 0) {
        return UINT32;
    }
    else if (strcmp("uint64", enum_string) == 0) {
        return UINT64;
    }

    return -1;
}

char *
data_type_to_sr_enum(TSN_Module_Data_Entry_Type type)
{
    if (type == BINARY) {
        return "binary";
    }
    else if (type == BOOLEAN) {
        return "boolean";
    }
    else if (type == DECIMAL64) {
        return "decimal64";
    }
    else if (type == INSTANCE_IDENTIFIER) {
        return "instance-identifier";
    }
    else if (type == INT8) {
        return "int8";
    }
    else if (type == INT16) {
        return "int16";
    }
    else if (type == INT32) {
        return "int32";
    }
    else if (type == INT64) {
        return "int64";
    }
    else if (type == STRING) {
        return "string";
    }
    else if (type == UINT8) {
        return "uint8";
    }
    else if (type == UINT16) {
        return "uint16";
    }
    else if (type == UINT32) {
        return "uint32";
    }
    else if (type == UINT64) {
        return "uint64";
    }

    return NULL;
}

TSN_Module_Data_Entry_Value 
sr_data_to_data_value(sr_data_t *data, TSN_Module_Data_Entry_Type type)
{
    TSN_Module_Data_Entry_Value val;

    if (type == BINARY) {
        val.binary_val = data->binary_val;
    }
    else if (type == BOOLEAN) {
        val.boolean_val = data->bool_val;
    }
    else if (type == DECIMAL64) {
        val.decimal64_val = data->decimal64_val;
    }
    else if (type == INSTANCE_IDENTIFIER) {
        val.instance_identifier_val = data->instanceid_val;
    }
    else if (type == INT8) {
        val.int8_val = data->int8_val;
    }
    else if (type == INT16) {
        val.int16_val = data->int16_val;
    }
    else if (type == INT32) {
        val.int32_val = data->int32_val;
    }
    else if (type == INT64) {
        val.int64_val = data->int64_val;
    }
    else if (type == STRING) {
        val.string_val = data->string_val;
    }
    else if (type == UINT8) {
        val.uint8_val = data->uint8_val;
    }
    else if (type == UINT16) {
        val.uint16_val = data->uint16_val;
    }
    else if (type == UINT32) {
        val.uint32_val = data->uint32_val;
    }
    else if (type == UINT64) {
        val.uint64_val = data->uint64_val;
    }

    return val;
}

sr_val_t 
data_value_to_sr_value(TSN_Module_Data_Entry_Value value, TSN_Module_Data_Entry_Type type)
{
    sr_val_t val;

    if (type == BINARY) {
        val.type = SR_BINARY_T;
        val.data.binary_val = value.binary_val;
    }
    else if (type == BOOLEAN) {
        val.type = SR_BOOL_T;
        val.data.bool_val = value.boolean_val;
    }
    else if (type == DECIMAL64) {
        val.type = SR_DECIMAL64_T;
        val.data.decimal64_val = value.decimal64_val;
    }
    else if (type == INSTANCE_IDENTIFIER) {
        val.type = SR_INSTANCEID_T;
        val.data.instanceid_val = value.instance_identifier_val;
    }
    else if (type == INT8) {
        val.type = SR_INT8_T;
        val.data.int8_val = value.int8_val;
    }
    else if (type == INT16) {
        val.type = SR_INT16_T;
        val.data.int16_val = value.int16_val;
    }
    else if (type == INT32) {
        val.type = SR_INT32_T;
        val.data.int32_val = value.int32_val;
    }
    else if (type == INT64) {
        val.type = SR_INT64_T;
        val.data.int64_val = value.int64_val;
    }
    else if (type == STRING) {
        val.type = SR_STRING_T;
        val.data.string_val = value.string_val;
    }
    else if (type == UINT8) {
        val.type = SR_UINT8_T;
        val.data.uint8_val = value.uint8_val;
    }
    else if (type == UINT16) {
        val.type = SR_UINT16_T;
        val.data.uint16_val = value.uint16_val;
    }
    else if (type == UINT32) {
        val.type = SR_UINT32_T;
        val.data.uint32_val = value.uint32_val;
    }
    else if (type == UINT64) {
        val.type = SR_UINT64_T;
        val.data.uint64_val = value.uint64_val;
    }

    return val;
}