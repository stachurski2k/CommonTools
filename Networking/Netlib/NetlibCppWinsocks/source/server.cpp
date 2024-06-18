#include "server.h"
#include<iostream>
namespace netlib {
    Server::Server() : clientMutex(), clients(), threadClients(), isRunning(false), port(-1) {
        OnClientConnected = nullptr;
        OnPacket = nullptr;
        OnClientDisconnected = nullptr;
        sock = INVALID_SOCKET;
        acceptThread = nullptr;
    }

    CODE Server::Start(int port) {
        if (isRunning) { return CODE::SERVER_ALREADY_RUNNING; }
        if (InitWSA()!=CODE::OK) { return CODE::SOCKETS_NOT_SUPPORTED; }
        try {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET)return CODE::SOCKET_CREATE_ERROR;
            sockaddr_in service;
            service.sin_family = AF_INET;
            service.sin_addr.s_addr = INADDR_ANY;
            service.sin_port = htons(port);

            int res = bind(sock, (sockaddr *) &service, sizeof(service));
            if (res == SOCKET_ERROR) {
                closesocket(sock);
                return CODE::SERVER_PORT_TAKEN;
            }
            res = listen(sock, 5);
            if (res == SOCKET_ERROR) {
                closesocket(sock);
                return CODE::SERVER_START_LISTENING_FAILED;
            }

            //get info
            this->port = port;
            this->ip = GetEndPoint(service).first;

            isRunning = true;
            if (acceptThread != nullptr) {
                acceptThread->join();
                delete acceptThread;
            }
            acceptThread = new std::thread(&Server::AcceptThread, this);
        }
        catch (const std::exception &e) {
            return CODE::SERVER_STARTING_ERROR;
        }
        return CODE::OK;
    }

    void Server::AcceptThread() {
        sockaddr_in remote;
        int sajz = sizeof(remote);
        while (isRunning) {
            SOCKET client = accept(sock,
                                   (sockaddr *) &remote, &sajz);
            if (client == INVALID_SOCKET) { break; }//socket was closed
            //add new client
            int newClientId = GetNextClientId();
            {
                std::lock_guard<std::mutex> guard(clientMutex);

                clients[newClientId] = new Stream(client);
                if (clients[newClientId] == nullptr || clients[newClientId] == NULL) {
                    clients.erase(newClientId);
                    try {
                        closesocket(client);

                    }catch(const std::exception& e){}
                    continue;
                }

            }
            if (OnClientConnected != nullptr)OnClientConnected(*this, newClientId);
            threadClients[newClientId] = new std::thread(&Server::ReciveFromClient, this, newClientId);

            std::vector<int> toremove;
            for (auto& e : threadClients) {
                if (clients.find(e.first) != clients.end()) {
                    continue;
                }
                e.second->join();
                delete e.second;
                toremove.push_back(e.first);
            }
            for (auto e : toremove) {
                threadClients.erase(e);
            }
        }
    }

    void Server::ReciveFromClient(int id) {
        if (clients.find(id) == clients.end()) {
            return;
        }
        while (isRunning) {
            Packet recvPacket(-1);
            CODE status = clients[id]->Recive(recvPacket);
            if (status == CODE::STREAM_SOCKET_CLOSED) { break; }//connection closed
            if (status == CODE::STREAM_RECIVING_ERROR) { continue; }//error accured
            if (recvPacket.GetPacketId() == -2) {//client requested to close connection
                break;
            }
            if (OnPacket != nullptr)
				OnPacket(*this,id,recvPacket);
        }
        if (OnClientDisconnected != nullptr) {
			OnClientDisconnected(*this,id);
        }
        clients[id]->Close();
        {
            std::lock_guard<std::mutex> guard(clientMutex);
            delete clients[id];
            clients.erase(id);
        }
        //cant delete thread here
    }

    CODE Server::SendToClient(int id, Packet &p) {
        std::lock_guard<std::mutex> guard(clientMutex);
        if (clients.find(id) == clients.end())return CODE::SERVER_CLIENT_NOT_FOUND;
        return clients[id]->Send(p);
    }

    CODE Server::SentToAll(Packet &packet) {
        std::lock_guard<std::mutex> guard(clientMutex);
        CODE error = CODE::OK;
        for (auto c: clients) {
            if (c.second->Send(packet) != CODE::OK) {
                error = CODE::SERVER_SEND_TO_ALL_FAILED;
            }
        }
        return error;
    }

    int Server::GetNextClientId() {
        static int nextclientId = 0;
        return nextclientId++;
    }

    Stream *Server::GetClientStream(int id) {
        if (clients.find(id) == clients.end())return nullptr;
        return clients[id];
    }

    void Server::Close() {
        isRunning = false;
        try {
            closesocket(sock);
        }
        catch (const std::exception &e) {}
        acceptThread->join();
        delete acceptThread;
        acceptThread = nullptr;
        {
            std::lock_guard<std::mutex> guard(clientMutex);
            std::map<int, Stream *> clientsCopy(clients);
            for (auto c: clientsCopy) {
                c.second->Close();
            }
            clientsCopy.clear();
        }
        for (auto t: threadClients) {
            if (t.second == nullptr)continue;
            t.second->join();
            delete t.second;
        }
        threadClients.clear();
        clients.clear();
    }

    void Server::DisconnectClient(int clientId) {
        std::lock_guard<std::mutex> guard(clientMutex);
        if (clients.find(clientId) == clients.end())return;
        clients[clientId]->Close();
    }

    Server::~Server() {
        if (isRunning) {
            Close();
        }
    }
}
