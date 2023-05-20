#include "functions.hpp"

int main()
{
    std::srand(std::time(nullptr));

    std::string file_name = "image.jpg";
    std::string new_file_name;
    float error_percentage = 0.01f;
    int integrityCheckChoice = 0;

    WelcomeMessage();

    try
    {
        std::cout << "Type name of file-to-import (e.g. \"image.jpg\"): ";
        std::cin >> file_name;
        new_file_name = "new" + file_name;
        std::cout << "Type percentage of file bytes that should be corrupted (suggested = 0.01): ";
        std::cin >> error_percentage;
        std::cout << "\nChoose type of packet integrity check.\n1 = Parity Bit (not recommended, the least secure)" << 
            "\n2 = Modulo Sum\n3 = Crc\nOption: ";
        std::cin >> integrityCheckChoice;

        PrintSectionEnd();

        FileStruct ImportedFile = readFileBytes(file_name);
        size_t BytesLength = ImportedFile.filelength;
        std::cout << "Size of imported file= " << BytesLength << " bytes";

        PrintSectionEnd();

        std::list<int> error_indexes;

        InvalidBytesIndexes(BytesLength, error_percentage, error_indexes, N);

        SendData(ImportedFile, 10, error_indexes, new_file_name, static_cast<CheckSum>(integrityCheckChoice));

        delete ImportedFile.file;

        std::cout << "\nInspect " << new_file_name << " file to see the received file.";

        return 0;
    }
    catch (WrongCheckSumException e)
    {
        e.PrintException();
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
}
