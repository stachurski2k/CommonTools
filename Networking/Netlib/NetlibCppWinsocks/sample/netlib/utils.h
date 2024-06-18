#ifndef SKG_NETLIB_UTILS_H
#define SKG_NETLIB_UTILS_H

#include <winsock2.h>
#include <string>
#include "codes.h"
namespace netlib {

    CODE InitWSA();//checks if sockets are suppor

    std::pair<std::string, int> GetEndPoint(const sockaddr_in &addr);//extracts ip and port

}

#endif //SKG_NETLIB_UTILS_H