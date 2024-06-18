#ifndef SKG_NETLIB_CODES_H
#define SKG_NETLIB_CODES_H

#include <cstdint>
#include <string>
namespace netlib {
    //This file contains status codes for netlib
    //generaly if sth returns CODE::OK then everything is ok
    //0-99 general codes
    //100-199 packet codes
    //200-299 stream codes
    //300-399 client codes
    //400-499 server codes
    //500-599 broadcaster codes
    enum class CODE : int32_t {
        //general
        OK = 0,//operation succesfull
        NOT_OK=1,
        SOCKET_CREATE_ERROR = 2,//failed to create socket
        SOCKETS_NOT_SUPPORTED=3,//failed to init wsa or sth
        //packet
        PACKET_DESERIALIZE_FAILED=100,

        //stream
        STREAM_SOCKET_CLOSED=200,//socket is not opened
        STREAM_SENDING_ERROR=201,//failed to send packet
        STREAM_RECIVING_ERROR=202,//failed to recive packet

        //clientx
        CLIENT_NOT_CONNECTED=300,//you are not connected
        CLIENT_ALREADY_CONNECTED=301,//you are already connected
        CLIENT_CONNECTING_FAILED=302,//failed to connect
        CLIENT_CONNECTING_ERROR=303,//error when connecting
        //client automatic finding
        CLIENT_SERVER_NOT_FOUND_TIMEOUT=304,//server not found in time
        CLIENT_SERVER_NOT_FOUND=305,//server not found

        //serverx
        SERVER_ALREADY_RUNNING=400,//server is already running
        SERVER_NOT_RUNNING=401,//server is not running
        SERVER_PORT_TAKEN=402,//port is already taken
        SERVER_START_LISTENING_FAILED=403,//failed to start listening
        SERVER_STARTING_ERROR    =404,//failed to start server
        SERVER_CLIENT_NOT_FOUND  =405,//client not found
        SERVER_SEND_TO_ALL_FAILED =406,//failed to send to all

        //broadcaster
        CASTER_ALREADY_FINDING=500,//already finding
        CASTER_SERVER_NOT_FOUND=501,//found something
        CASTER_ALREADY_CASTING=502,//already casting
        CASTER_FAILED_TO_CREATE_BROADCAST_SOCKET=503,//failed to create socket
        CASTER_STARTING_CASTING_ERROR   =504,//failed to start casting
    };

    //Packet types <0 are reserved for internal use
    enum class ReservedPacketTypes:int16_t{
        INVALID_PACKET=-1,
        CLOSE_CONNECTION_REQUEST=-2
    };

    //returns code description in en, format= "CODE(int): description"
    std::string getCodeDescription(CODE code);
}
#endif