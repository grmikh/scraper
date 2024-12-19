#include "Parser.h"
#include "boost/date_time/posix_time/posix_time.hpp"

auto parseSlots(std::string_view html) -> std::set<boost::posix_time::ptime>
{
    std::stringstream stringStream(html.data());
    std::set<boost::posix_time::ptime> slots;
    std::string line;
    std::vector<std::string> wordVector;
    std::size_t prev = 0, pos;
    while(std::getline(stringStream, line))
    {
        while ((pos = line.find("class=\"bookable\"", prev)) != std::string::npos)
        {
            auto bgn = line.find("value=\"", pos);
            wordVector.push_back(line.substr(bgn + 7, 24));
            prev = pos + 1;
        }
    }
    std::transform( wordVector.begin(), wordVector.end(),
                    std::inserter( slots, slots.begin() ),
                    [](const auto& s){
                        const auto date = boost::gregorian::from_simple_string(s.substr(8, 10));
                        boost::posix_time::time_duration td{ std::stol(s.substr(19, 2)),0, 0, 0 };
                        return boost::posix_time::ptime(date, td);
    } );
    return slots;
}
