#include <iostream>
#include <string>

namespace Exceptions
{ 
    struct GeneralException
    {
        GeneralException() = default;
        virtual ~GeneralException() {};
        virtual void what() const = 0;
    };
    
    struct FileException : GeneralException
    {
        std::string missingFilesList;
        
        FileException(std::string);
        void what() const override final;
    };

    struct ConfigException : GeneralException
    {
        bool isConfigFileOpen;

        ConfigException(bool);
        void what() const override final;
    };
}