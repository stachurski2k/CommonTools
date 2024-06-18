#ifndef SKG_NETLIB_CLIENT_H
#define SKG_NETLIB_CLIENT_H

#include "packet.h"
#include "stream.h"
#include "utils.h"
#include <thread>
#include <string>
#include <vector>
#include <atomic>
#include "codes.h"
namespace netlib {
    class Client {
        Stream *stream;
        std::thread *reciveThread;
        std::thread *specialEventThread;//when you try to connect in disconnect event
        std::atomic_bool isConnected = false;

        void ReciveThread();
	void DisconnectEvent();
    //todo: remove below
    std::vector<std::string> errors = {
    "Sukces",
    "Błąd inicjalizacji WSA",
    "Błąd tworzenia socketa",
    "Błąd połączenia",
    "Błąd wewnętrzny poczas łączenia",
    "Klient już połączony",
    "Nie znaleziono serwera",
    "Brakuje historii łączności",
    "Błąd automatycznego łączenia: klient już połączony"
    }; 
    const unsigned errorCount = std::size(errors);
    public:
	void(*OnPacket)(Client& caller,Packet& packet);//event is raised on reciving thread when packet comes
	void(*OnDisconnected)(Client& caller);//event is raised on reciving thread when client is disconnected
    Client();

    Client(const Client &other) = delete;

    bool IsConnected() { return isConnected; }

    CODE FindServerAndConnect(int timeoutMs = 2000,std::string preambule="");//blocking,see code types in codes.h
    CODE ConnectToServer(std::string serverIp, int serverPort);//blocking,see code types in codes.h
    CODE SendToServer(Packet &packet);//blocking,see code types in codes.h
    void Disconnect();//request to server is send and awaited
    void reportError(int);//to remove
    ~Client();
    };
}

#endif//SKG_NETLIB_CLIENT_H