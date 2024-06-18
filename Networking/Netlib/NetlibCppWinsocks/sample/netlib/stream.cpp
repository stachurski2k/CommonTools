#include "stream.h"
#include <iostream>
#include "utils.h"

namespace netlib {
    Stream::Stream() : bufferOffset(0), sendlock(), recivelock(), recivebuffer(StartingBufferSize), remoteIp(""),
                       remotePort(-1), localIp(""), localPort(-1) {
        sock = INVALID_SOCKET;
    }

    Stream::Stream(SOCKET _socket) : bufferOffset(0), sendlock(), recivelock(), recivebuffer(StartingBufferSize),
                                     remoteIp(""), remotePort(-1), localIp(""), localPort(-1) {
        From(_socket);
    }

    int Stream::Avilable() {
        return -1;
    }

    void Stream::From(SOCKET _sock) {
        sock = _sock;
        try {//obtain remote and local ep info
            sockaddr_in sockInfo;
            int sockInfoLen = sizeof(sockInfo);
            if (getsockname(sock, (sockaddr *) &sockInfo, &sockInfoLen) == SOCKET_ERROR) {
                std::cerr << "getsockname failed with error: " << WSAGetLastError() << std::endl;
            }
            auto localEp = GetEndPoint(sockInfo);
            localPort = localEp.second;
            localIp = localEp.first;
            if (getpeername(sock, (sockaddr *) &sockInfo, &sockInfoLen) == SOCKET_ERROR) {
                std::cerr << "getpeername failed with error: " << WSAGetLastError() << std::endl;
            }
            auto remoteEp = GetEndPoint(sockInfo);
            remotePort = remoteEp.second;
            remoteIp = remoteEp.first;
        } catch (const std::exception &e) {}
    }

    CODE Stream::Send(Packet &packet) {
        if (sock == INVALID_SOCKET)return CODE::STREAM_SOCKET_CLOSED;
        std::lock_guard<std::mutex> lc(sendlock);//unlock in destructor
        try {
            int bytesToSend = packet.GetDataLen() + Packet::HeaderSize;
            const char *actualData = packet.GetData();
            int totalsent = 0;
            //send can actually send less than provided...
            while (totalsent < bytesToSend) {
                totalsent += send(sock, actualData + totalsent, bytesToSend - totalsent, 0);
            }
        }
        catch (const std::exception &e) {
            return CODE::STREAM_SENDING_ERROR;
        }
        return CODE::OK;
    }

    CODE Stream::Recive(Packet &packetout) {
        if (sock == INVALID_SOCKET)return CODE::STREAM_SOCKET_CLOSED;
        std::lock_guard<std::mutex> lc(recivelock);//unlock in destructor
        packetout = Packet::InvalidPacket();
        try {
            //first recive header
            bufferOffset = 0;
            while (bufferOffset < Packet::HeaderSize) {
                int recived = 0;
                recived = recv(sock, recivebuffer.data() + bufferOffset, Packet::HeaderSize - bufferOffset, 0);
                if (recived == 0 || recived == SOCKET_ERROR || recived < 0) {//connection was closed
                    return CODE::STREAM_SOCKET_CLOSED;
                }
                bufferOffset += recived;
            }
            //unpack header

            bufferOffset = Packet::HeaderSize;
            std::pair<int16_t, int16_t> packetId_datalen
                    = Packet::DeserializeHeader(recivebuffer, 0);
            //adjust the recive buffer to incoming packet sajz
            if(packetId_datalen.second+Packet::HeaderSize>=recivebuffer.size()){
                recivebuffer.resize(Packet::HeaderSize+packetId_datalen.second);
            }

            //retrive the rest of the packet
            while (bufferOffset < packetId_datalen.second + Packet::HeaderSize) {
                int recived = 0;
                recived = recv(sock, recivebuffer.data() + bufferOffset,
                               packetId_datalen.second - bufferOffset + Packet::HeaderSize, 0);
                if (recived == 0 || recived == SOCKET_ERROR || recived < 0) {//connection was closed
                    return CODE::STREAM_SOCKET_CLOSED;
                }
                bufferOffset += recived;
            }
            //unpack whole packet
            return Packet::FromByteArray(packetout, recivebuffer, 0);
        }
        catch (const std::exception &e) {
            return CODE::STREAM_RECIVING_ERROR;
        }
        return CODE::OK;
    }
    bool Stream::IsOpened() {
        return sock != INVALID_SOCKET;
    }
    void Stream::Close() {
        std::lock_guard<std::mutex> lc(sendlock);
        if (sock == INVALID_SOCKET)return;//nothing to close
        try {
            closesocket(sock);
        } catch (const std::exception &e) {}
        sock = INVALID_SOCKET;
    }

    Stream::~Stream() {
        recivebuffer.clear();
    }
}
