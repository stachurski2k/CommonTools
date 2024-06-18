#include "client.h"
#include "broadcaster.h"
#include<iostream>
namespace netlib {
    Client::Client() {
        stream = new Stream();
        reciveThread = nullptr;
        OnPacket = nullptr;
        OnDisconnected = nullptr;
        specialEventThread = nullptr;
    }

    CODE Client::ConnectToServer(std::string serverIp, int serverPort) {
        if (isConnected)return CODE::CLIENT_ALREADY_CONNECTED;
        if (InitWSA()!=CODE::OK) { return CODE::SOCKETS_NOT_SUPPORTED; }
        try {
            SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET)return CODE::SOCKET_CREATE_ERROR;
            sockaddr_in service;
            service.sin_family = AF_INET;
            service.sin_addr.s_addr =
                    inet_addr(serverIp.c_str());
            service.sin_port = htons(serverPort);
            int res = connect(sock, (sockaddr *) &service, sizeof(service));
            if (res == SOCKET_ERROR)
            {
                closesocket(sock);
                return CODE::CLIENT_CONNECTING_FAILED;
            }
            isConnected = true;
            stream->From(sock);
            if (reciveThread != nullptr) {
                reciveThread->join();
                delete reciveThread;
            }
            reciveThread = new std::thread(&Client::ReciveThread, this);
        }
        catch (const std::exception &e) {
            return CODE::CLIENT_CONNECTING_ERROR;
        }
        return CODE::OK;
    }
	void Client::DisconnectEvent() {
		OnDisconnected(*this);
	}
    void Client::ReciveThread() {
        while (isConnected) {
            Packet recvPacket(-1);
            CODE status = stream->Recive(recvPacket);
            if (status == CODE::STREAM_SOCKET_CLOSED) { break; }//connection closed
            if (status == CODE::STREAM_RECIVING_ERROR) { continue; }//error occured
			if (OnPacket != nullptr)OnPacket(*this,recvPacket);
        }
        isConnected = false;
        stream->Close();
        if (specialEventThread != nullptr) {
            specialEventThread->join();
            delete specialEventThread;
            specialEventThread = nullptr;
        }
        if (OnDisconnected != nullptr) {
			specialEventThread = new std::thread(&Client::DisconnectEvent,this);
        }
       // std::cout << "RECIVE THREAD CLOSED!" << '\n';
    }

    CODE Client::SendToServer(Packet &packet) {
        if (!isConnected)return CODE::CLIENT_NOT_CONNECTED;
        return stream->Send(packet);
    }

    void Client::Disconnect() {
        if (!isConnected)return;
        try {
            Packet closeConnectionRequest(-2);//in case some data is still in the buffer, close connection on the server side
            if (SendToServer(closeConnectionRequest) == CODE::OK) {
                while (isConnected) {
                    Packet recvPacket(-1);
                    CODE status = stream->Recive(recvPacket);
                    if (status == CODE::STREAM_SOCKET_CLOSED) { break; }//connection closed
                   // std:: cout << "WAITING FOR SERVER" << '\n';
                }
            }
            isConnected = false;
            stream->Close();
        } catch (const std::exception &e) {}
        //std::cout << "DISCONNECTED!" << '\n';
    }

    CODE Client::FindServerAndConnect(int timeoutMs,std::string preambule) {
        if (isConnected)return CODE::CLIENT_ALREADY_CONNECTED;
        Broadcaster caster;
        if(preambule!=""){
            caster.SetCustomPreambule(preambule);
        }
        if (caster.FindOneServer(timeoutMs) != CODE::OK) {
            return CODE::CLIENT_SERVER_NOT_FOUND_TIMEOUT;//server not found
        }
        std::pair<std::string, int> serverEp;
        if (caster.GetLastServer(serverEp) != CODE::OK) {
            return CODE::CLIENT_SERVER_NOT_FOUND;//fatal error
        }
        return ConnectToServer(serverEp.first, serverEp.second);
    }

    Client::~Client() {
        if (isConnected) {
            Disconnect();
        }
        if (reciveThread != nullptr) {
            reciveThread->join();
            delete reciveThread;
        }
        if (specialEventThread != nullptr) {
            specialEventThread->join();
            delete specialEventThread;
        }
        //std::cout << "CLIENT DELETED!" << '\n';
        delete stream;
    }

    void Client::reportError(int code) {
		if (code < 0 || code > errorCount - 1) return;
		//log_warn("Błąd sieci: {}", errors[code]);
	} 
}