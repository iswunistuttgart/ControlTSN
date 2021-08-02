cmake_minimum_required(VERSION 3.16)

project(ControlTSN_2)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS ON)
#add_compile_options(-fms-extensions)

add_executable(ControlTSN_2 src/core.c
                            src/sysrepo/sysrepo_client.c)
target_link_libraries(ControlTSN_2 sysrepo)

# MODULE REST
add_executable(Module_REST src/modules/REST/module_rest.c)
target_link_libraries(Module_REST ulfius)