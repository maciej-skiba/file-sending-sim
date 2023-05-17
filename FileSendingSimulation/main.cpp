#include "functions.hpp"


int main()
{
    std::srand(std::time(nullptr));

    std::string file_name;
    std::string new_file_name;
    char integrityCheckChoice;

    try
    {
        std::cout << "Type name of file-to-import (e.g. \"image.jpg\"): ";
        std::cin >> file_name;
        new_file_name = "new" + file_name;
        std::cout << "Choose type of packet integrity check.\n1 = parity bit\n2 = modulo sum\n3 = crc\nOption: ";
        std::cin >> integrityCheckChoice;
    }
    catch (const std::exception& e)
    {
        std::cout << "Caught exception of type: " << typeid(e).name() << "\nProgram will terminate.";
        std::exit(0);
    }
    catch (...)
    {
        std::cout << "Unknown exception occurred.\nProgram will terminate.";
        std::exit(0);
    }
 
    FileStruct ImportedFile = readFileBytes(file_name);

    size_t BytesLength = ImportedFile.filelength;

    std::cout << "Size of imported file= " << BytesLength << " bytes\n";

    std::list<int> error_indexes;
    std::cout << "Error indexes size: " << error_indexes.size() << std::endl;

    InvalidBytesIndexes(BytesLength, 0.01f, error_indexes); //int = error percentage

    std::cout << "Error indexes size: " << error_indexes.size();

    SendData(ImportedFile, 10, error_indexes, new_file_name, static_cast<IntegrityCheck>(integrityCheckChoice));
    printf("\nCRC-%i has been used\n", CRC);

    delete ImportedFile.file;

    return 0;
}
