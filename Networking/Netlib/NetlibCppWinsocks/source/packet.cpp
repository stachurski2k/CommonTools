#include "packet.h"

namespace netlib {
    Packet::Packet(int16_t _packetId) : readIndex(4), packetId(_packetId), datalen(0) {
        data = std::vector<char>(HeaderSize);//4 bytes for header
        data.reserve(64);
        writeIndex = Packet::HeaderSize;//start writing after header
    }

    Packet::Packet(const Packet &other) {
        packetId = other.packetId;
        datalen = other.datalen;
        readIndex = other.readIndex;
        writeIndex = other.writeIndex;
        data.resize(datalen+Packet::HeaderSize);
        memcpy(data.data(), other.data.data(), datalen + Packet::HeaderSize);
    }

    Packet Packet::InvalidPacket() {
        return Packet(-1);
    }

    const char *Packet::GetData() {
        Write(0, (void *) &packetId, 2);
        Write(2, (void *) &datalen, 2);
        return data.data();
    }

    std::pair<int16_t, int16_t> Packet::DeserializeHeader(std::vector<char> &v, int offset) {
        int16_t _packetId = *((int16_t *) v.data() + offset);
        int16_t _datalen = *((int16_t *) (v.data() + offset + 2));
        return {_packetId, _datalen};
    }

    void Packet::ResetWrite(int _packetId) {
        packetId = _packetId;
        datalen = 0;
        writeIndex = Packet::HeaderSize;
    }

    void Packet::ResetRead() {
        readIndex = Packet::HeaderSize;
    }

    CODE Packet::FromByteArray(Packet &p, std::vector<char> &v, int offset) {
        try{
            std::pair<int16_t, int16_t> header = Packet::DeserializeHeader(v, offset);
            p.packetId = header.first;
            p.datalen = header.second;
            p.data.resize(p.datalen+ Packet::HeaderSize);
            p.readIndex = Packet::HeaderSize;
            p.writeIndex = p.datalen + Packet::HeaderSize;
            memcpy((void *) p.data.data(), (void *) (v.data() + offset), p.datalen + Packet::HeaderSize);
        }catch(const std::exception& e){
            return CODE::PACKET_DESERIALIZE_FAILED;
        }
        return CODE::OK;
    }

#pragma region Writing

    void Packet::Write(int pos, void *var, int len) {
        if (data.size() >= data.capacity() - 2) {
            data.reserve(data.size() + 4 * len);//increase capacity
        }
        data.resize(writeIndex + len);
        void *pointer = (void *) (data.data() + pos);//move pointer to copy location
        memcpy(pointer, var, len);//copy len bytes
    }

    Packet &Packet::Write(char v) {
        Write(writeIndex, (void *) &v, 1);
        datalen += 1;
        writeIndex += 1;
        return *this;
    }

    Packet &Packet::Write(bool v) {
        Write(writeIndex, (void *) &v, 1);
        datalen += 1;
        writeIndex += 1;
        return *this;
    }

    Packet &Packet::Write(int32_t v) {
        Write(writeIndex, (void *) &v, 4);
        datalen += 4;
        writeIndex += 4;
        return *this;
    }

    Packet &Packet::Write(uint32_t v) {
        Write(writeIndex, (void *) &v, 4);
        datalen += 4;
        writeIndex += 4;
        return *this;
    }

    Packet &Packet::Write(float v) {
        Write(writeIndex, (void *) &v, 4);
        datalen += 4;
        writeIndex += 4;
        return *this;
    }

    Packet &Packet::Write(double v) {
        Write(writeIndex, (void *) &v, 8);
        datalen += 8;
        writeIndex += 8;
        return *this;
    }

    Packet &Packet::Write(std::string &v) {
        int32_t strsize = v.size();
        Write(strsize);
        Write(writeIndex, (void *) v.data(), v.size());
        datalen += v.size();
        writeIndex += v.size();
        return *this;
    }

    Packet &Packet::Write(std::vector<char> &v) {
        int32_t strsize = v.size();
        Write(strsize);
        Write(writeIndex, (void *) v.data(), v.size());
        datalen += v.size();
        writeIndex += v.size();
        return *this;
    }
    Packet& Packet::Write(std::array<uint8_t, 256>& v) {
        int32_t sajz = v.size();
        Write(sajz);
        Write(writeIndex, (void*)v.data(), v.size());
        datalen += v.size();
        writeIndex += v.size();
        return *this;
    }


#pragma endregion
#pragma region Reading

    bool Packet::CanRead(int x) {
        return readIndex + x <= datalen + Packet::HeaderSize;
    }

    void *Packet::Read(int position) {
        return (void *) (data.data() + position);
    }

    char Packet::ReadByte() {
        char *res = (char *) Read(readIndex);
        readIndex += 1;
        return *res;
    }

    bool Packet::ReadBool() {
        bool *res = (bool *) Read(readIndex);
        readIndex += 1;
        return *res;
    }

    int32_t Packet::ReadInt() {
        int32_t *res = (int32_t *) Read(readIndex);
        readIndex += 4;
        return *res;
    }

    uint32_t Packet::ReadUInt() {
        uint32_t *res = (uint32_t *) Read(readIndex);
        readIndex += 4;
        return *res;
    }

    float Packet::ReadFloat() {
        float *res = (float *) Read(readIndex);
        readIndex += 4;
        return *res;
    }

    double Packet::ReadDouble() {
        double *res = (double *) Read(readIndex);
        readIndex += 8;
        return *res;
    }

    std::string Packet::ReadString() {
        int32_t sajz = ReadInt();
        char *strchar = (char *) Read(readIndex);
        std::string s = std::string(strchar, strchar + sajz);
        readIndex += sajz;
        return s;
    }

    std::vector<char> Packet::ReadBytes() {
        int32_t sajz = ReadInt();
        std::vector<char> vec(sajz, 0);
        char *pointer = (char *) Read(readIndex);
        memcpy(vec.data(), pointer, sajz);
        readIndex += sajz;
        return vec;//vector is moved
    }
    std::array<uint8_t,256> Packet::ReadUint8Array256() {
        int32_t sajz = ReadInt();
        std::array<uint8_t,256> arr;
        char* pointer = (char*)Read(readIndex);
        memcpy(arr.data(), pointer, sajz);
        readIndex += sajz;
        return arr;//vector is moved
    }

#pragma endregion

    Packet::~Packet() {
        data.clear();
    }
}
