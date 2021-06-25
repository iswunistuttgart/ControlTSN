#ifndef __TYPE_DEFINITIONS_H__
#define __TYPE_DEFINITIONS_H__


typedef struct Stream {
    // The unique id of the stream in the format XX-XX-XX-XX-XX-XX:YY-YY,
    // where XX is the MAC of the Talker and YY a unique number
    char *stream_id;        
} Stream;

typedef struct TSN_Uni {
    Stream *stream_list;
} TSN_Uni;


#endif // __TYPE_DEFINITIONS_H__