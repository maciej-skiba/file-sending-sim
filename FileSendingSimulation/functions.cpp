#include "functions.hpp"

/// <summary>
/// Reads file at puts it to MyFile structure.
/// </summary>
/// <param name="fileName">Name of the filename, which must be in project folder</param>
/// <returns>Imported file in FileStruct object</returns>
FileStruct readFileBytes(std::string fileName)
{
    FileStruct MyFile;

    try
    {
        std::ifstream fl(fileName, std::ofstream::binary);

        fl.seekg(0, std::ios::end);
        MyFile.filelength = fl.tellg();
        MyFile.file = new char[MyFile.filelength];
        fl.seekg(0, std::ios::beg);
        fl.read(MyFile.file, MyFile.filelength);
        fl.close();

        return MyFile;
    }
    catch (std::bad_array_new_length&)
    {
        std::cout << "There was problem with assigning file length.\nProgram will terminate.";
        std::exit(0);
    }
    catch (const std::exception &e)
    {
        std::cout << "Caught exception of type: " << typeid(e).name() << "\nProgram will terminate.";
        std::exit(0);
    }
    catch (...)
    {
        std::cout << "Unknown exception occurred.\nProgram will terminate.";
        std::exit(0);
    }
}

/// <summary>
/// Bytes to bitset<8> conversion function
/// </summary>
/// <param name="byte">Byte to convert</param>
/// <returns>8-bit set</returns>
std::bitset<8> ToBits(char byte)
{
    return std::bitset<8>(byte);
}

/// <summary>
/// Calculates parity bit of data packet. 
/// </summary>
/// <param name="packet">Packet of bytes</param>
/// <param name="PacketLength">Packet length</param>
/// <returns>return 0= even 1=odd</returns>
int ParityBit(const char packet[], int PacketLength)
{
    int sum = 0;

    for (int i = 0; i < PacketLength; i++)
    {
        std::bitset<8> my_bset = ToBits(packet[i]);
        for (int j = 0; j < 8; j++)
        {
            sum += my_bset[j];
        }
    }

    return sum % 2;
}

/// <summary>
/// Calculates modulo sum of data packet.
/// </summary>
/// <param name="packet">Packet of bytes</param>
/// <param name="PacketLength">Packet length</param>
/// <returns>Sum of bytes in packets #modulo operation# N</returns>
int ModuloSum(const char packet[], int PacketLength) 
{
    int sum = 0;
    for (int i = 0; i < PacketLength; i++)
    {
        sum += abs((int)packet[i]);
    }

    return sum % MODULO;
}
/// <summary>
/// Calculates packet's Crc checksum
/// </summary>
/// <param name="packet"></param>
/// <param name="PacketLength">Packet length</param>
/// <returns>Crc checksum result</returns>
int Crc(const char packet[], int packetLength)
{
    std::bitset<N * 8 + CRC> b;          //N and crc_ defined by preprocessor

    for (int i = 0; i < packetLength; ++i)
    {
        char c = packet[i];
        for (int j = 7; j >= 0 && c; --j) {
            if (c & 0x1) {
                b.set(8 * i + j);
            }
            c >>= 1;
        }
    }

    b <<= CRC;    //move the bitset to prepare for calculating crc

    //-------Preparing crc divider-------
    int power = pow(2, CRC) - 1;
    int crc_rand = CRC_RANDOM % power;// e.g. for CRC-3 gives nr 1-15
    crc_rand += pow(2, CRC); //adding extra "1" to the left

    std::bitset<N * 8 + CRC> b_divider(crc_rand);
    b_divider <<= N * 8 - 1;

    for (int i = 0; i <= (N * 8 - 1);i++)
    {
        if (b[N * 8 + CRC - i - 1] == 1)
        {
            //std::cout << std::endl << "b=" << b << std::endl;      //unccomment to see that CRC works
            //std::cout << "d=" << b_divider << std::endl;           //
            b ^= b_divider;
            b_divider >>= 1;
        }
        else(b_divider >>= 1);
    }

    int crc_result;
    crc_result = (int)(b.to_ulong());

    return crc_result;
}


/// <summary>
/// Simulates sending data 
/// </summary>
/// <param name="MyFile"> Imported file as str</param>
/// <param name="packetSize">Size of packet</param>
/// <param name="errors_list">List or corrupted bytes</param>
/// <param name="file_name"></param>
/// <param name="checkSumChoice">Parity bit/Modulo sum/Crc</param>
void SendData(FileStruct const& MyFile, int packetSize, std::list<int>& errors_list,
               std::string file_name, const CheckSum checkSumChoice)
{
    bool moduloEqualsZero = true;
    size_t amountOfPackets = MyFile.filelength / packetSize;     //divide file into n-bytes packets

    if ((MyFile.filelength % packetSize) != 0)       //e.g. 20bytes / 8bytes = 2, so we need amountOfPackets++ to handle it
    {
        amountOfPackets++;
        moduloEqualsZero = false;
    }

    std::ofstream outfile(file_name, std::ofstream::binary);

    for (int i = 0; i < amountOfPackets; i++)   //making headers
    {
        PacketStruct<int> packet;
        packet.header.ID = i;

        //-----------Write size of the packet to "header"-----------

        if (i == (amountOfPackets - 1) && moduloEqualsZero == false)
        {
            packet.header.PacketSize = MyFile.filelength % packetSize;
        }
        else (packet.header.PacketSize = packetSize);

        //----------------------------------------------------------
        
        SendPacket(packet, errors_list, MyFile, packetSize, i, checkSumChoice);
        
        ReceiveAndVerifyPacket(packet, checkSumChoice, packetSize);
        
        outfile.write(packet.PacketData, 10);
    }

    PrintSectionEnd();
}

/// <summary>
/// Function that randomly generates which bytes of the file should be modified
/// </summary>
/// <param name="filelength"> Length of imported file in bytes </param>
/// <param name="errorpercentage"> How many bytes of the file should be changed </param>
/// <param name="errors_list">List or corrupted bytes</param>
/// <param name="packetSize">Size of packet</param>
void InvalidBytesIndexes(size_t filelength, float errorpercentage, std::list<int>& errors_list, int packetSize)
{
    std::cout << "Simulating packet corruption...\n\n";

    size_t amount_of_errors = (filelength * errorpercentage / 100);

    for (size_t i = 0; i < amount_of_errors; i++)
    {
        errors_list.push_back(std::rand() % filelength);
    }
    errors_list.sort();

    for (auto corruptedByte : errors_list)
    {
        int corruptedPacketNumber = corruptedByte / packetSize;
        std::cout << "Changing file's byte number " << corruptedByte << " (packet no. " << corruptedPacketNumber << ")\n";
    }

    PrintSectionEnd();
}

/// <summary>
/// Simulates sending packet with data, which means either assigning packet byte to a proper file byte
/// or to a corrupted file byte (to simulate "hum").
/// </summary>
/// <param name="packet">Packet struct</param>
/// <param name="errors_list">List of corrupted bytes</param>
/// <param name="MyFile">File to read</param>
/// <param name="packetSize">Packet size</param>
/// <param name="packetIndex">Index of packet</param>
/// <param name="checkSumChoice">Parity bit/Modulo sum/Crc</param>
void SendPacket(PacketStruct<int> &packet, std::list<int> &errors_list, FileStruct const& MyFile, 
    int packetSize, int packetIndex, const CheckSum checkSumChoice)
{
    for (int k = 0; k < packet.header.PacketSize; k++)
    {
        packet.PacketData[k] = MyFile.file[packetIndex * packetSize + k];
    }

    packet.checkSum = ReturnPacketCheckSum(packet, checkSumChoice, packetSize);

    // Byte corruption; "Hum" simulation
    for (int k = 0; k < packet.header.PacketSize; k++)
    {
        if (errors_list.size() > 0)
        {
            packet.PacketData[k] = MyFile.file[packetIndex * packetSize + k];

            if (packetIndex * packetSize + k == errors_list.front())      //modifying the byte to simulate "sending error"
            {
                while (true)
                {
                    packet.PacketData[k] = rand() % 222 + 32;

                    if (packet.PacketData[k] != MyFile.file[packetIndex * packetSize + k])  //making sure its "a change"
                    {
                        break;
                    }
                }
                errors_list.pop_front();
            }
        }
    }
}

/// <summary>
/// Receive and verify packet integrity using checksum function
/// </summary>
/// <param name="packet">Packet struct</param>
/// <param name="checkSumChoice">Parity bit/Modulo sum/Crc</param>
/// <param name="packetSize">Packet size</param>
void ReceiveAndVerifyPacket(const PacketStruct<int> &packet, CheckSum checkSumChoice, const int packetSize)
{
    int checkSumOfReceivedPacket = ReturnPacketCheckSum(packet, checkSumChoice, packetSize);

    if (checkSumOfReceivedPacket != packet.checkSum)
    {
        std::cout << "Packet number " << packet.header.ID << " is corrupted!\n";
    }
}

/// <summary>
/// Return checksum of packet
/// </summary>
/// <param name="packet">Packet struct</param>
/// <param name="checkSumChoice">Parity bit/Modulo sum/Crc</param>
/// <param name="packetSize">Packet size</param>
/// <returns>Packet checksum</returns>
int ReturnPacketCheckSum(const PacketStruct<int> &packet, const CheckSum checkSumChoice, const int packetSize)
{
    switch (checkSumChoice)
    {
        case CheckSum::ParityBit:
            return ParityBit(packet.PacketData, packetSize);
            break;
        case CheckSum::ModuloSum:
            return ModuloSum(packet.PacketData, packetSize);
            break;
        case CheckSum::Crc:
            return Crc(packet.PacketData, packetSize);
            break;
    }

    throw WrongCheckSumException();
}

/// <summary>
/// Print section end with dashes
/// </summary>
void PrintSectionEnd()
{
    std::cout << "\n-------------------------------------\n";
}

/// <summary>
/// Welcome message at the begin
/// </summary>
void WelcomeMessage()
{
    std::cout << "######################################\n";
    std::cout << "####### FILE SENDING SIMULATOR #######\n";
    std::cout << "######################################\n\n";

    std::cout << "This program simulates sending a file as packets of data\nand corrupts some of the bytes " <<
        "to simulate how received files would look like\nif there was no packet integrity checking.\n\n" <<
        "To visualize the result as \"newimage.jpg\", it is recommended to use \"image.jpg\" as testing file,\n" <<
        "which is included in the project directory.\n\n";

    std::cout << "WARNING! Program is not malicious input proof, so please follow the input instructions.\n";

    PrintSectionEnd();
    std::cout << std::endl;
}

void WrongCheckSumException::PrintException()
{
    std::cout << "Error! You entered wrong number while choosing the checksum!\n";
}
