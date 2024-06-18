#include "netlib/server.h"
#include <iostream>
#include "netlib/broadcaster.h"

using namespace std;
//define your packet types
enum class PacketType {
    PING = 1,
    PONG = 2,
};
netlib::Server server;

//called when packet is coming from client
void OnPacket(netlib::Server& caller,int clientId, netlib::Packet& packet) {
    //print info about packet
    std::cout << "Packet from " << clientId << " type : " << packet.GetPacketId() << '\n';
    double send_time = packet.ReadDouble();
    Sleep(10);//simulate some delay
    double curtime = (double) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    //after reading data, reset packet to and write some data
    packet.ResetWrite((int) PacketType::PONG);
    packet.Write(send_time).Write(curtime);
    server.SendToClient(clientId, packet);
    //server.DisconnectClient(clientId);
}

//new client
void OnClientConnected(netlib::Server& caller, int newClientId) {
    //get client stream to obtain more info about his ip and port
    netlib::Stream *s = server.GetClientStream(newClientId);
    cout << "Client Connected id= " << newClientId << " ";
    cout << s->remoteIp << " " << s->remotePort << '\n';
}

//client disconnected
void OnClientDisconnected(netlib::Server& caller, int clientId) {
    cout << "Client Disconnected id=" << clientId << '\n';
}

int main() {
    //attach callbacks
    server.OnClientConnected = OnClientConnected;
    server.OnPacket = OnPacket;
    server.OnClientDisconnected = OnClientDisconnected;
    //start
    netlib::CODE res = server.Start(8080);
    if (res != netlib::CODE::OK) {
        cout<<netlib::getCodeDescription(res) << '\n';
        return 1;
    }
    cout << "Server up and running!" << '\n';
    netlib::Broadcaster caster;
    if (caster.StartBroadcast(server.GetPort(), 400) != netlib::CODE::OK) {
        cout << "Failed to start broadcast!" << '\n';
    }
    char x;
    cin >> x;
    //close every thing
    server.Close();
    return 0;
}