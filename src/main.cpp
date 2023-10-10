#include "servers.h"

using Converter = Converters::ConverterJSON;
using SearchServer = Servers::SearchServer;
using Timer = Telemetry::Timer;

const char* ENGINE_VERSION = "0.4.2"; // current Haystack Search Engine version

int main()
{
    try
    {
        Converter converter(ENGINE_VERSION);

        SearchServer sServer(converter.getTextDocuments(), Timer("Search server construction and initial dictionary fill"));

        converter.putAnswers(sServer.search(converter.getRequests(), Timer("Search process")), Timer("Serialization process"));
    }
    catch (const Exceptions::GeneralException& e)
    {
        e.what();
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}