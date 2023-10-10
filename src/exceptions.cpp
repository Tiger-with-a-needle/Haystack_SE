#include "exceptions.h"

Exceptions::FileException::FileException(std::string missingFilesList) : missingFilesList(missingFilesList) {};

void Exceptions::FileException::what() const
{
    std::cerr << "\e[1;31mError:\e[0m missing critical file(s) - " << missingFilesList << "\nProgram terminated.\n";
}

Exceptions::ConfigException::ConfigException(bool isConfigFileOpen) : isConfigFileOpen(isConfigFileOpen) {};

void Exceptions::ConfigException::what() const
{
    std::cerr << (isConfigFileOpen ? "\e[1;31mError: \e[0mconfig.json is empty or corrupted\n\nProgram terminated\n" : "\e[1;31mError:\e[0m Could not open or create config.json / requests.json / answers.json \n\nProgram terminated\n");
}