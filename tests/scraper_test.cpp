#include <gtest/gtest.h>
#include <fstream>
#include "../query_executor.h"

// Demonstrate some basic assertions.
TEST(ScraperTest, BasicAssertions) {
    std::ifstream file{"data.json"};
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto res = parseSlots(buffer.str());
    EXPECT_EQ(5, res.size());;
}

TEST(ScraperTest, GetData) {
    auto resp = getSlotsFiltered(std::set<std::string>{"BethnalGreenGardens", "KingEdwardMemorialPark",
                                                                                 "WappingGardens"});
    for (const auto& item : resp)
    {
        std::cout << item.first << " " <<  item.second << std::endl;
    }
}