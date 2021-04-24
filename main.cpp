//Maciej Skiba 171779
//Zadanie 01 Sieci Teleinformatyczne

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <ctime>
#include <bitset>
#include <cstdlib>
#include <math.h>

#define N    10
#define crc_  3


struct FileStruct{             //doing struct so I can return both -char* and -filelength in my "readFileBytes" function
    char* file;                //to use it in main()
    size_t filelength;
};


struct Header{
    size_t ID;
    int PacketSize;
};

template<typename T>
struct PacketStruct{                  // PACKET = [HEADER][DATA][CONTROL SUM]
    Header header;
    char PacketData[N];
    T controlSum;           //parity bit, modulo, crc have different sizes

};

/*
bool parityBit(Header h,      // EVEN = 1;  ODD = 0;

;
*/

FileStruct readFileBytes(const char *name)
{
    FileStruct MyFile;

    std::ifstream fl(name,std::ofstream::binary);


    fl.seekg( 0, std::ios::end );
    MyFile.filelength = fl.tellg();
    MyFile.file = new char[MyFile.filelength];
    fl.seekg(0, std::ios::beg);
    fl.read(MyFile.file, MyFile.filelength);
    fl.close();


    /*
     //function which converts some single byte (char) into 8 bits
    char c = MyFile.file[300];
    for (int i = 0; i < 8; ++i)
    {
        bool is_set = c & (1 << i);
        std::cout << "Bit " << i << ": " << is_set << '\n';
    }
    */

    return MyFile;
}

std::bitset<8> ToBits(char byte)
{
    return std::bitset<8>(byte);
}


bool parityBit(const std::string& packet, int n) // n= length of the packet (n bytes);     return 0= even 1=odd
{
    int sum= 0;
    for(int i=0; i<n; i++)
    {
        std::bitset<8> my_bset=ToBits(packet[i]);
        for(int j=0; j<8; j++)
        {
            sum+= my_bset[j];
        }
    }

    return sum%2;
}

int moduloSum(const std::string& packet, int n, int mod) // n= length of the packet (n bytes);     return 0= even 1=odd
{
    int sum= 0;
    for(int i=0; i<n; i++)
    {
        sum+=abs((int)packet[i]);
    }

   //std::cout << "sum = " << sum << std::endl;
    return sum%mod;
}

int crc(std::string packet, int n)
{

    std::bitset<N*8+crc_> b;          //N and crc_ defined by preprocessor
    for(int i=0; i<n; ++i)
    {
      char c = packet[i];
      for (int j = 7; j >= 0 && c; --j) {
        if (c & 0x1) {
          b.set(8 * i + j);
        }
        c >>= 1;
      }
    }
    b <<= crc_;    //move the bitset to prepare for calculating crc
    //std::cout << std::endl << "b=" << b << std::endl;

    //-------Preparing crc divider-------
    int power= pow(2,crc_) -1;
    int crc_rand= rand()%power;// e.g. for CRC-3 gives nr 1-15
    crc_rand+= pow(2,crc_); //adding extra "1" to the left

    //std::cout << "crc_rand= " << crc_rand << std::endl;
    std::bitset<N*8+crc_> b_divider(crc_rand);
    b_divider <<= N*8-1;

    //std::cout << "d=" << b_divider << std::endl;

    for(int i=0; i<= (N*8-1) ;i++)
    {
        if(b[N*8+crc_-i-1]==1)
        {
        //std::cout << std::endl << "b=" << b << std::endl;      //unccomment to see that CRC works
        //std::cout << "d=" << b_divider << std::endl;           //
        b^=b_divider;
        b_divider >>= 1;
        }
        else(b_divider >>= 1);
    }


    //std::cout << std::endl << "b final=" << b << std::endl;
    //std::cout << "d final=" << b_divider << std::endl;

    int crc_result;
    crc_result = (int)(b.to_ulong());
    //std::cout << crc_result << std::endl;

    return crc_result;

    /*
    //function which prints 8 bits of a byte
    for(int j=0; j<n; j++)
    {
        unsigned char c = packet[j];
        printf("char= %c\n", c);
        for (int i = 0; i < 8; ++i)
        {
            bool is_set = c & (1 << i);
            std::cout << "Bit " << i << ": " << is_set << '\n';
        }
    }
    */


}



void SendData(FileStruct const &MyFile, int n, std::list<int> &errors_list, std::string file_name)//
{
    bool moduloEqualsZero= true;
    size_t amountOfPackets = MyFile.filelength/n;     //divide file into n-bytes packets

    if((MyFile.filelength % n) != 0)       //e.g. 20bytes / 8bytes = 2, so we need amountOfPackets++ to handle it
    {
        amountOfPackets++;
        moduloEqualsZero= false;
    }


    std::ofstream outfile (file_name, std::ofstream::binary);


    for(int i=0; i < amountOfPackets; i++)   //making headers //for(int i=0; i < amountOfPackets; i++)
    {
        PacketStruct<int> packet;             //atm <int> only
        packet.header.ID = i;

        //-----------Write size of the packet to "header"-----------
        if(i == (amountOfPackets - 1) && moduloEqualsZero == false)
        {
            packet.header.PacketSize= MyFile.filelength % n;
        }
        else(packet.header.PacketSize= n);
        //----------------------------------------------------------

        //--------------Construct data of the packet-----------------

        //packet.PacketData.resize(packet.header.PacketSize);

        for(int k=0; k<packet.header.PacketSize; k++)
        {
            packet.PacketData[k] =  MyFile.file[i*n + k];

            if(errors_list.size()>0)
            {
                if(i*n+k == errors_list.front())      //modifying the byte to simulate "sending error"
                {
                    while(true)
                    {
                        packet.PacketData[k]= rand()%222 + 32;
                        if(packet.PacketData[k]!=MyFile.file[i*n + k])  //making sure its "a change"
                        {
                            false;
                        }
                    }

                    errors_list.pop_front();

                }
                else{
                    packet.PacketData[k] =  MyFile.file[i*n + k];

                }
            }



        }
        outfile.write(packet.PacketData, 10);

    //-----------Add control sum to the packet--------------

    packet.controlSum = moduloSum(packet.PacketData, n, 5);

    //printf("packet nr %d: ", i);


    //packet.controlSum = modulo(packet.PacketData, n);
    packet.controlSum = crc(packet.PacketData, n);
    //printf("\nCRC= %d\n", packet.controlSum);

    //---------Packet is set and ready to send----------


    }

}


void InvalidBytesIndexes(size_t filelength, float errorpercentage, std::list<int> &errors_list)   //function returns ar
{

    size_t amount_of_errors = (filelength*errorpercentage/100);


    for(size_t i=0; i < amount_of_errors; i++)
    {
        errors_list.push_back(std::rand()%filelength);

    }
    errors_list.sort();

    std::cout << std::endl << errors_list.size() << std::endl;
    printf("%s","teststring1");
}





int main()
{
    std::srand(std::time(nullptr));

    const char* file_name= "image.jpg";
    std::string new_file_name= "newimage.jpg";

    FileStruct ImportedFile= readFileBytes(file_name);




    size_t BytesLength = ImportedFile.filelength;

    std::cout << "Size of imported file= " << BytesLength << " bytes";

    std::list<int> error_indexes;
    std::cout << "Error indexes size: " << error_indexes.size();

    InvalidBytesIndexes(BytesLength, 0.01, error_indexes); //int = error percentage

    std::cout << "Error indexes size: " << error_indexes.size();

    SendData(ImportedFile, 10, error_indexes, new_file_name);      //


    printf("\nCRC-%i has been used\n", crc_);



    delete ImportedFile.file;


    return 0;

}
