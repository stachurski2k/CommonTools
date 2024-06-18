#include "utils.h"
#include <ws2tcpip.h>

namespace netlib {

    CODE InitWSA() {
        static bool initialized = false;
        if (initialized)return CODE::OK;
        WSAData wsaData;
        int nCode;
        char errdesc[100];
        if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0) {
            return CODE::SOCKETS_NOT_SUPPORTED;
        }
        initialized = true;
        return CODE::OK;
    }

    std::pair<std::string, int> GetEndPoint(const sockaddr_in &addr) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
        std::string _ip(ip);
        return {_ip, ntohs(addr.sin_port)};
    }
}