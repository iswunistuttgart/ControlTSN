#ifndef __TYPE_DEFINITIONS_H__
#define __TYPE_DEFINITIONS_H__

#include <stdint.h>


typedef struct Talker {
    // ieee802-dot1q-tsn-types -> group-talker
    // ...
} Talker;

typedef struct Listener {
    uint16_t index;

    // ieee802-dot1q-tsn-types -> group-listener
    // ...
} Listener;

typedef struct StatusTalker {
    // ieee802-dot1q-tsn-types -> group-status-talker-listener
    // ...
} StatusTalker;

typedef struct StatusListener {
    uint16_t index;
    
    // ieee802-dot1q-tsn-types -> group-status-talker-listener
    // ...
} StatusListener;

typedef struct Configuration {
    // ieee802-dot1q-tsn-types -> group-status-stream
    // ...

    StatusTalker *talker;
    StatusListener *listener_list;
} Configuration;

typedef struct Request {
    Talker *talker;
    Listener *listener_list;
} Request;

typedef struct Stream {
    // The unique id of the stream in the format XX-XX-XX-XX-XX-XX:YY-YY,
    // where XX is the MAC of the Talker and YY a unique number
    char *stream_id;   
    Request *request;
    Configuration *configuration;
} Stream;

typedef struct TSN_Uni {
    Stream *stream_list;
} TSN_Uni;


#endif // __TYPE_DEFINITIONS_H__