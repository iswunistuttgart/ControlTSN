/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "type_definitions.h"

// ----------------------------------------------
//      Printing
// ----------------------------------------------
/**
 * @brief Prints the data of a module to the console.
 * @param module The data of the module to print
 */
void print_module_data(TSN_Module_Data data);

/**
 * @brief Prints a module to the console.
 * @param module The module struct to print
 */
void print_module(TSN_Module module);

/**
 * @brief Prints a topology to the console.
 * @param topology The topology struct to print
 */
void print_topology(TSN_Topology topology);

void print_enddevice(TSN_Enddevice enddevice);

void print_stream_request(TSN_Request request);
void print_stream_configuration(TSN_Configuration config);
void print_stream(TSN_Stream stream);

void print_app(TSN_App app);

#endif // __LOGGER_H__
