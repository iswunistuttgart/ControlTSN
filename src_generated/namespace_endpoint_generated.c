/* WARNING: This is a generated file.
 * Any manual changes will be overwritten. */

#include "namespace_endpoint_generated.h"


/* TSNApplicationParameter - ns=1;i=3008 */

static UA_StatusCode function_namespace_endpoint_generated_0_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_DataTypeAttributes attr = UA_DataTypeAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplicationParameter");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_DATATYPE,
UA_NODEID_NUMERIC(ns[1], 3008LU),
UA_NODEID_NUMERIC(ns[0], 22LU),
UA_NODEID_NUMERIC(ns[0], 45LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplicationParameter"),
 UA_NODEID_NULL,
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_0_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 3008LU)
);
}

/* Default JSON - ns=1;i=5006 */

static UA_StatusCode function_namespace_endpoint_generated_1_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default JSON");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5006LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default JSON"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5006LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3008LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_1_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5006LU)
);
}

/* Default XML - ns=1;i=5005 */

static UA_StatusCode function_namespace_endpoint_generated_2_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default XML");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5005LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default XML"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5005LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3008LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_2_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5005LU)
);
}

/* Default Binary - ns=1;i=5004 */

static UA_StatusCode function_namespace_endpoint_generated_3_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default Binary");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5004LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default Binary"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5004LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3008LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_3_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5004LU)
);
}

/* TSNApplicationEngineeringParameters - ns=1;i=3004 */

static UA_StatusCode function_namespace_endpoint_generated_4_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_DataTypeAttributes attr = UA_DataTypeAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplicationEngineeringParameters");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_DATATYPE,
UA_NODEID_NUMERIC(ns[1], 3004LU),
UA_NODEID_NUMERIC(ns[0], 22LU),
UA_NODEID_NUMERIC(ns[0], 45LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplicationEngineeringParameters"),
 UA_NODEID_NULL,
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_4_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 3004LU)
);
}

/* Default JSON - ns=1;i=5003 */

static UA_StatusCode function_namespace_endpoint_generated_5_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default JSON");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5003LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default JSON"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5003LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3004LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_5_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5003LU)
);
}

/* Default XML - ns=1;i=5002 */

static UA_StatusCode function_namespace_endpoint_generated_6_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default XML");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5002LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default XML"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5002LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3004LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_6_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5002LU)
);
}

/* Default Binary - ns=1;i=5001 */

static UA_StatusCode function_namespace_endpoint_generated_7_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default Binary");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5001LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default Binary"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5001LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3004LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_7_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5001LU)
);
}

/* TSNApplication - ns=1;i=3005 */

static UA_StatusCode function_namespace_endpoint_generated_8_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_DataTypeAttributes attr = UA_DataTypeAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplication");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_DATATYPE,
UA_NODEID_NUMERIC(ns[1], 3005LU),
UA_NODEID_NUMERIC(ns[0], 22LU),
UA_NODEID_NUMERIC(ns[0], 45LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplication"),
 UA_NODEID_NULL,
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_8_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 3005LU)
);
}

/* Default JSON - ns=1;i=5009 */

static UA_StatusCode function_namespace_endpoint_generated_9_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default JSON");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5009LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default JSON"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5009LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3005LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_9_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5009LU)
);
}

/* Default XML - ns=1;i=5008 */

static UA_StatusCode function_namespace_endpoint_generated_10_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default XML");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5008LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default XML"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5008LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3005LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_10_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5008LU)
);
}

/* Default Binary - ns=1;i=5007 */

static UA_StatusCode function_namespace_endpoint_generated_11_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Default Binary");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5007LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_NODEID_NUMERIC(ns[0], 0LU),
UA_QUALIFIEDNAME(ns[0], "Default Binary"),
UA_NODEID_NUMERIC(ns[0], 76LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 5007LU), UA_NODEID_NUMERIC(ns[0], 38LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 3005LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_11_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5007LU)
);
}

/* https://isw.uni-stuttgart.de/TSNEndpointModel/ - ns=1;i=5011 */

static UA_StatusCode function_namespace_endpoint_generated_12_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "https://isw.uni-stuttgart.de/TSNEndpointModel/");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 11715LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "https://isw.uni-stuttgart.de/TSNEndpointModel/"),
UA_NODEID_NUMERIC(ns[0], 11616LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_12_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5011LU)
);
}

/* StaticStringNodeIdPattern - ns=1;i=6018 */

static UA_StatusCode function_namespace_endpoint_generated_13_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
attr.displayName = UA_LOCALIZEDTEXT("", "StaticStringNodeIdPattern");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6018LU),
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "StaticStringNodeIdPattern"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_13_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6018LU)
);
}

/* StaticNumericNodeIdRange - ns=1;i=6017 */

static UA_StatusCode function_namespace_endpoint_generated_14_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
attr.valueRank = 1;
attr.arrayDimensionsSize = 1;
UA_UInt32 arrayDimensions[1];
arrayDimensions[0] = 0;
attr.arrayDimensions = &arrayDimensions[0];
attr.dataType = UA_NODEID_NUMERIC(ns[0], 291LU);
attr.displayName = UA_LOCALIZEDTEXT("", "StaticNumericNodeIdRange");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6017LU),
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "StaticNumericNodeIdRange"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_14_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6017LU)
);
}

/* StaticNodeIdTypes - ns=1;i=6016 */

static UA_StatusCode function_namespace_endpoint_generated_15_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
attr.valueRank = 1;
attr.arrayDimensionsSize = 1;
UA_UInt32 arrayDimensions[1];
arrayDimensions[0] = 0;
attr.arrayDimensions = &arrayDimensions[0];
attr.dataType = UA_NODEID_NUMERIC(ns[0], 256LU);
attr.displayName = UA_LOCALIZEDTEXT("", "StaticNodeIdTypes");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6016LU),
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "StaticNodeIdTypes"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_15_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6016LU)
);
}

/* NamespaceVersion - ns=1;i=6015 */

static UA_StatusCode function_namespace_endpoint_generated_16_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6015_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6015_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6015_variant_DataContents);
*variablenode_ns_1_i_6015_variant_DataContents = UA_STRING_ALLOC("0.0.1");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6015_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "NamespaceVersion");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6015LU),
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "NamespaceVersion"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6015_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_16_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6015LU)
);
}

/* NamespaceUri - ns=1;i=6014 */

static UA_StatusCode function_namespace_endpoint_generated_17_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6014_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6014_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6014_variant_DataContents);
*variablenode_ns_1_i_6014_variant_DataContents = UA_STRING_ALLOC("https://isw.uni-stuttgart.de/TSNEndpointModel/");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6014_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "NamespaceUri");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6014LU),
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "NamespaceUri"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6014_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_17_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6014LU)
);
}

/* NamespacePublicationDate - ns=1;i=6013 */

static UA_StatusCode function_namespace_endpoint_generated_18_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 13LU);
UA_DateTime *variablenode_ns_1_i_6013_variant_DataContents =  UA_DateTime_new();
if (!variablenode_ns_1_i_6013_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_DateTime_init(variablenode_ns_1_i_6013_variant_DataContents);
*variablenode_ns_1_i_6013_variant_DataContents = ( (UA_DateTime)(1663677714000 * UA_DATETIME_MSEC) + UA_DATETIME_UNIX_EPOCH);
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6013_variant_DataContents, &UA_TYPES[UA_TYPES_DATETIME]);
attr.displayName = UA_LOCALIZEDTEXT("", "NamespacePublicationDate");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6013LU),
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "NamespacePublicationDate"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_DateTime_delete(variablenode_ns_1_i_6013_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_18_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6013LU)
);
}

/* IsNamespaceSubset - ns=1;i=6012 */

static UA_StatusCode function_namespace_endpoint_generated_19_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 1LU);
UA_Boolean *variablenode_ns_1_i_6012_variant_DataContents =  UA_Boolean_new();
if (!variablenode_ns_1_i_6012_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_Boolean_init(variablenode_ns_1_i_6012_variant_DataContents);
*variablenode_ns_1_i_6012_variant_DataContents = (UA_Boolean) false;
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6012_variant_DataContents, &UA_TYPES[UA_TYPES_BOOLEAN]);
attr.displayName = UA_LOCALIZEDTEXT("", "IsNamespaceSubset");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6012LU),
UA_NODEID_NUMERIC(ns[1], 5011LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "IsNamespaceSubset"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_Boolean_delete(variablenode_ns_1_i_6012_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_19_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6012LU)
);
}

/* Endpoint - ns=1;i=5010 */

static UA_StatusCode function_namespace_endpoint_generated_20_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "Endpoint");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5010LU),
UA_NODEID_NUMERIC(ns[0], 85LU),
UA_NODEID_NUMERIC(ns[0], 35LU),
UA_QUALIFIEDNAME(ns[1], "Endpoint"),
UA_NODEID_NUMERIC(ns[0], 58LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_20_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5010LU)
);
}

/* applications - ns=1;i=6009 */

static UA_StatusCode function_namespace_endpoint_generated_21_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 3;
attr.valueRank = 1;
attr.arrayDimensionsSize = 1;
UA_UInt32 arrayDimensions[1];
arrayDimensions[0] = 0;
attr.arrayDimensions = &arrayDimensions[0];
attr.dataType = UA_NODEID_NUMERIC(ns[1], 3005LU);
attr.displayName = UA_LOCALIZEDTEXT("", "applications");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6009LU),
UA_NODEID_NUMERIC(ns[1], 5010LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "applications"),
UA_NODEID_NUMERIC(ns[0], 63LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_21_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6009LU)
);
}

/* system - ns=1;i=5013 */

static UA_StatusCode function_namespace_endpoint_generated_22_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "system");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5013LU),
UA_NODEID_NUMERIC(ns[1], 5010LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "system"),
UA_NODEID_NUMERIC(ns[0], 58LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_22_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5013LU)
);
}

/* network - ns=1;i=5012 */

static UA_StatusCode function_namespace_endpoint_generated_23_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_ObjectAttributes attr = UA_ObjectAttributes_default;
attr.displayName = UA_LOCALIZEDTEXT("", "network");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT,
UA_NODEID_NUMERIC(ns[1], 5012LU),
UA_NODEID_NUMERIC(ns[1], 5010LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "network"),
UA_NODEID_NUMERIC(ns[0], 58LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],NULL, NULL);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_23_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 5012LU)
);
}

/* TypeDictionary - ns=1;i=6003 */
static const UA_Byte variablenode_ns_1_i_6003_variant_DataContents_byteArray[3501] = {60, 120, 115, 58, 115, 99, 104, 101, 109, 97, 32, 101, 108, 101, 109, 101, 110, 116, 70, 111, 114, 109, 68, 101, 102, 97, 117, 108, 116, 61, 34, 113, 117, 97, 108, 105, 102, 105, 101, 100, 34, 32, 116, 97, 114, 103, 101, 116, 78, 97, 109, 101, 115, 112, 97, 99, 101, 61, 34, 104, 116, 116, 112, 115, 58, 47, 47, 105, 115, 119, 46, 117, 110, 105, 45, 115, 116, 117, 116, 116, 103, 97, 114, 116, 46, 100, 101, 47, 84, 83, 78, 69, 110, 100, 112, 111, 105, 110, 116, 77, 111, 100, 101, 108, 47, 84, 121, 112, 101, 115, 46, 120, 115, 100, 34, 32, 120, 109, 108, 110, 115, 58, 116, 110, 115, 61, 34, 104, 116, 116, 112, 115, 58, 47, 47, 105, 115, 119, 46, 117, 110, 105, 45, 115, 116, 117, 116, 116, 103, 97, 114, 116, 46, 100, 101, 47, 84, 83, 78, 69, 110, 100, 112, 111, 105, 110, 116, 77, 111, 100, 101, 108, 47, 84, 121, 112, 101, 115, 46, 120, 115, 100, 34, 32, 120, 109, 108, 110, 115, 58, 117, 97, 61, 34, 104, 116, 116, 112, 58, 47, 47, 111, 112, 99, 102, 111, 117, 110, 100, 97, 116, 105, 111, 110, 46, 111, 114, 103, 47, 85, 65, 47, 50, 48, 48, 56, 47, 48, 50, 47, 84, 121, 112, 101, 115, 46, 120, 115, 100, 34, 32, 120, 109, 108, 110, 115, 58, 120, 115, 61, 34, 104, 116, 116, 112, 58, 47, 47, 119, 119, 119, 46, 119, 51, 46, 111, 114, 103, 47, 50, 48, 48, 49, 47, 88, 77, 76, 83, 99, 104, 101, 109, 97, 34, 62, 10, 32, 60, 120, 115, 58, 105, 109, 112, 111, 114, 116, 32, 110, 97, 109, 101, 115, 112, 97, 99, 101, 61, 34, 104, 116, 116, 112, 58, 47, 47, 111, 112, 99, 102, 111, 117, 110, 100, 97, 116, 105, 111, 110, 46, 111, 114, 103, 47, 85, 65, 47, 50, 48, 48, 56, 47, 48, 50, 47, 84, 121, 112, 101, 115, 46, 120, 115, 100, 34, 47, 62, 10, 32, 60, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 62, 10, 32, 32, 60, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 73, 68, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 80, 97, 116, 104, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 32, 110, 97, 109, 101, 61, 34, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 110, 97, 109, 101, 61, 34, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 47, 62, 10, 32, 32, 60, 47, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 60, 47, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 62, 10, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 47, 62, 10, 32, 60, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 32, 110, 97, 109, 101, 61, 34, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 62, 10, 32, 32, 60, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 117, 110, 98, 111, 117, 110, 100, 101, 100, 34, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 32, 110, 105, 108, 108, 97, 98, 108, 101, 61, 34, 116, 114, 117, 101, 34, 47, 62, 10, 32, 32, 60, 47, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 60, 47, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 62, 10, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 32, 110, 97, 109, 101, 61, 34, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 32, 110, 105, 108, 108, 97, 98, 108, 101, 61, 34, 116, 114, 117, 101, 34, 47, 62, 10, 32, 60, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 62, 10, 32, 32, 60, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 67, 111, 109, 109, 97, 110, 100, 101, 100, 83, 116, 97, 116, 117, 115, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 67, 117, 114, 114, 101, 110, 116, 83, 116, 97, 116, 117, 115, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 98, 111, 111, 108, 101, 97, 110, 34, 32, 110, 97, 109, 101, 61, 34, 83, 101, 110, 100, 82, 101, 99, 101, 105, 118, 101, 69, 110, 97, 98, 108, 101, 100, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 117, 110, 115, 105, 103, 110, 101, 100, 76, 111, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 66, 97, 115, 101, 116, 105, 109, 101, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 105, 110, 116, 34, 32, 110, 97, 109, 101, 61, 34, 81, 98, 118, 79, 102, 102, 115, 101, 116, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 68, 101, 115, 116, 105, 110, 97, 116, 105, 111, 110, 77, 65, 67, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 83, 117, 98, 115, 99, 114, 105, 98, 101, 100, 77, 65, 67, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 73, 110, 116, 101, 114, 102, 97, 99, 101, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 100, 111, 117, 98, 108, 101, 34, 32, 110, 97, 109, 101, 61, 34, 67, 121, 99, 108, 101, 84, 105, 109, 101, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 105, 110, 116, 34, 32, 110, 97, 109, 101, 61, 34, 83, 111, 99, 107, 101, 116, 80, 114, 105, 111, 114, 105, 116, 121, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 117, 110, 115, 105, 103, 110, 101, 100, 73, 110, 116, 34, 32, 110, 97, 109, 101, 61, 34, 87, 67, 69, 84, 34, 47, 62, 10, 32, 32, 60, 47, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 60, 47, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 62, 10, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 47, 62, 10, 32, 60, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 32, 110, 97, 109, 101, 61, 34, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 62, 10, 32, 32, 60, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 117, 110, 98, 111, 117, 110, 100, 101, 100, 34, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 110, 105, 108, 108, 97, 98, 108, 101, 61, 34, 116, 114, 117, 101, 34, 47, 62, 10, 32, 32, 60, 47, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 60, 47, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 62, 10, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 110, 97, 109, 101, 61, 34, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 110, 105, 108, 108, 97, 98, 108, 101, 61, 34, 116, 114, 117, 101, 34, 47, 62, 10, 32, 60, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 62, 10, 32, 32, 60, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 78, 97, 109, 101, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 84, 121, 112, 101, 34, 47, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 49, 34, 32, 116, 121, 112, 101, 61, 34, 120, 115, 58, 115, 116, 114, 105, 110, 103, 34, 32, 110, 97, 109, 101, 61, 34, 86, 97, 108, 117, 101, 34, 47, 62, 10, 32, 32, 60, 47, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 60, 47, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 62, 10, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 47, 62, 10, 32, 60, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 32, 110, 97, 109, 101, 61, 34, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 62, 10, 32, 32, 60, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 32, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 109, 105, 110, 79, 99, 99, 117, 114, 115, 61, 34, 48, 34, 32, 109, 97, 120, 79, 99, 99, 117, 114, 115, 61, 34, 117, 110, 98, 111, 117, 110, 100, 101, 100, 34, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 32, 110, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 32, 110, 105, 108, 108, 97, 98, 108, 101, 61, 34, 116, 114, 117, 101, 34, 47, 62, 10, 32, 32, 60, 47, 120, 115, 58, 115, 101, 113, 117, 101, 110, 99, 101, 62, 10, 32, 60, 47, 120, 115, 58, 99, 111, 109, 112, 108, 101, 120, 84, 121, 112, 101, 62, 10, 32, 60, 120, 115, 58, 101, 108, 101, 109, 101, 110, 116, 32, 116, 121, 112, 101, 61, 34, 116, 110, 115, 58, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 32, 110, 97, 109, 101, 61, 34, 76, 105, 115, 116, 79, 102, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 32, 110, 105, 108, 108, 97, 98, 108, 101, 61, 34, 116, 114, 117, 101, 34, 47, 62, 10, 60, 47, 120, 115, 58, 115, 99, 104, 101, 109, 97, 62, 10};



static UA_StatusCode function_namespace_endpoint_generated_24_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 15LU);
UA_ByteString *variablenode_ns_1_i_6003_variant_DataContents =  UA_ByteString_new();
if (!variablenode_ns_1_i_6003_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_ByteString_init(variablenode_ns_1_i_6003_variant_DataContents);
variablenode_ns_1_i_6003_variant_DataContents->length = 3501;
variablenode_ns_1_i_6003_variant_DataContents->data = (UA_Byte *)(void*)(uintptr_t)variablenode_ns_1_i_6003_variant_DataContents_byteArray;
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6003_variant_DataContents, &UA_TYPES[UA_TYPES_BYTESTRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TypeDictionary");
#ifdef UA_ENABLE_NODESET_COMPILER_DESCRIPTIONS
attr.description = UA_LOCALIZEDTEXT("", "Collects the data type descriptions of https://isw.uni-stuttgart.de/TSNEndpointModel/");
#endif
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6003LU),
UA_NODEID_NUMERIC(ns[0], 92LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TypeDictionary"),
UA_NODEID_NUMERIC(ns[0], 72LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
variablenode_ns_1_i_6003_variant_DataContents->data = NULL;
variablenode_ns_1_i_6003_variant_DataContents->length = 0;
UA_ByteString_delete(variablenode_ns_1_i_6003_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_24_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6003LU)
);
}

/* TSNApplication - ns=1;i=6011 */

static UA_StatusCode function_namespace_endpoint_generated_25_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6011_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6011_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6011_variant_DataContents);
*variablenode_ns_1_i_6011_variant_DataContents = UA_STRING_ALLOC("//xs:element[@name='TSNApplication']");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6011_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplication");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6011LU),
UA_NODEID_NUMERIC(ns[1], 6003LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplication"),
UA_NODEID_NUMERIC(ns[0], 69LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6011_variant_DataContents);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 6011LU), UA_NODEID_NUMERIC(ns[0], 39LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 5008LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_25_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6011LU)
);
}

/* TSNApplicationParameter - ns=1;i=6008 */

static UA_StatusCode function_namespace_endpoint_generated_26_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6008_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6008_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6008_variant_DataContents);
*variablenode_ns_1_i_6008_variant_DataContents = UA_STRING_ALLOC("//xs:element[@name='TSNApplicationParameter']");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6008_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplicationParameter");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6008LU),
UA_NODEID_NUMERIC(ns[1], 6003LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplicationParameter"),
UA_NODEID_NUMERIC(ns[0], 69LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6008_variant_DataContents);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 6008LU), UA_NODEID_NUMERIC(ns[0], 39LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 5005LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_26_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6008LU)
);
}

/* TSNApplicationEngineeringParameters - ns=1;i=6006 */

static UA_StatusCode function_namespace_endpoint_generated_27_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6006_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6006_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6006_variant_DataContents);
*variablenode_ns_1_i_6006_variant_DataContents = UA_STRING_ALLOC("//xs:element[@name='TSNApplicationEngineeringParameters']");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6006_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplicationEngineeringParameters");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6006LU),
UA_NODEID_NUMERIC(ns[1], 6003LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplicationEngineeringParameters"),
UA_NODEID_NUMERIC(ns[0], 69LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6006_variant_DataContents);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 6006LU), UA_NODEID_NUMERIC(ns[0], 39LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 5002LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_27_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6006LU)
);
}

/* NamespaceUri - ns=1;i=6004 */

static UA_StatusCode function_namespace_endpoint_generated_28_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6004_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6004_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6004_variant_DataContents);
*variablenode_ns_1_i_6004_variant_DataContents = UA_STRING_ALLOC("https://isw.uni-stuttgart.de/TSNEndpointModel/Types.xsd");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6004_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "NamespaceUri");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6004LU),
UA_NODEID_NUMERIC(ns[1], 6003LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "NamespaceUri"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6004_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_28_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6004LU)
);
}

/* TypeDictionary - ns=1;i=6001 */
static const UA_Byte variablenode_ns_1_i_6001_variant_DataContents_byteArray[1902] = {60, 111, 112, 99, 58, 84, 121, 112, 101, 68, 105, 99, 116, 105, 111, 110, 97, 114, 121, 32, 120, 109, 108, 110, 115, 58, 120, 115, 105, 61, 34, 104, 116, 116, 112, 58, 47, 47, 119, 119, 119, 46, 119, 51, 46, 111, 114, 103, 47, 50, 48, 48, 49, 47, 88, 77, 76, 83, 99, 104, 101, 109, 97, 45, 105, 110, 115, 116, 97, 110, 99, 101, 34, 32, 120, 109, 108, 110, 115, 58, 116, 110, 115, 61, 34, 104, 116, 116, 112, 115, 58, 47, 47, 105, 115, 119, 46, 117, 110, 105, 45, 115, 116, 117, 116, 116, 103, 97, 114, 116, 46, 100, 101, 47, 84, 83, 78, 69, 110, 100, 112, 111, 105, 110, 116, 77, 111, 100, 101, 108, 47, 34, 32, 68, 101, 102, 97, 117, 108, 116, 66, 121, 116, 101, 79, 114, 100, 101, 114, 61, 34, 76, 105, 116, 116, 108, 101, 69, 110, 100, 105, 97, 110, 34, 32, 120, 109, 108, 110, 115, 58, 111, 112, 99, 61, 34, 104, 116, 116, 112, 58, 47, 47, 111, 112, 99, 102, 111, 117, 110, 100, 97, 116, 105, 111, 110, 46, 111, 114, 103, 47, 66, 105, 110, 97, 114, 121, 83, 99, 104, 101, 109, 97, 47, 34, 32, 120, 109, 108, 110, 115, 58, 117, 97, 61, 34, 104, 116, 116, 112, 58, 47, 47, 111, 112, 99, 102, 111, 117, 110, 100, 97, 116, 105, 111, 110, 46, 111, 114, 103, 47, 85, 65, 47, 34, 32, 84, 97, 114, 103, 101, 116, 78, 97, 109, 101, 115, 112, 97, 99, 101, 61, 34, 104, 116, 116, 112, 115, 58, 47, 47, 105, 115, 119, 46, 117, 110, 105, 45, 115, 116, 117, 116, 116, 103, 97, 114, 116, 46, 100, 101, 47, 84, 83, 78, 69, 110, 100, 112, 111, 105, 110, 116, 77, 111, 100, 101, 108, 47, 34, 62, 10, 32, 60, 111, 112, 99, 58, 73, 109, 112, 111, 114, 116, 32, 78, 97, 109, 101, 115, 112, 97, 99, 101, 61, 34, 104, 116, 116, 112, 58, 47, 47, 111, 112, 99, 102, 111, 117, 110, 100, 97, 116, 105, 111, 110, 46, 111, 114, 103, 47, 85, 65, 47, 34, 47, 62, 10, 32, 60, 111, 112, 99, 58, 83, 116, 114, 117, 99, 116, 117, 114, 101, 100, 84, 121, 112, 101, 32, 66, 97, 115, 101, 84, 121, 112, 101, 61, 34, 117, 97, 58, 69, 120, 116, 101, 110, 115, 105, 111, 110, 79, 98, 106, 101, 99, 116, 34, 32, 78, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 34, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 73, 68, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 80, 97, 116, 104, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 73, 110, 116, 51, 50, 34, 32, 78, 97, 109, 101, 61, 34, 78, 111, 79, 102, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 76, 101, 110, 103, 116, 104, 70, 105, 101, 108, 100, 61, 34, 78, 111, 79, 102, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 32, 78, 97, 109, 101, 61, 34, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 116, 110, 115, 58, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 32, 78, 97, 109, 101, 61, 34, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 47, 62, 10, 32, 60, 47, 111, 112, 99, 58, 83, 116, 114, 117, 99, 116, 117, 114, 101, 100, 84, 121, 112, 101, 62, 10, 32, 60, 111, 112, 99, 58, 83, 116, 114, 117, 99, 116, 117, 114, 101, 100, 84, 121, 112, 101, 32, 66, 97, 115, 101, 84, 121, 112, 101, 61, 34, 117, 97, 58, 69, 120, 116, 101, 110, 115, 105, 111, 110, 79, 98, 106, 101, 99, 116, 34, 32, 78, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 69, 110, 103, 105, 110, 101, 101, 114, 105, 110, 103, 80, 97, 114, 97, 109, 101, 116, 101, 114, 115, 34, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 67, 111, 109, 109, 97, 110, 100, 101, 100, 83, 116, 97, 116, 117, 115, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 67, 117, 114, 114, 101, 110, 116, 83, 116, 97, 116, 117, 115, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 66, 111, 111, 108, 101, 97, 110, 34, 32, 78, 97, 109, 101, 61, 34, 83, 101, 110, 100, 82, 101, 99, 101, 105, 118, 101, 69, 110, 97, 98, 108, 101, 100, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 85, 73, 110, 116, 54, 52, 34, 32, 78, 97, 109, 101, 61, 34, 66, 97, 115, 101, 116, 105, 109, 101, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 73, 110, 116, 51, 50, 34, 32, 78, 97, 109, 101, 61, 34, 81, 98, 118, 79, 102, 102, 115, 101, 116, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 68, 101, 115, 116, 105, 110, 97, 116, 105, 111, 110, 77, 65, 67, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 83, 117, 98, 115, 99, 114, 105, 98, 101, 100, 77, 65, 67, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 73, 110, 116, 101, 114, 102, 97, 99, 101, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 68, 111, 117, 98, 108, 101, 34, 32, 78, 97, 109, 101, 61, 34, 67, 121, 99, 108, 101, 84, 105, 109, 101, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 73, 110, 116, 51, 50, 34, 32, 78, 97, 109, 101, 61, 34, 83, 111, 99, 107, 101, 116, 80, 114, 105, 111, 114, 105, 116, 121, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 85, 73, 110, 116, 51, 50, 34, 32, 78, 97, 109, 101, 61, 34, 87, 67, 69, 84, 34, 47, 62, 10, 32, 60, 47, 111, 112, 99, 58, 83, 116, 114, 117, 99, 116, 117, 114, 101, 100, 84, 121, 112, 101, 62, 10, 32, 60, 111, 112, 99, 58, 83, 116, 114, 117, 99, 116, 117, 114, 101, 100, 84, 121, 112, 101, 32, 66, 97, 115, 101, 84, 121, 112, 101, 61, 34, 117, 97, 58, 69, 120, 116, 101, 110, 115, 105, 111, 110, 79, 98, 106, 101, 99, 116, 34, 32, 78, 97, 109, 101, 61, 34, 84, 83, 78, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 80, 97, 114, 97, 109, 101, 116, 101, 114, 34, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 78, 97, 109, 101, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 84, 121, 112, 101, 34, 47, 62, 10, 32, 32, 60, 111, 112, 99, 58, 70, 105, 101, 108, 100, 32, 84, 121, 112, 101, 78, 97, 109, 101, 61, 34, 111, 112, 99, 58, 67, 104, 97, 114, 65, 114, 114, 97, 121, 34, 32, 78, 97, 109, 101, 61, 34, 86, 97, 108, 117, 101, 34, 47, 62, 10, 32, 60, 47, 111, 112, 99, 58, 83, 116, 114, 117, 99, 116, 117, 114, 101, 100, 84, 121, 112, 101, 62, 10, 60, 47, 111, 112, 99, 58, 84, 121, 112, 101, 68, 105, 99, 116, 105, 111, 110, 97, 114, 121, 62, 10};



static UA_StatusCode function_namespace_endpoint_generated_29_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 15LU);
UA_ByteString *variablenode_ns_1_i_6001_variant_DataContents =  UA_ByteString_new();
if (!variablenode_ns_1_i_6001_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_ByteString_init(variablenode_ns_1_i_6001_variant_DataContents);
variablenode_ns_1_i_6001_variant_DataContents->length = 1902;
variablenode_ns_1_i_6001_variant_DataContents->data = (UA_Byte *)(void*)(uintptr_t)variablenode_ns_1_i_6001_variant_DataContents_byteArray;
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6001_variant_DataContents, &UA_TYPES[UA_TYPES_BYTESTRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TypeDictionary");
#ifdef UA_ENABLE_NODESET_COMPILER_DESCRIPTIONS
attr.description = UA_LOCALIZEDTEXT("", "Collects the data type descriptions of https://isw.uni-stuttgart.de/TSNEndpointModel/");
#endif
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6001LU),
UA_NODEID_NUMERIC(ns[0], 93LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TypeDictionary"),
UA_NODEID_NUMERIC(ns[0], 72LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
variablenode_ns_1_i_6001_variant_DataContents->data = NULL;
variablenode_ns_1_i_6001_variant_DataContents->length = 0;
UA_ByteString_delete(variablenode_ns_1_i_6001_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_29_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6001LU)
);
}

/* TSNApplication - ns=1;i=6010 */

static UA_StatusCode function_namespace_endpoint_generated_30_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6010_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6010_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6010_variant_DataContents);
*variablenode_ns_1_i_6010_variant_DataContents = UA_STRING_ALLOC("TSNApplication");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6010_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplication");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6010LU),
UA_NODEID_NUMERIC(ns[1], 6001LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplication"),
UA_NODEID_NUMERIC(ns[0], 69LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6010_variant_DataContents);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 6010LU), UA_NODEID_NUMERIC(ns[0], 39LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 5007LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_30_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6010LU)
);
}

/* TSNApplicationParameter - ns=1;i=6007 */

static UA_StatusCode function_namespace_endpoint_generated_31_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6007_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6007_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6007_variant_DataContents);
*variablenode_ns_1_i_6007_variant_DataContents = UA_STRING_ALLOC("TSNApplicationParameter");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6007_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplicationParameter");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6007LU),
UA_NODEID_NUMERIC(ns[1], 6001LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplicationParameter"),
UA_NODEID_NUMERIC(ns[0], 69LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6007_variant_DataContents);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 6007LU), UA_NODEID_NUMERIC(ns[0], 39LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 5004LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_31_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6007LU)
);
}

/* TSNApplicationEngineeringParameters - ns=1;i=6005 */

static UA_StatusCode function_namespace_endpoint_generated_32_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6005_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6005_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6005_variant_DataContents);
*variablenode_ns_1_i_6005_variant_DataContents = UA_STRING_ALLOC("TSNApplicationEngineeringParameters");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6005_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "TSNApplicationEngineeringParameters");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6005LU),
UA_NODEID_NUMERIC(ns[1], 6001LU),
UA_NODEID_NUMERIC(ns[0], 47LU),
UA_QUALIFIEDNAME(ns[1], "TSNApplicationEngineeringParameters"),
UA_NODEID_NUMERIC(ns[0], 69LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6005_variant_DataContents);
retVal |= UA_Server_addReference(server, UA_NODEID_NUMERIC(ns[1], 6005LU), UA_NODEID_NUMERIC(ns[0], 39LU), UA_EXPANDEDNODEID_NUMERIC(ns[1], 5001LU), false);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_32_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6005LU)
);
}

/* NamespaceUri - ns=1;i=6002 */

static UA_StatusCode function_namespace_endpoint_generated_33_begin(UA_Server *server, UA_UInt16* ns) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
UA_VariableAttributes attr = UA_VariableAttributes_default;
attr.minimumSamplingInterval = 0.000000;
attr.userAccessLevel = 1;
attr.accessLevel = 1;
/* Value rank inherited */
attr.valueRank = -2;
attr.dataType = UA_NODEID_NUMERIC(ns[0], 12LU);
UA_String *variablenode_ns_1_i_6002_variant_DataContents =  UA_String_new();
if (!variablenode_ns_1_i_6002_variant_DataContents) return UA_STATUSCODE_BADOUTOFMEMORY;
UA_String_init(variablenode_ns_1_i_6002_variant_DataContents);
*variablenode_ns_1_i_6002_variant_DataContents = UA_STRING_ALLOC("https://isw.uni-stuttgart.de/TSNEndpointModel/");
UA_Variant_setScalar(&attr.value, variablenode_ns_1_i_6002_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
attr.displayName = UA_LOCALIZEDTEXT("", "NamespaceUri");
retVal |= UA_Server_addNode_begin(server, UA_NODECLASS_VARIABLE,
UA_NODEID_NUMERIC(ns[1], 6002LU),
UA_NODEID_NUMERIC(ns[1], 6001LU),
UA_NODEID_NUMERIC(ns[0], 46LU),
UA_QUALIFIEDNAME(ns[0], "NamespaceUri"),
UA_NODEID_NUMERIC(ns[0], 68LU),
(const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],NULL, NULL);
UA_String_delete(variablenode_ns_1_i_6002_variant_DataContents);
return retVal;
}

static UA_StatusCode function_namespace_endpoint_generated_33_finish(UA_Server *server, UA_UInt16* ns) {
return UA_Server_addNode_finish(server, 
UA_NODEID_NUMERIC(ns[1], 6002LU)
);
}

static UA_DataTypeArray customUA_TYPES_ENDPOINT = {
    NULL,
    UA_TYPES_ENDPOINT_COUNT,
    UA_TYPES_ENDPOINT
};

UA_StatusCode namespace_endpoint_generated(UA_Server *server) {
UA_StatusCode retVal = UA_STATUSCODE_GOOD;
/* Use namespace ids generated by the server */
UA_UInt16 ns[2];
ns[0] = UA_Server_addNamespace(server, "http://opcfoundation.org/UA/");
ns[1] = UA_Server_addNamespace(server, "https://isw.uni-stuttgart.de/TSNEndpointModel/");

/* Load custom datatype definitions into the server */
if(UA_TYPES_ENDPOINT_COUNT > 0) {
customUA_TYPES_ENDPOINT.next = UA_Server_getConfig(server)->customDataTypes;
UA_Server_getConfig(server)->customDataTypes = &customUA_TYPES_ENDPOINT;

}
if((retVal = function_namespace_endpoint_generated_0_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_1_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_2_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_3_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_4_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_5_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_6_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_7_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_8_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_9_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_10_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_11_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_12_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_13_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_14_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_15_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_16_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_17_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_18_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_19_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_20_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_21_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_22_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_23_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_24_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_25_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_26_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_27_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_28_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_29_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_30_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_31_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_32_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_33_begin(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_33_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_32_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_31_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_30_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_29_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_28_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_27_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_26_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_25_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_24_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_23_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_22_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_21_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_20_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_19_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_18_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_17_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_16_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_15_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_14_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_13_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_12_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_11_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_10_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_9_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_8_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_7_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_6_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_5_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_4_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_3_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_2_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_1_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
if((retVal = function_namespace_endpoint_generated_0_finish(server, ns)) != UA_STATUSCODE_GOOD) return retVal;
return retVal;
}
