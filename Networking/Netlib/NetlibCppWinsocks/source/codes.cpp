#include "codes.h"
namespace netlib{

    std::string getCodeDescription(CODE code) {
        std::string res = "CODE(" + std::to_string(static_cast<int>(code)) + "): ";
        switch (code) {
            case CODE::OK: res+= "operation successful"; break;
            case CODE::NOT_OK: res+= "operation not successful"; break;
            case CODE::SOCKET_CREATE_ERROR: res+= "failed to create socket"; break;
            case CODE::SOCKETS_NOT_SUPPORTED: res+= "failed to init WSA"; break;
            case CODE::PACKET_DESERIALIZE_FAILED: res+= "failed to deserialize packet"; break;
            case CODE::STREAM_SOCKET_CLOSED: res+= "socket is not opened"; break;
            case CODE::STREAM_SENDING_ERROR: res+= "failed to send packet"; break;
            case CODE::STREAM_RECIVING_ERROR: res+= "failed to receive packet"; break;
            case CODE::CLIENT_NOT_CONNECTED: res+= "client is not connected"; break;
            case CODE::CLIENT_ALREADY_CONNECTED: res+= "client is already connected"; break;
            case CODE::CLIENT_CONNECTING_FAILED: res+= "client failed to connect"; break;
            case CODE::CLIENT_CONNECTING_ERROR: res+= "client error when connecting"; break;
            case CODE::CLIENT_SERVER_NOT_FOUND_TIMEOUT: res+= "server not found in time"; break;
            case CODE::CLIENT_SERVER_NOT_FOUND: res+= "server not found"; break;
            case CODE::SERVER_ALREADY_RUNNING: res+= "server is already running"; break;
            case CODE::SERVER_NOT_RUNNING: res+= "server is not running"; break;
            case CODE::SERVER_PORT_TAKEN: res+= "port is already taken"; break;
            case CODE::SERVER_START_LISTENING_FAILED: res+= "failed to start listening"; break;
            case CODE::SERVER_STARTING_ERROR: res+= "error when starting server"; break;
            case CODE::SERVER_CLIENT_NOT_FOUND: res+= "client not found"; break;
            case CODE::SERVER_SEND_TO_ALL_FAILED: res+= "failed to send to all"; break;
            case CODE::CASTER_ALREADY_FINDING: res+= "broadcaster already finding"; break;
            case CODE::CASTER_SERVER_NOT_FOUND: res+= "broadcaster did not found server"; break;
            case CODE::CASTER_ALREADY_CASTING: res+= "already broadcasting"; break;
            case CODE::CASTER_FAILED_TO_CREATE_BROADCAST_SOCKET: res+= "failed to create broadcast socket"; break;
            case CODE::CASTER_STARTING_CASTING_ERROR: res+= "failed to start broadcasting"; break;
            default: res+= "unknown code"; break;
        }
        return res;
    }
}