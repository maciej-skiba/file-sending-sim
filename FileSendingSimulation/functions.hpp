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
#define CRC  3
#define MODULO 5

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
    T controlSum;           //parity bit, modulo, crc have different sizes
};

enum IntegrityCheck
{
    ParityBit = 1,
    ModuloSum = 2,
    Crc = 3
};

FileStruct readFileBytes(std::string);

std::bitset<8> ToBits(char);

bool parityBit(const std::string&, int);

int moduloSum(const char[], int, int);

int crc(const char[], int);

void SendData(FileStruct const&, int, std::list<int>&, std::string, IntegrityCheck);

void SendPacket(PacketStruct<int>&, std::list<int>&, FileStruct const&, int, int);

void InvalidBytesIndexes(size_t, float, std::list<int>&);