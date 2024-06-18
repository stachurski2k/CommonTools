#ifndef SKG_NETLIB_SERVER_H
#define SKG_NETLIB_SERVER_H

#include "packet.h"
#include "stream.h"
#include <map>
#include <thread>
#include "utils.h"
#include <iostream>
#include<atomic>
namespace netlib {
    class Server {
        std::map<int, Stream *> clients;
        std::map<int, std::thread *> threadClients;
        std::thread *acceptThread;
        SOCKET sock;//accept sock
        int GetNextClientId();//intitaily 0 then increment

        void AcceptThread();//thread for accepting clients

        void ReciveFromClient(int id);//thread for reciving form client

        std::atomic_bool isRunning = false;
        std::mutex clientMutex;
        std::string ip;
        int port;
    public:
        std::string GetIp() { return ip; }

        int GetPort() { return port; }
        void(*OnClientConnected)(Server& caller,int id);//new cliend it, called on accept thread
        void(*OnPacket)(Server& caller, int id, Packet& packet);//client id, called on client reciving thread
        void(*OnClientDisconnected)(Server& caller, int id);//client id, called on client reciving thread
        Server();

        Server(const Server &other) = delete;

        bool IsRunning() { return isRunning; }
	    CODE Start(int port);//CODE::OK or error,see code types in codes.h
        Stream *GetClientStream(int id);//get client stream to obtain his endpoint
//core
        CODE SendToClient(int cliendId, Packet &packet);//see code types in codes.h
        CODE SentToAll(Packet &packet);//see code types in codes.h
        void DisconnectClient(int clientId);//disconnect client and remove him from list
        void Close();

        ~Server();
    };
}

#endif//SKG_NETLIB_SERVER_H