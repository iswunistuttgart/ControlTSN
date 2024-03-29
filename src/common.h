/*
 * Copyright (C) 2023 Institute for Control Engineering of Machine Tools and Manufacturing Units at the University of Stuttgart
 * Author Stefan Oechsle <stefan.oechsle@isw.uni-stuttgart.de>
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include "type_definitions.h"
#include "sysrepo/sysrepo_client.h"


// ----------------------------------------------
//      FUNCTIONS - Module-Handling
// ----------------------------------------------
/**
 * @brief Connects the module to sysrepo
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_connect();

/**
 * @brief Initializes a module by getting the stored module information from the registered modules
 * and writing the information to the passed struct.
 * @param module_name The name of the module to search for in the list of registered modules
 * @param module The struct for writing the gathered information
 * @param adjusted_subscribed_events_mask If > -1 this mask will be used instead of the stored one from the list of available modules
 * @param cb_event The events callback function of the module
 */
int module_init(char *module_name, TSN_Module **module, uint64_t adjusted_subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data));
//int module_init(char *module_name, TSN_Module **module, uint32_t adjusted_subscribed_events_mask, void (*cb_event)(TSN_Event_CB_Data));

/**
 * @brief Shutdown the module by disconnecting from sysrepo and stopping the subscription of notification.
 * @param module_id The id of the module
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_shutdown(int module_id);

/**
 * @brief Registers a module in the core.
 * @param module_id The id of the module to register
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_register(int module_id);

/**
 * @brief Unregisters a module from the core. Before that the function stops the module if necessary.
 * @param module_id The id of the module to unregister
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_unregister(int module_id);

/**
 * @brief Get a specific module based on the id from the list of registered modules.
 * @param module_id The id of the module to get
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
//int module_get_registered(int module_id, TSN_Module **module);

/**
 * @brief Get a specific module based on the id from the list of available modules.
 * @param module_id The id of the module to get
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
//int module_get_available(int module_id, TSN_Module **module);

/**
 * @brief Get all modules stored in the datastore
 * @param modules The TSN_Modules struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_all(TSN_Modules **modules);

/**
 * @brief Get a specific module stored in the datastore
 * @param module_id The id of the desired module
 * @param module The TSN_Module struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_id(int module_id, TSN_Module **module);


/**
 * @brief Deletes a specific module from the list of available modules. 
 * Before that, the module will be unregistered
 * @param module_id The id of the module to delete
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_delete(int module_id);

/**
 * @brief Starts a specific module by calling the executable defined by the path.
 * @param module_id The ID of the module to start
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_start(int module_id);

/**
 * @brief Stops the execution of a specific module by terminating the process defined by the PID.
 * The function uses 'kill' with the SIGUSR1 signal.
 * @param module_id The id of the module to start
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_stop(int module_id);


/**
 * @brief Writes the module specific data to the corresponding place in the sysrepo.
 * @param module_id The id of the module for which the data is to be updated
 * @param module_data The module specific data
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_update_data(int module_id, TSN_Module_Data module_data);

/**
 * @brief Get the module specific data of a module.
 * @param module_id The id of the module
 * @param module_data A pointer where the data will be written
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int module_get_data(int module_id, TSN_Module_Data **module_data);

/**
 * @brief Find and return a specific entry from the module data.
 * @param module_data The whole module data struct
 * @param entry_name The name of the entry to search for
 * @return The specific entry or NULL
 */
TSN_Module_Data_Entry *module_get_data_entry(TSN_Module_Data *module_data, const char *entry_name);

// ----------------------------------------------
//      FUNCTIONS - Stream-Handling
// ----------------------------------------------
/**
 * @brief Get all streams stored in the datastore.
 * @param streams The TSN_Streams struct to write on
 * @param without_configured_ones Set to 1 if only the not yet configured streams are desired
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int streams_get_all(TSN_Streams **streams, uint8_t without_configured_ones);

/**
 * @brief Get a specific stream based on the streamId
 * @param stream_id The ID of the stream
 * @param stream The TSN_Stream strcut to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
*/
int streams_get(char *stream_id, TSN_Stream **stream);

/**
 * @brief Converts the user (or app) requirements to the request model that the CUC expects.
 * NOTICE: I am not sure if this function should be part of the CUC (i.e. the CUC would only get the user requirements as input)
 * or if a middleware like this engineering framework can/should handle it.
 */
TSN_Request create_stream_request(TSN_Enddevice *talker_device, uint16_t count_listeners, TSN_Enddevice *listener_devices, IEEE_TrafficSpecification *traffic_spec, IEEE_UserToNetworkRequirements *qos_talker, IEEE_UserToNetworkRequirements *qos_listeners);

/**
 * @brief Request a new stream.
 * @param request The TSN_Request struct containing the necessary information for the computation
 * @param generated_stream_id The generated stream id will be written to this string
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int stream_request(TSN_Request *request, char **generated_stream_id);

/**
 * @brief Sets a stream as computed by writing the computed configuration to the datastore.
 * @param stream_id The stream id of the computed stream
 * @param configuration The TSN_Configuration struct containing the computed information
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int stream_set_computed(char *stream_id, TSN_Configuration *configuration);

/**
 * @brief Deletes a specific stream. 
 * @param stream_id The id of the stream to delete
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int streams_delete(char *stream_id);

// ----------------------------------------------
//      FUNCTIONS - Topology-Handling
// ----------------------------------------------
/**
 * @brief Get the complete topology information containing the devices and the connections.
 * @param topology The TSN_Topology struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int topology_get(TSN_Topology **topology);
/**
 * @brief Get all devices from the topology.
 * @param devices The TSN_Devices struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int topology_get_devices(TSN_Devices **devices);
/**
 * @brief Put all devices from the topology.
 * @param devices The TSN_Devices struct to free
 */
void topology_put_devices(TSN_Devices *devices);
/**
 * @brief Get the graph of the topology containing all connections.
 * @param graph The TSN_Graph struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int topology_get_graph(TSN_Graph **graph);

// ----------------------------------------------
//      FUNCTIONS - Application-Handling
// ----------------------------------------------
/**
 * @brief Get the application information containing all apps and images.
 * @param applications The TSN_Applications struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int application_get(TSN_Application **application);
/**
 * @brief Put the application information.
 * @param application The TSN_App struct to free
 */
void application_app_put(TSN_App *application);
/**
 * @brief Get the stored apps.
 * @param apps The TSN_Apps struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int application_get_apps(TSN_Apps **apps);
/**
 * @brief Put the stored apps.
 * @param apps The TSN_Apps struct to free
 */
void application_put_apps(TSN_Apps *apps);
/**
 * @brief Get the stored images.
 * @param images The TSN_Images struct to write on
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int application_get_images(TSN_Images **images);
/**
 * @brief Put the stored images.
 * @param images The TSN_Images struct to free
 */
void application_put_images(TSN_Images *images);

/**
 * @brief Find the enddevice for a specific App
 * @param app_id The id of the app
 * @param enddevices The list of enddevices to search through
 * @param count_enddevices The length of the list of enddevices
 * @return The enddevice the app is running on if found
*/
const TSN_Enddevice *
configuration_find_enddevice_of_app(char *app_id, const TSN_Enddevice *enddevices,
                                    uint16_t count_enddevices);

/**
 * @brief Set the SendReceive flag of a specific stream
 * @param stream_id The ID of the stream
 * @param enable Whether the stream should send/receive or not
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
*/
int configuration_stream_set_sendreceive(char *stream_id, bool enable);

/**
 * @brief Get the SendReceive flag of a specific stream
 * @param stream_id The ID of the stream
 * @return True or False
*/
bool configuration_stream_get_sendreceive(char *stream_id);

/**
 * @brief Deploys the application-specific parameters to the engineering interface fo the app.
 * @param app_id The ID of the app
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
*/
int configuration_app_deploy_parameters(char *app_id);

#endif // __COMMON_H__
