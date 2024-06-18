#ifndef SKG_NETLIB_STREAM_H
#define SKG_NETLIB_STREAM_H

#include <winsock2.h>
#include "packet.h"
#include <mutex>
#include <vector>
#include "codes.h"
//intermediary class for handling connections
//This should recive an connected TCP socket
namespace netlib {
    class Stream {
        SOCKET sock;
        std::vector<char> recivebuffer;//adaptive size
        int bufferOffset;//0 initialy
        std::mutex sendlock;//sync recive and send
        std::mutex recivelock;
    public:
        std::string remoteIp;
        std::string localIp;
        int remotePort;
        int localPort;
        static constexpr int StartingBufferSize=256;//size of recv buffer will adapt to incoming packet

        Stream();

        Stream(SOCKET _socket);

        Stream(const Stream &other) = delete;

        void From(SOCKET _sock);
        bool IsOpened();
        //blocking until all bytes sent
        CODE Send(Packet &packet);//see code types in codes.h

        int Avilable();//bytes to read in stream,not suported returns -1

        //see code types in codes.h, CODE::STREAM_SOCKET_CLOSED if connection is closed
        CODE Recive(Packet &packetOut/*int timeout?? idk if winsocks support*/);

        void Close();

        ~Stream();
    };
}

#endif//SKG_NETLIB_STREAM_H