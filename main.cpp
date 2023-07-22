#include <tgbot/tgbot.h>
#include <set>
#include "boost/date_time/local_time/local_time.hpp"
#include "MyHttpClient.h"

#include <nlohmann/json.hpp>

void run(const TgBot::Bot &bot);

using json = nlohmann::json;

auto parseSlots(const std::string& json) -> std::set<boost::posix_time::ptime>
{
    std::set<boost::posix_time::ptime> res{};
    auto basicStringstream = std::stringstream(json);
    auto js = json::parse(basicStringstream);
    for (const auto& elem : js["Resources"])
    {
        for(const auto& day : elem["Days"])
        {
            for (const auto& session : day["Sessions"])
            {
                if (session["Capacity"] != 0)
                {
                    const auto date = boost::gregorian::from_simple_string(std::string(day["Date"].get<std::string>()).substr(0, 10));
                    const auto hours = session.at("StartTime").get<int>() / 60;
                    boost::posix_time::time_duration td{ hours,0, 0, 0 };
                    res.emplace(date, td);
                }
            }
        }
    }
    return res;
}

std::pair<std::string, std::string> getDateRange() {
    auto today = boost::gregorian::day_clock::local_day();
    auto end = today + boost::gregorian::date_duration(7);
    return std::make_pair(to_iso_extended_string(today), to_iso_extended_string(end));
}

auto getSlots(const std::set<std::string>& venues) -> std::set<std::pair<std::string, boost::posix_time::ptime>> {
    std::set<std::pair<std::string, boost::posix_time::ptime>> res_set{};
    const auto [startDate, endDate] = getDateRange();
    MyHttpClient cl;
    for (const auto& venue : venues)
    {
        std::stringstream ss;
        ss << "/v0/VenueBooking/" << venue << "/GetVenueSessions?resourceID=&startDate=" <<  startDate << "&endDate=" << endDate;
        const auto *const host = "clubspark.lta.org.uk";
        const auto url = ss.str();
        std::stringstream fullurl;
        fullurl << "https://" << host << url;
        auto resp = cl.makeRequest(fullurl.str(), {});

        for (const auto &item: parseSlots(resp))
        {
            res_set.insert(make_pair(venue, item));
        }
    }
    return res_set;
}

auto hideInResultSet(const std::pair<std::string, boost::posix_time::ptime> & obj) -> bool
{
    const auto time = obj.second;
    const auto dow = time.date().day_of_week();
    return time.time_of_day().hours() < 18 && dow != boost::date_time::weekdays::Saturday && dow != boost::date_time::weekdays::Sunday;
}

auto getSlotsFiltered(const std::set<std::string>& venues) -> std::set<std::pair<std::string, boost::posix_time::ptime>>
{
    auto res = getSlots(venues);
    for ( auto it = res.begin(); it != res.end(); /* blank */ ) {
        if ( hideInResultSet(*it) ) {
            res.erase( it++ );       // Note the subtlety here
        }
        else {
            ++it;
        }
    }
    return res;
}

std::string getMessage() {
    static const std::unordered_map<std::string, std::string> names = {
            {"BethnalGreenGardens", "BBG"},
            {"KingEdwardMemorialPark", "KEMP"},
            {"WappingGardens", "WG"}
    };
    auto resp = getSlotsFiltered(std::set<std::string>{"BethnalGreenGardens", "KingEdwardMemorialPark",
                                                       "WappingGardens"});
    std::stringstream ss;
    for (const auto& val : resp)
    {
        ss << names.at(val.first) << ": " << val.second.date().day_of_week().as_short_string() << " " << val.second << std::endl;
    }
    return ss.str();
}


void run() {
    TgBot::Bot bot("985088905:AAFsjswEAesYlvSiiiX-dybUqXrG8QyZjc0");
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });
    bot.getEvents().onCommand("courts", [&bot](TgBot::Message::Ptr message) {
        printf("Req from %s", message->chat->username.c_str());
        auto msg = getMessage();
        if (!msg.empty()) {
            bot.getApi().sendMessage(message->chat->id, msg);
        }
    });
    printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
    TgBot::TgLongPoll longPoll(bot);
    while (true) {
        printf("Long poll started\n");
        longPoll.start();
    }
}

int main() {

    try {
        run();
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
        sleep(60*5);
        printf("restarting");
        run();
    }

    return 0;
}

