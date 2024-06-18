#ifndef SKG_NETLIB_BROADCASTER_H
#define SKG_NETLIB_BROADCASTER_H

#include <thread>
#include <set>
#include "utils.h"
#include "packet.h"
#include<atomic>
namespace netlib {
    class Broadcaster {
        std::set<std::pair<std::string, int>> results;//all servers endpoints found so far
        std::pair<std::string, int> lastServerEp;
        std::thread *logic;//send broadcast logic
        std::thread *recvLogic;//recv broadcast logic
        std::atomic_bool isRunning;//used for sending
        std::atomic_bool isReciving;
        SOCKET broadcastSocket;
        SOCKET reciverSocket;
        int broadcastPeriodsMs;//periods in sending
        int infoPort;//this port will be send in broadcast message
        std::string CustomPreambule;
        void SendBroadcastLogic();
        void FindLogic();

    public:
        static constexpr int BroadcastPort = 8888;//client should listen on this port
        const char *BroadcastPreamble = "SKG2024";//for checking if broadcast is from the server
        void SetCustomPreambule(std::string s);//overrides default BroadcastPreamble
        int GetCustomPreambuleSize();//prembule.size() + Packet::Header size + 2 int size(8B)
        std::set<std::pair<std::string, int>> GetResults() { return results; };
        void (*OnServerFound)(std::pair<std::string, int>);//callback for server found
        Broadcaster();

        //all return status code, CODE::OK or error codes, see codes.h
        CODE FindServers();//async, starts new thread, notifies by OnServerFound
        CODE FindServers(
                int timeoutMs);//sync, blocks for timeout, finds as many servers as it could in given time 0-found some,CODE::OK if found
        CODE FindOneServer(
                int timeoutMs = 2000);//syncronous, blocks until one server is found or timeout is reached,CODE::OK if found
        void StopFinding();//stops the above ^^^^

        CODE GetLastServer(std::pair<std::string, int> &ep);//CODE::OK if found
        CODE StartBroadcast(int infoPort,
                           int periodsMs = 400);//Starts broadcast on new thread, sends it every Ms, infoPort is writen in mg, CODE::OK if succesfull start
        void StopBroadcast();//stops broadcast thread

        ~Broadcaster();
    };
}

#endif//SKG_NETLIB_BROADCASTER_H
