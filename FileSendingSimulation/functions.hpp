#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <ctime>
#include <bitset>
#include <cstdlib>
#include <math.h>

#define N    10
#define MODULO 64
#define CRC  8
#define CRC_RANDOM 123456789

struct FileStruct {             //doing struct so I can return both -char* and -filelength in my "readFileBytes" function
    char* file;                //to use it in main()
    size_t filelength;
};

struct Header {
    size_t ID;
    int PacketSize;
};

template<typename T>
struct PacketStruct {                  // PACKET = [HEADER][DATA][CONTROL SUM]
    Header header;
    char PacketData[N];
    T checkSum;           //parity bit, modulo, crc have different sizes
};

enum class CheckSum
{
    ParityBit = 1,
    ModuloSum = 2,
    Crc = 3
};

class WrongCheckSumException
{
public:
    void PrintException();
};

FileStruct readFileBytes(std::string);

std::bitset<8> ToBits(char);

int ParityBit(const char[], int);

int ModuloSum(const char[], int);

int Crc(const char[], int);

void SendData(FileStruct const&, int, std::list<int>&, std::string, CheckSum);

void SendPacket(PacketStruct<int>&, std::list<int>&, FileStruct const&, int, int, const CheckSum);

void InvalidBytesIndexes(size_t, float, std::list<int>&, int);

void ReceiveAndVerifyPacket(const PacketStruct<int>&, CheckSum, const int);

int ReturnPacketCheckSum(const PacketStruct<int>&, CheckSum, const int);

void PrintSectionEnd();

void WelcomeMessage();