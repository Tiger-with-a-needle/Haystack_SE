#include <algorithm>
#include <initializer_list>
#include <map>
#include <sstream>
#include <thread>
#include <unordered_set>
#include <utility>
#include "converters.h"

namespace Servers
{
    struct Entry
    {
        size_t doc_id;  // номер документа по порядку из файла config.json
        size_t count;   // кол-во вхождений слова в рамках doc_id документа

        Entry(const std::initializer_list<size_t> initList) : doc_id(*initList.begin()), count(*(initList.begin() + 1)) {};

        bool operator== (const Entry &other) const  // для использовании в тестах фреймворка GoogleTest
        {
            return (doc_id == other.doc_id && count == other.count);
        }
    };

    class SearchServer
    {
        static void fetchKnowledge(const std::string &, std::map<std::string, Servers::Entry> *, size_t); // метод для многопоточного наполнения частотного словаря
    
    public:
        SearchServer(const std::vector<std::string> input_docs, Telemetry::Timer timer)
        {
            fillOutDocumentBase(input_docs);
            timer.check();
        };

        void fillOutDocumentBase(const std::vector<std::string>); // метод переименован из updateDocumentBase в fillOutDocumentBase
        // вопрос обновления после первичного заполнения в ТЗ подробно не рассматривается, поэтому функционал ограничен только заполнением словаря, повторный вызов метода приводит к очищению словаря и его новому заполнению

        std::vector<std::vector<Servers::RelativeIndex>> search(const std::vector<std::string>, Telemetry::Timer);

    protected:
        std::map<std::string, std::vector<Entry>> freq_dictionary; // частотный словарь
    };
}