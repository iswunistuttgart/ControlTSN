cmake_minimum_required(VERSION 3.16)

project(ControlTSN_2)

# Set C standard and enable warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall")

## Check for dependencies
find_package(PkgConfig REQUIRED)

pkg_search_module(ULFIUS libulfius REQUIRED)
pkg_search_module(SYSREPO sysrepo REQUIRED)
pkg_search_module(JANSSON jansson REQUIRED)

## The Common lib
add_library(common SHARED src/common.c src/sysrepo/sysrepo_client.c src/type_definitions.c src/logger.c)
target_link_libraries(common ${SYSREPO_LIBRARIES})

## JSON Serializer
add_library(json_serializer SHARED src/helper/json_serializer.c src/type_definitions.c)
target_link_libraries(json_serializer ${JANSSON_LIBRARIES})

# MODULE MAIN
add_executable(MainModule src/core.c)
target_link_libraries(MainModule common)

# MODULE REST
add_executable(RESTModule src/modules/REST/module_rest.c)
target_link_libraries(RESTModule common ${ULFIUS_LIBRARIES} json_serializer)

# MODULE CUC
add_executable(CUCModule src/modules/CUC/module_cuc.c)
target_link_libraries(CUCModule common ${ULFIUS_LIBRARIES} json_serializer)

# MODULE MONITOR
add_executable(MonitorModule src/modules/monitor/module_monitor.c)
target_link_libraries(MonitorModule common)

# MODULE CONTAINER
add_executable(ContainerModule src/modules/container/module_container.c)
target_link_libraries(ContainerModule common ${ULFIUS_LIBRARIES} json_serializer)

##### This is just for Testing purposes #####
add_executable(CNCPrototype src/CNC/cnc_prototype.c)
target_link_libraries(CNCPrototype ${ULFIUS_LIBRARIES} json_serializer)
