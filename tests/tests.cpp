#include "gtest/gtest.h"
#include "servers.h"

using Converter = Converters::ConverterJSON;
using SearchServer = Servers::SearchServer;
using Timer = Telemetry::Timer;

const char* ENGINE_VERSION = "0.4.2"; // current Haystack Search Engine version

const std::map<std::string, std::vector<Servers::Entry>> MODEL_DICTIONARY {
    {"G", {{0, 1}, {1, 5}, {2, 996}}}, {"M", {{2, 1}}},
    {"a", {{1, 1}}}, {"b", {{1, 1}}}, {"bread", {{1, 4}, {2, 1}}}, {"c", {{1, 1}}}, {"coffee", {{0, 1}, {2, 1}}},
    {"d", {{1, 1}}}, {"milk", {{0, 2}, {1, 4}, {2, 1}}}, {"salt", {{0, 1}}}, {"sugar", {{0, 1}}}
};

const std::vector<std::vector<Servers::RelativeIndex>> MODEL_SEARCH_RESULT {
    {{2, 1.0}, {0, 1.0}}, {{2, 1.0}, {1, 0.013}}, {{2, 1.0}, {1, 0.005}, {0, 0.001}},
    {{2, 1.0}}, {{1, 1.0}, {0, 0.5}, {2, 0.25}}, {}, {{2, 1.0}, {0, 0.002}}
};

//

struct TestServer : public SearchServer
{
    TestServer(const std::vector<std::string> input_docs) : SearchServer(input_docs, Timer("Search server construction and initial dictionary fill")) {}
    
    std::map<std::string, std::vector<Servers::Entry>> getFreqDictionary() const
    {
        return freq_dictionary;
    }
};

class TestFixture : public ::testing::Test
{
protected:
    
    Converter *conv;
    TestServer *ts;
    std::vector<std::vector<Servers::RelativeIndex>> searchResult;

    void SetUp()
    {
       conv = new Converter(ENGINE_VERSION);
       ts = new TestServer(conv->getTextDocuments());
       searchResult = ts->search(conv->getRequests(), Timer("Search process"));
    }
    
    void TearDown()
    {
        delete ts;
        delete conv;
    }
};

//

TEST_F(TestFixture, Frequency_dictionary_test)
{
    ASSERT_EQ(ts->getFreqDictionary(), MODEL_DICTIONARY) << std::endl << "Error comment: Frequency dictionaries aren't equal";
}

TEST_F(TestFixture, Search_test)
{
    ASSERT_EQ(searchResult, MODEL_SEARCH_RESULT) << std::endl << "Error comment: Search results aren't equal";
}

//

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
