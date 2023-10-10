#include "converters.h"

using json = nlohmann::json;
using Converter = Converters::ConverterJSON;
namespace fs = std::filesystem;

const char* CONFIG_FILE     {"config/config.json"};
const char* REQUESTS_FILE   {"requests/requests.json"};
const char* ANSWERS_FILE    {"answers/answers.json"};

const size_t MAX_REQUESTS {1000};           // ТЗ - "Список содержит не более 1000 запросов, каждый из которых включает от одного до десяти слов"
const size_t RESPONSES_LIMIT_MAX {100};     // максимальное кол-во ответов на один запрос
const size_t RESPONSES_LIMIT_DEFAULT {5};   // значение по-умолчанию, если в файле config.json указана величина < 1

std::vector<std::string> Converter::getTextDocuments() const
{
    std::cout << "\e[1;34m\nConfig.json \"files\" array status check ...\e[0m\n";
    std::ifstream configFile(CONFIG_FILE);
    if (!configFile.is_open()) throw Exceptions::ConfigException(false);

    json data = json::parse(configFile);
    configFile.close();

    std::vector<std::string> textFilesList;
    textFilesList.reserve(data["files"].size());

    fs::path currentFile;

    for (auto& [index, filePath] : data["files"].items())
    {
        currentFile.assign(filePath.get<std::string>());
        
        if (fs::exists(currentFile))
        {
            textFilesList.push_back(currentFile.string());
            std::cout << textFilesList.back() << " [\e[1;32mOk\e[0m]\n";
        }
        else std::cout << currentFile.string() << " [\e[1;31mMissing\e[0m]\n";
    }

    return textFilesList;
}

std::vector<std::string> Converter::getRequests() const
{
    std::cout << "\e[1;34m\nGetting and processing list of requests from requests.json ...\e[0m\n";
    std::ifstream requestsFile(REQUESTS_FILE);
    if (!requestsFile.is_open()) throw Exceptions::ConfigException(false);

    json data = json::parse(requestsFile);
    requestsFile.close();

    size_t requestsListSize {data["requests"].size()};
    auto requestsEnd = data["requests"].end();

    if (requestsListSize > MAX_REQUESTS)
    {
        requestsListSize = MAX_REQUESTS;
        requestsEnd = data["requests"].begin() + MAX_REQUESTS;
    }

    std::vector<std::string> requestsList;
    requestsList.reserve(requestsListSize);

    for (json::iterator json_it = data["requests"].begin(), requestsLimit = data["requests"].end(); json_it != requestsEnd; ++json_it)
    {
        std::string currentRequestString {json_it.value().get<std::string>()};
        auto str_end {currentRequestString.end()};
        int wordCounter {currentRequestString.length() && currentRequestString[0] != ' ' ? 1 : 0};

        for (std::string::iterator str_it = currentRequestString.begin(); str_it != str_end; ++str_it) if (*str_it == ' ' && (str_it + 1 != str_end && *(str_it + 1) != ' ')) ++wordCounter;

        if (wordCounter > 0 && wordCounter < 11) requestsList.push_back(currentRequestString);
        else if (requestsEnd != requestsLimit) ++requestsEnd;
    }

    return requestsList;
}

void Converter::putAnswers(const std::vector<std::vector<Servers::RelativeIndex>> answers, Telemetry::Timer timer)
{
    json data;
    size_t answersVecSize {answers.size()};
    std::string requestFinalString("request0000");

    for (size_t currentRequestNumber {0}; currentRequestNumber < answersVecSize; ++currentRequestNumber)
    {
        std::sprintf(&requestFinalString[7], "%04lu", currentRequestNumber + 1);

        if (!answers.at(currentRequestNumber).empty())
        {
            data["answers"][requestFinalString]["result"] = true;

            if (answers.at(currentRequestNumber).size() == 1 || max_responses == 1)
            {
                data["answers"][requestFinalString]["docid"] = answers.at(currentRequestNumber)[0].doc_id;
                data["answers"][requestFinalString]["rank"] = answers.at(currentRequestNumber)[0].rank; // для красивого вывода внёс изменения в json.hpp, строки с 18748 с комментарием YMS
            }
            else
            {
                std::array<char, 6> rankString {'\0'};
                char *rankData {rankString.begin()};
                
                for (auto& relevanceEntry : answers.at(currentRequestNumber))
                {                    
                    if (&relevanceEntry - &*answers.at(currentRequestNumber).begin() == max_responses) break;
                    std::sprintf(rankData, "%.3f", relevanceEntry.rank);
                    std::string relevanceString {"{\"docid\": "};
                    relevanceString += std::to_string(relevanceEntry.doc_id) + ", \"rank\": " + rankData + "}";
                    data["answers"][requestFinalString]["relevance"].push_back(relevanceString);
                }
            }
        }
        else data["answers"][requestFinalString]["result"] = false;
    }
    
    std::cout << "\e[1;34m\nSaving resulting data to answers.json ...\e[0m\n";
    std::ofstream answersFile(ANSWERS_FILE);
    if (!answersFile.is_open()) throw Exceptions::ConfigException(false);

    answersFile << std::setw(4) << data << std::endl;
    answersFile.close();

    timer.check();
    std::cout << "\e[1;34m\nData serialized successfully.\e[0m\n";
}

Converter::ConverterJSON(const std::string engineVersion)
{
    std::cout << "\e[1;34mCritical files status check ...\e[0m\n";
    std::string missingFilesList;

    if (!fs::exists(fs::path(CONFIG_FILE)))
    {
        missingFilesList = CONFIG_FILE;
        if (!fs::exists(fs::path(REQUESTS_FILE))) (missingFilesList += ", ") += REQUESTS_FILE;
        throw Exceptions::FileException(missingFilesList);
    }
    else if (!fs::exists(fs::path(REQUESTS_FILE)))
    {
        missingFilesList = REQUESTS_FILE;
        throw Exceptions::FileException(missingFilesList);
    }
    else std::cout << CONFIG_FILE << "\t[\e[1;32mOk\e[0m]\n" << REQUESTS_FILE << "\t[\e[1;32mOk\e[0m]\n";
    
    std::cout << "\e[1;34m\nCritical files integrity check ...\e[0m\n";
    std::ifstream configFile(CONFIG_FILE);
    if (!configFile.is_open()) throw Exceptions::ConfigException(false);

    json data = json::parse(configFile);
    configFile.close();
    
    if (!data.count("config") || !data.count("files")) throw Exceptions::ConfigException(true);
    std::cout << "config.json\t[\e[1;32mOk\e[0m]\n";

    const int maxResponsesRead {data["config"]["max_responses"]};
    if (maxResponsesRead < 1) max_responses = RESPONSES_LIMIT_DEFAULT;
    else if (maxResponsesRead > RESPONSES_LIMIT_MAX) max_responses = RESPONSES_LIMIT_MAX;
    else max_responses = maxResponsesRead;

    if (data["config"]["version"] != engineVersion) std::cout << "\e[1;33mWarning:\e[0m config.json version differs from the engine one [config: " << data["config"]["version"].get<std::string>() << " | engine: " << engineVersion << "]\n";
    std::cout << "\n\e[1;34m" << data["config"]["name"].get<std::string>() << " is running ...\e[0m\n";
}