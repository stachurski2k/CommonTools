#ifndef SKG_NETLIB_PACKET_H
#define SKG_NETLIB_PACKET_H

#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include "codes.h"
///Assuming:
///Packet consists of PacketId(2 bytes), DataLengthInBytes(2 bytes), PacketData(X bytes)
///Packet does not support reading and writing at the same time!
///Note:
///Vector does not act like a pointer when coping, instead whole vector is copied
///Warning: pointer to vector data might change
namespace netlib {
    class Packet {
        std::vector<char> data;//actual data to send, first 4 bytes are header
        int readIndex = 4;
        int writeIndex = 4;
        int16_t datalen = 0;
        int16_t packetId = 0;//should be >=0, -1 is considered invalid packet,<0 are for internal use of this lib
        void Write(int position, void *var, int len);

        void *Read(int position);

    public:
        static constexpr int HeaderSize = 4;

        static CODE FromByteArray(Packet &p, std::vector<char> &v, int offset);//CODE:OK or error code

        Packet(int16_t _packetId);//packet should always have an id
        Packet(const Packet &other);//vector is copied as well

        static Packet InvalidPacket();//packet with id=-1

        int16_t GetPacketId() { return packetId; }

        int16_t GetDataLen() { return datalen; }

        //resets for writing(index)
        void ResetWrite(int packetId);
        //resets for reading(index)
        void ResetRead();

//Core functionality
        const char *GetData();//returns pointer to data ready for sending
        //packetId,datalen
        static std::pair<int16_t, int16_t> DeserializeHeader(std::vector<char> &v, int offset = 0);

        bool CanRead(int xbytes);

//Writing Data
        Packet &Write(char v);

        Packet &Write(bool v);

        Packet &Write(int32_t v);

        Packet &Write(uint32_t v);

        Packet &Write(float v);

        Packet &Write(double v);

        Packet &Write(std::string &v);

        Packet &Write(std::vector<char> &v);
        Packet& Write(std::array<uint8_t,256>& v);

//Reading Data
        char ReadByte();

        bool ReadBool();

        int32_t ReadInt();

        uint32_t ReadUInt();

        float ReadFloat();

        double ReadDouble();

        std::string ReadString();

        std::vector<char> ReadBytes();

        std::array<uint8_t, 256> ReadUint8Array256();
        ~Packet();
    };
}

#endif //SKG_NETLIB_PACKET_H