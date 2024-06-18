#include "broadcaster.h"
#include <iostream>
#include <chrono>

#define timenow std::chrono::high_resolution_clock::now
namespace netlib {
    Broadcaster::Broadcaster() : isRunning(false), isReciving(false), results() {
        OnServerFound = nullptr;
        logic = nullptr;
        recvLogic = nullptr;
        broadcastPeriodsMs = 400;
        infoPort = 8080;
        broadcastSocket = INVALID_SOCKET;
        reciverSocket = INVALID_SOCKET;
        lastServerEp = {"127.0.0.1", 8080};
        CustomPreambule=std::string(BroadcastPreamble);
    }

    CODE Broadcaster::GetLastServer(std::pair<std::string, int> &ep) {
        ep = lastServerEp;
        if (results.size() > 0) {
            return CODE::OK;
        }
        return CODE::CASTER_SERVER_NOT_FOUND;
    }
    void Broadcaster::SetCustomPreambule(std::string s) {
        CustomPreambule=s;
    }
    int Broadcaster::GetCustomPreambuleSize(){
        return Packet::HeaderSize+CustomPreambule.size()+8;//8 bytes for port and preambule size
    }
#pragma region SendingBroadcast

    CODE Broadcaster::StartBroadcast(int port, int periodsMs) {
        this->infoPort = port;
        broadcastPeriodsMs = periodsMs;
        if (isRunning)return CODE::CASTER_ALREADY_CASTING;
        try {
            if (InitWSA()!=CODE::OK)return CODE::SOCKETS_NOT_SUPPORTED;
            //prepare broadcast socket
            broadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (broadcastSocket == INVALID_SOCKET) { return CODE::SOCKET_CREATE_ERROR; }
            char broadcast = 'a';//as in docs
            if (setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
                closesocket(broadcastSocket);
                return CODE::CASTER_FAILED_TO_CREATE_BROADCAST_SOCKET;
            }
            //start logic
            isRunning = true;
            logic = new std::thread(&Broadcaster::SendBroadcastLogic, this);
        }
        catch (const std::exception &e) { return CODE::CASTER_STARTING_CASTING_ERROR; }
        return CODE::OK;
    }

    void Broadcaster::SendBroadcastLogic() {
        //prepare addres
        sockaddr_in broadcastAddr;
        broadcastAddr.sin_family = AF_INET;
        broadcastAddr.sin_port = htons(BroadcastPort);
        broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

        //prepare msg, use packet for simplicity, msg is just packet with preamble and infoport
        Packet broadcastMessage(-1);
        broadcastMessage.Write(CustomPreambule);
        broadcastMessage.Write(infoPort);

        //start sending broadcast
        int offset = 0;
        int bytesToSend=broadcastMessage.GetDataLen()+Packet::HeaderSize;
        const char* data=broadcastMessage.GetData();
        while (isRunning) {
            while (offset < bytesToSend) {
                int sendResult = sendto(broadcastSocket, data+offset, bytesToSend-offset, 0,
                                        (sockaddr *) &broadcastAddr, sizeof(broadcastAddr));
                if (sendResult == SOCKET_ERROR) { continue; }
                offset += sendResult;
            }
            offset = 0;
            Sleep(broadcastPeriodsMs);
        }
        try {
            closesocket(broadcastSocket);
        }
        catch (const std::exception &e) {}
    }

    void Broadcaster::StopBroadcast() {
        isRunning = false;
        if (logic != nullptr) {
            logic->join();
            delete logic;
            logic = nullptr;
        }
    }

#pragma endregion
#pragma region RecivingBroadcast

    void Broadcaster::FindLogic() {
        if (InitWSA()!=CODE::OK) { return; }
        sockaddr_in recvAddr;
        try {
            recvAddr.sin_family = AF_INET;
            recvAddr.sin_port = htons(BroadcastPort);
            recvAddr.sin_addr.s_addr = INADDR_ANY;
            reciverSocket = INVALID_SOCKET;
            //port might be ocupied by other app so wait till its free
            while (isReciving && reciverSocket == INVALID_SOCKET) {
                reciverSocket = INVALID_SOCKET;
                try {
                    reciverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                    if (reciverSocket == INVALID_SOCKET) {
                        continue;
                    }
                    char broadcast = 'a';
                    if (setsockopt(reciverSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
                        closesocket(reciverSocket);
                        reciverSocket = INVALID_SOCKET;
                        continue;
                    }
                    if (bind(reciverSocket, (sockaddr*)&recvAddr, sizeof(recvAddr)) < 0) {
                        closesocket(reciverSocket);
                        reciverSocket = INVALID_SOCKET;
                        continue;
                    }
                }
                catch (const std::exception& e) {}
            }
            std::vector<char> recvBuf(1024);
            int recvBufLen = 1024;
            //address of the potential server
            sockaddr_in senderAddr;
            int senderAddrSize = sizeof(senderAddr);

            while (isReciving) {
                try {
                    int recvResult = recvfrom(reciverSocket, recvBuf.data(), recvBufLen, 0, (sockaddr*)&senderAddr,
                        &senderAddrSize);
                    if (recvResult == SOCKET_ERROR || recvResult == 0) { break; }//stop reciving called
                    if (recvResult == GetCustomPreambuleSize()) {
                        bool preambuleFound = true;
                        //check if preambules match
                        for (int i = 0; i < CustomPreambule.size(); i++) {
                            if (recvBuf[i + 8] != CustomPreambule[i]) {
                                preambuleFound = false;
                            }
                        }
                        if (!preambuleFound) {
                            continue;
                        }
                        Packet packet(-1);
                        if (Packet::FromByteArray(packet, recvBuf, 0)==CODE::OK) {
                            std::string preamble = packet.ReadString();
                            //obtain server endpoint
                            int port = packet.ReadInt();
                            auto ep = GetEndPoint(senderAddr);
                            lastServerEp = { ep.first, port };
                            results.insert({ ep.first, port });
                            if (OnServerFound != nullptr) {
                                OnServerFound({ ep.first, port });
                            }
                        }
                    }
                }catch(const std::exception& e ){}
            }
        }catch(const std::exception& e){}
        if (reciverSocket != INVALID_SOCKET) {
            try {
                closesocket(reciverSocket);
            }
            catch (const std::exception &e) {}
        }
    }

    CODE Broadcaster::FindOneServer(int timeoutMs) {
        if (isReciving) { return CODE::CASTER_ALREADY_FINDING; };
        isReciving = true;
        auto start = timenow();
        results.clear();
        recvLogic = new std::thread(&Broadcaster::FindLogic, this);
        //wait for timeout or server
        while (true) {
            auto now = timenow();
            int elapsed = duration_cast<std::chrono::milliseconds>(now - start).count();
            if (elapsed >= timeoutMs) {
                break;
            }
            if (results.size() > 0) {
                break;
            }
            Sleep(2);
        }
        StopFinding();
        if (results.size() > 0) {
            return CODE::OK;
        }
        return CODE::CASTER_SERVER_NOT_FOUND;
    }

    CODE Broadcaster::FindServers(int timeoutMs) {
        if (isReciving) { return CODE::CASTER_ALREADY_FINDING; };
        isReciving = true;
        auto start = timenow();
        results.clear();
        recvLogic = new std::thread(&Broadcaster::FindLogic, this);
        //wait for timeout or server
        while (true) {
            auto now = timenow();
            int elapsed = duration_cast<std::chrono::milliseconds>(now - start).count();
            if (elapsed >= timeoutMs) {
                break;
            }
            Sleep(2);
        }
        StopFinding();
        if (results.size() > 0) {
            return CODE::OK;
        }
        return CODE::CASTER_SERVER_NOT_FOUND;
    }

    CODE Broadcaster::FindServers() {
        if (isReciving) { return CODE::CASTER_ALREADY_FINDING; };
        isReciving = true;
        recvLogic = new std::thread(&Broadcaster::FindLogic, this);
        return CODE::OK;
    }

    void Broadcaster::StopFinding() {
        if (isReciving) {
            isReciving = false;
            if (reciverSocket != INVALID_SOCKET) {
                try {
                    closesocket(reciverSocket);
                    reciverSocket = INVALID_SOCKET;
                }
                catch (const std::exception &e) {}
            }
            if (recvLogic != nullptr) {
                recvLogic->join();
                delete recvLogic;
                recvLogic = nullptr;
            }
        }
    }

#pragma endregion

    Broadcaster::~Broadcaster() {
        if (isRunning) {
            StopBroadcast();
        }
        if (isReciving) {
            StopFinding();
        }
    }
}