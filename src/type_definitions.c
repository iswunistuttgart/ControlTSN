/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#include <stdio.h>
#include <string.h>
#include "type_definitions.h"

TSN_Module_Data_Entry_Type
string_to_data_type(const char *enum_string)
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
data_type_to_string(TSN_Module_Data_Entry_Type type)
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

static char *
_data_value_to_string(TSN_Module_Data_Entry_Type type, TSN_Module_Data_Entry_Value value)
{
    char *str_val = NULL;

    if (type == BINARY) {
        str_val = value.binary_val;
    }
    else if (type == BOOLEAN) {
        str_val = value.boolean_val ? strdup("TRUE") : strdup("FALSE");
    }
    else if (type == DECIMAL64) {
        size_t size_needed = snprintf(NULL, 0, "%g", value.decimal64_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%g", value.decimal64_val);
    }
    else if (type == INSTANCE_IDENTIFIER) {
        str_val = value.instance_identifier_val;
    }
    else if (type == INT8) {
        size_t size_needed = snprintf(NULL, 0, "%d", value.int8_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%d", value.int8_val);
    }
    else if (type == INT16) {
        size_t size_needed = snprintf(NULL, 0, "%d", value.int16_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%d", value.int16_val);
    }
    else if (type == INT32) {
        size_t size_needed = snprintf(NULL, 0, "%d", value.int32_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%d", value.int32_val);
    }
    else if (type == INT64) {
        size_t size_needed = snprintf(NULL, 0, "%ld", value.int64_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%ld", value.int64_val);
    }
    else if (type == STRING) {
        str_val = value.string_val;
    }
    else if (type == UINT8) {
        size_t size_needed = snprintf(NULL, 0, "%u", value.uint8_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%u", value.uint8_val);
    }
    else if (type == UINT16) {
        size_t size_needed = snprintf(NULL, 0, "%u", value.uint16_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%u", value.uint16_val);
    }
    else if (type == UINT32) {
        size_t size_needed = snprintf(NULL, 0, "%u", value.uint32_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%u", value.uint32_val);
    }
    else if (type == UINT64) {
        size_t size_needed = snprintf(NULL, 0, "%lu", value.uint64_val) + 1;
        str_val = malloc(size_needed);
        sprintf(str_val, "%lu", value.uint64_val);
    }

    return str_val;
}

char *
data_value_to_string(TSN_Module_Data_Entry_Type type, TSN_Module_Data_Entry_Value value)
{
    return _data_value_to_string(type, value);
}

char *
parameter_data_value_to_string(TSN_App_Parameter *parameter)
{
    return _data_value_to_string(parameter->type, parameter->value);
}

TSN_Module_Data_Entry_Value
sysrepo_data_to_data_value(sr_data_t data, TSN_Module_Data_Entry_Type type)
{
    TSN_Module_Data_Entry_Value val;

    if (type == BINARY) {
        val.binary_val = strdup(data.binary_val);
    }
    else if (type == BOOLEAN) {
        val.boolean_val = data.bool_val;
    }
    else if (type == DECIMAL64) {
        //val.decimal64_val = data.decimal64_val;

        // TODO: There is a problem with handling of dec64 values in sysrepo... Maybe use the internal libyang api instead to write and read these param values?
        // Somehow the decimal64 value is stored with the type string...
        val.decimal64_val = strtod(data.string_val, NULL);
    }
    else if (type == INSTANCE_IDENTIFIER) {
        val.instance_identifier_val = strdup(data.instanceid_val);
    }
    else if (type == INT8) {
        val.int8_val = data.int8_val;
    }
    else if (type == INT16) {
        val.int16_val = data.int16_val;
    }
    else if (type == INT32) {
        val.int32_val = data.int32_val;
    }
    else if (type == INT64) {
        val.int64_val = data.int64_val;
    }
    else if (type == STRING) {
        val.string_val = strdup(data.string_val);
    }
    else if (type == UINT8) {
        val.uint8_val = data.uint8_val;
    }
    else if (type == UINT16) {
        val.uint16_val = data.uint16_val;
    }
    else if (type == UINT32) {
        val.uint32_val = data.uint32_val;
    }
    else if (type == UINT64) {
        val.uint64_val = data.uint64_val;
    }

    /*
    val.binary_val = data.binary_val;
    val.boolean_val = data.bool_val;
    val.decimal64_val = data.decimal64_val;
    val.instance_identifier_val = data.instanceid_val;
    val.int8_val = data.int8_val;
    val.int16_val = data.int16_val;
    val.int32_val = data.int32_val;
    val.int64_val = data.int64_val;
    val.string_val = data.string_val;
    val.uint8_val = data.uint8_val;
    val.uint16_val = data.uint16_val;
    val.uint32_val = data.uint32_val;
    val.uint64_val = data.uint64_val;
    */

    return val;
}

TSN_Module_Data_Entry_Value sysrepo_value_to_data_value(sr_val_t value)
{
    TSN_Module_Data_Entry_Value val = { };

    if (value.type == SR_BINARY_T) {
        val.binary_val = strdup(value.data.binary_val);
    }
    else if (value.type == SR_BOOL_T) {
        val.boolean_val = value.data.bool_val;
    }
    else if (value.type == SR_DECIMAL64_T) {
        val.decimal64_val = value.data.decimal64_val;
    }
    else if (value.type == SR_INSTANCEID_T) {
        val.instance_identifier_val = strdup(value.data.instanceid_val);
    }
    else if (value.type == SR_INT8_T) {
        val.int8_val = value.data.int8_val;
    }
    else if (value.type == SR_INT16_T) {
        val.int16_val = value.data.int16_val;
    }
    else if (value.type == SR_INT32_T) {
        val.int32_val = value.data.int32_val;
    }
    else if (value.type == SR_INT64_T) {
        val.int64_val = value.data.int64_val;
    }
    else if (value.type == SR_STRING_T) {
        val.string_val = strdup(value.data.string_val);
    }
    else if (value.type == SR_UINT8_T) {
        val.uint8_val = value.data.uint8_val;
    }
    else if (value.type == SR_UINT16_T) {
        val.uint16_val = value.data.uint16_val;
    }
    else if (value.type == SR_UINT32_T) {
        val.uint32_val = value.data.uint32_val;
    }
    else if (value.type == SR_UINT64_T) {
        val.uint64_val = value.data.uint64_val;
    }

    return val;
}

TSN_Module_Data_Entry_Type sysrepo_value_to_data_type(sr_val_t value)
{
    TSN_Module_Data_Entry_Type type;

    switch (value.type) {
    case SR_BINARY_T:
        type = BINARY;
        break;
    case SR_BOOL_T:
        type = BOOLEAN;
        break;
    case SR_DECIMAL64_T:
        type = DECIMAL64;
        break;
    case SR_INSTANCEID_T:
        type = INSTANCE_IDENTIFIER;
        break;
    case SR_INT8_T:
        type = INT8;
        break;
    case SR_INT16_T:
        type = INT16;
        break;
    case SR_INT32_T:
        type = INT32;
        break;
    case SR_INT64_T:
        type = INT64;
        break;
    case SR_STRING_T:
        type = STRING;
        break;
    case SR_UINT8_T:
        type = UINT8;
        break;
    case SR_UINT16_T:
        type = UINT16;
        break;
    case SR_UINT32_T:
        type = UINT32;
        break;
    case SR_UINT64_T:
        type = UINT64;
        break;
    default:
        type = STRING;
    }

    return type;
}

sr_val_t
data_value_to_sysrepo_value(TSN_Module_Data_Entry_Value value, TSN_Module_Data_Entry_Type type)
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
