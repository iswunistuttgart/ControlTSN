#ifndef __SYSREPO_CLIENT_H__
#define __SYSREPO_CLIENT_H__

#include <sysrepo.h>
#include "type_definitions.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// -------------------------------------------------------- //
//  Initialization and connection stuff
// -------------------------------------------------------- //
int sysrepo_connect();
int sysrepo_disconnect();
int sysrepo_start_listening();
int sysrepo_stop_listening();

// -------------------------------------------------------- //
//  Callbacks
// -------------------------------------------------------- //
void sysrepo_init_callbacks(
    void (*cb_stream_requested)(TSN_Stream *),
    void (*cb_stream_configured)(TSN_Stream *),
    // ...
    void (*cb_error)()
);

// -------------------------------------------------------- //
//  CRUD methods
// -------------------------------------------------------- //
int sysrepo_get_root(TSN_Uni **root);

int sysrepo_write_stream(TSN_Stream stream);
int sysrepo_read_stream(char *stream_id, TSN_Stream **stream);
int sysrepo_delete_stream(char *stream_id);

#endif // __SYSREPO_CLIENT_H__