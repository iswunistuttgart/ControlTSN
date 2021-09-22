cmake_minimum_required(VERSION 3.16)

project(ControlTSN_2)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS ON)
#add_compile_options(-fms-extensions)

add_library(common SHARED src/common.c src/sysrepo/sysrepo_client.c src/type_definitions.c)
target_link_libraries(common sysrepo)


# MODULE MAIN
add_executable(MainModule src/core.c)
target_link_libraries(MainModule common)

# MODULE REST
#add_executable(Module_REST src/modules/REST/module_rest.c src/sysrepo/sysrepo_client.c)
#target_link_libraries(Module_REST ulfius jansson sysrepo)
#add_executable(Module_REST src/modules/REST/module_rest.c )
#target_link_libraries(Module_REST ulfius jansson common)
add_executable(RESTModule src/modules/REST/module_rest.c)
target_link_libraries(RESTModule common ulfius jansson)
