#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include "exceptions.h"
#include "json.hpp"
#include "telemetry.h"

namespace Servers
{
    struct RelativeIndex
    {
        size_t doc_id;
        double rank;

        RelativeIndex(const size_t doc_id, const double rank) : doc_id(doc_id), rank(rank) {};

        bool operator== (const RelativeIndex &other) const  // для использовании в тестах фреймворка GoogleTest
        {
            return (doc_id == other.doc_id && (fabs(rank - other.rank) < 0.001));
        }
    };
}

namespace Converters
{
    class ConverterJSON
    {
        size_t max_responses;

    public:
        ConverterJSON(const std::string);

        std::vector<std::string> getTextDocuments() const; // возвращает список путей к файлам, перечисленных в config.json
        
        std::vector<std::string> getRequests() const; // возвращает список запросов из файла requests.json
        
        void putAnswers(const std::vector<std::vector<Servers::RelativeIndex>>, Telemetry::Timer); // размещает в файле answers.json результаты поисковых запросов
    };
}