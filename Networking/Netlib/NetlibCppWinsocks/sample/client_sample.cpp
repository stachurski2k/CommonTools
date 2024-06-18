#include "netlib/client.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "netlib/broadcaster.h"

using namespace std;
netlib::Client client;

//define your own packet types
enum class PacketType {
    PING = 1,
    PONG = 2,
};

//create a function that will be called whenever packet arives from server
void OnPacket(netlib::Client& caller,netlib::Packet& packet) {
    double send_time = packet.ReadDouble();
    double recv_time = packet.ReadDouble();
    std::cout << "Ping to server: " << recv_time - send_time << "ms \n";
}

//called when connection to server is lost,
void OnDisconnected(netlib::Client& caller) {
    cout << "Disconnected!" << '\n';
    client.FindServerAndConnect(100000);
}

//additional logic for pinging the server
void PingLogic() {
    while (true) {//when server is avilable ping it
        //packet creation an data writing

        if (client.IsConnected()) {
            netlib::Packet pingPacket((int) PacketType::PING);
            double curtime = (double) std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            pingPacket.Write(curtime);

            client.SendToServer(pingPacket);
        }
        Sleep(2000);
    }
}

int main() {
    char x;
    //atach your functions
    client.OnPacket = OnPacket;
    client.OnDisconnected = OnDisconnected;
    //try to connect to server
    if (client.FindServerAndConnect() !=netlib::CODE::OK) {
        cout << "Could not connect!" << '\n';
        cin >> x;
        return 1;
    }
    cout << "Connected To Server!" << '\n';
    //start ping logic
    std::thread pingThread(PingLogic);

    cin >> x;
    //disconnect from server
    client.Disconnect();
    pingThread.join();
}