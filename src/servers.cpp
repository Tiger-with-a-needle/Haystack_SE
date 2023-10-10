#include "servers.h"

using SearchServer = Servers::SearchServer;

const size_t MAX_WORDS_PER_KNOWLEDGE {1000}; // ТЗ - "документ содержит не более 1000 слов с максимальной длиной каждого в 100 символов"
const size_t MAX_WORD_SIZE {100 + 1};

void SearchServer::fetchKnowledge(const std::string &filePath, std::map<std::string, Servers::Entry> *storageCell, size_t fileId)
{
    if (std::ifstream file {filePath})
    {
        std::string tempWord;
        
        for (int wordCount {0}; file >> tempWord && wordCount < MAX_WORDS_PER_KNOWLEDGE; ++wordCount)
        {
            if (tempWord.size() < MAX_WORD_SIZE)
            {                             
                if (storageCell->count(tempWord)) storageCell->at(tempWord).count++;
                else storageCell->insert(std::pair<std::string, Servers::Entry>(tempWord, Servers::Entry {fileId, 1}));
            }
            else --wordCount;
        }

        file.close();
    }
    else std::cerr << "\nError reading file: thread(" << std::this_thread::get_id() << ") file(" << filePath << ")\n";
}

void SearchServer::fillOutDocumentBase(const std::vector<std::string> input_docs)
{
    const size_t docsListSize {input_docs.size()};

    std::vector<std::map<std::string, Servers::Entry>*> dataCells(docsListSize);
    std::for_each(dataCells.begin(), dataCells.end(), [] (auto &element) {
        element = new std::map<std::string, Servers::Entry>;
    });
    
    std::vector<std::thread*> threads(docsListSize);

    for (size_t cntr {0}; cntr < docsListSize; ++cntr)
        threads.at(cntr) = new std::thread(&fetchKnowledge, input_docs.at(cntr), dataCells.at(cntr), cntr);
    
    std::for_each(threads.begin(), threads.end(), [] (auto &thread) {
        if (thread->joinable()) thread->join();
        delete thread;
    });
    
    if (!freq_dictionary.empty()) freq_dictionary.clear();

    for (auto& storageCell : dataCells)
    {
        for (auto& cellEntry : *storageCell)
        {
            if (freq_dictionary.count(cellEntry.first)) freq_dictionary.at(cellEntry.first).push_back(cellEntry.second);
            else freq_dictionary[cellEntry.first] = std::vector<Servers::Entry> {cellEntry.second};
        }
    }

    for (auto& storageCell : dataCells) delete storageCell;
}

std::vector<std::vector<Servers::RelativeIndex>> SearchServer::search(const std::vector<std::string> requestsList, Telemetry::Timer timer)
{
    const size_t requestsListSize {requestsList.size()};
    std::vector<std::vector<Servers::RelativeIndex>> searchResult(requestsListSize);
    
    auto searchThread = [&] (const std::string &currentRequest, std::vector<Servers::RelativeIndex> *searchResultEntry) -> void {

        std::unordered_set<std::string> uniqueWordsRequest;
        std::multimap<size_t, std::vector<Servers::Entry>*> requestFrequency;

        std::stringstream ss(currentRequest);
        bool isRequestValid {true};
     
        for (std::string currentWord; ss >> currentWord; )
        {
            if (!freq_dictionary.count(currentWord))
            {
                isRequestValid = false;
                break;
            }
            else if (!uniqueWordsRequest.count(currentWord))
            {
                uniqueWordsRequest.insert(currentWord);
                requestFrequency.insert(std::pair<size_t, std::vector<Servers::Entry>*>(freq_dictionary.at(currentWord).size(), &freq_dictionary.at(currentWord)));
            }
        }

        if (isRequestValid)
        {
            double absoluteRelevance {0};

            std::multimap<size_t, size_t> requestStats;

            for (auto& rarestWordStats : *requestFrequency.begin()->second)
            {
                size_t cumulativeRelevance {rarestWordStats.count};
                
                for (auto& otherWords {++requestFrequency.begin()}; otherWords != requestFrequency.end(); ++otherWords)
                {
                    isRequestValid = false;
                    
                    for (auto& otherWordsStatsEntry : *otherWords->second)
                    {
                        if (rarestWordStats.doc_id == otherWordsStatsEntry.doc_id)
                        {
                            cumulativeRelevance += otherWordsStatsEntry.count;
                            isRequestValid = true;
                            break;
                        }
                    }

                    if (!isRequestValid) break;
                }

                if (isRequestValid)
                {
                    requestStats.insert(std::pair<size_t, size_t>(cumulativeRelevance, rarestWordStats.doc_id));
                    if (absoluteRelevance < cumulativeRelevance) absoluteRelevance = cumulativeRelevance;
                }
            }
            
            std::map<size_t, size_t>::reverse_iterator requestStatsRevEnd {requestStats.rend()};

            for (std::map<size_t, size_t>::reverse_iterator rit {requestStats.rbegin()}; rit != requestStatsRevEnd; ++rit)
                searchResultEntry->push_back(Servers::RelativeIndex (rit->second, rit->first / absoluteRelevance));
        }
    };

    std::vector<std::thread*> threads(requestsListSize);
    
    for (size_t cntr {0}; cntr < requestsListSize; ++cntr)
        threads.at(cntr) = new std::thread(searchThread, requestsList.at(cntr), &searchResult.at(cntr));
    
    std::for_each(threads.begin(), threads.end(), [] (auto &thread) {
        if (thread->joinable()) thread->join();
        delete thread;
    });

    timer.check();
    
    return searchResult;
}