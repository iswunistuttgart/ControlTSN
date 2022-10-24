cmake_minimum_required(VERSION 3.16)

project(ControlTSN_2)


## The Common lib
add_library(common SHARED src/common.c src/sysrepo/sysrepo_client.c src/type_definitions.c src/logger.c)
target_link_libraries(common sysrepo)

## JSON Serializer
add_library(json_serializer SHARED src/helper/json_serializer.c src/type_definitions.c)
target_link_libraries(json_serializer jansson)

# MODULE MAIN
add_executable(MainModule src/core.c)
target_link_libraries(MainModule common)

# MODULE REST
#add_executable(RESTModule src/modules/REST/module_rest.c src/modules/REST/json_serializer.c)
#target_link_libraries(RESTModule common ulfius jansson)
add_executable(RESTModule src/modules/REST/module_rest.c)
target_link_libraries(RESTModule common ulfius json_serializer)

# MODULE CUC
add_executable(CUCModule src/modules/CUC/module_cuc.c)
target_link_libraries(CUCModule common ulfius open62541 json_serializer)

# MODULE MONITOR
add_executable(MonitorModule src/modules/monitor/module_monitor.c)
target_link_libraries(MonitorModule common)

# MODULE CONTAINER
add_executable(ContainerModule src/modules/container/module_container.c)
target_link_libraries(ContainerModule common ulfius json_serializer)
target_compile_options(ContainerModule PRIVATE -Wall -Wextra -pedantic)

# MODULE CONFIGURATION
find_package(open62541 REQUIRED)

set(GENERATED_SERVER_OUTPUT ${PROJECT_SOURCE_DIR}/src_generated)
set(FILE_NS_DIRPREFIX ${PROJECT_SOURCE_DIR}/nodeset)

ua_generate_nodeset_and_datatypes(
  NAME "endpoint"
  FILE_NS "${FILE_NS_DIRPREFIX}/tsnendpointmodel.xml"
  FILE_BSD "${FILE_NS_DIRPREFIX}/tsnendpointtypes.bsd"
  FILE_CSV "${FILE_NS_DIRPREFIX}/tsnendpointmodel.csv"
  NAMESPACE_MAP "2:https://isw.uni-stuttgart.de/TSNEndpointModel/"
  OUTPUT_DIR "${GENERATED_SERVER_OUTPUT}"
  INTERNAL
)

add_executable(ConfigurationModule
  src/modules/configuration/module_configuration.c
  src_generated/namespace_endpoint_generated.c
  src_generated/types_endpoint_generated.c
)
add_dependencies(ConfigurationModule open62541-generator-ns-endpoint)
target_link_libraries(ConfigurationModule common ulfius json_serializer open62541)
target_compile_options(ConfigurationModule PRIVATE -Wall -Wextra -pedantic)

##### This is just for Testing purposes #####
add_executable(CNCPrototype src/CNC/cnc_prototype.c)
target_link_libraries(CNCPrototype ulfius json_serializer)
