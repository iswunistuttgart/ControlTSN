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
add_executable(ConfigurationModule src/modules/configuration/module_configuration.c)
target_link_libraries(ConfigurationModule common ulfius json_serializer open62541)
target_compile_options(ConfigurationModule PRIVATE -Wall -Wextra -pedantic)

##### This is just for Testing purposes #####
add_executable(CNCPrototype src/CNC/cnc_prototype.c)
target_link_libraries(CNCPrototype ulfius json_serializer)
