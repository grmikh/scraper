#pragma once
#include <map>
#include <string>
#include <vector>

#include "court_context.h"
#include "new/Parser.h"

namespace {
    auto hideInResultSet(const std::pair<std::string, boost::posix_time::ptime> &obj) -> bool {
        const auto time = obj.second;
        const auto dow = time.date().day_of_week();
        return time.time_of_day().hours() < 18 && (dow != boost::date_time::weekdays::Saturday && dow !=
                                                   boost::date_time::weekdays::Sunday || time.time_of_day().hours() <
                                                   9);
    }

    auto getSlots(const std::set<std::string> &venues,
                  const std::string &day) -> std::set<std::pair<std::string, boost::posix_time::ptime> > {
        std::set<std::pair<std::string, boost::posix_time::ptime> > res_set{};
        mylittlehttpclient cl;
        for (const auto &venue: venues) {
            std::stringstream ss;
            ss << "/book/courts/" << venue << "/" << day;
            const auto host = CourtContext::getUrl();
            const auto url = ss.str();
            std::stringstream fullurl;
            fullurl << "https://" << host << url;
            auto resp = cl.makeRequest(fullurl.str(), {});
            std::this_thread::sleep_for(std::chrono::seconds(20 + (rand() % 5)));
            for (const auto &item: parseSlots(resp)) {
                res_set.insert(make_pair(venue, item));
            }
        }
        return res_set;
    }

    auto getSlotsFiltered(const std::set<std::string> &venues,
                          const std::string &day) -> std::set<std::pair<std::string, boost::posix_time::ptime> > {
        auto res = getSlots(venues, day);
        for (auto it = res.begin(); it != res.end(); /* blank */) {
            if (hideInResultSet(*it)) {
                res.erase(it++); // Note the subtlety here
            } else {
                ++it;
            }
        }
        return res;
    }
}

struct MessageStore {
    std::map<std::pair<std::string, boost::posix_time::ptime>, boost::posix_time::ptime> _store;

    std::string getMessage(const std::string &day) {
        auto resp = getSlotsFiltered(
            CourtContext::getCourts()
            , day);
        auto found = std::find_if(resp.begin(), resp.end(), [&](auto val) {
            const std::map<std::pair<std::string, boost::posix_time::ptime>, boost::posix_time::ptime>::iterator &
                    iterator = _store.find(
                        val);
            if (iterator == _store.end())
                return true;
            if (iterator->second - boost::posix_time::second_clock::local_time() > boost::posix_time::hours(1)) {
                iterator->second = boost::posix_time::second_clock::local_time();
                return true;
            }
            iterator->second = boost::posix_time::second_clock::local_time();
            return _store.find(val) == _store.end();
        });
        std::vector filtered(found, resp.end());
        std::stringstream ss;
        for (const auto &val: filtered) {
            ss << CourtContext::getCourtsMapping().at(val.first) << ": " << val.second.date().day_of_week().as_short_string() << " " << val.second
                    << std::endl;
            _store[val] = boost::posix_time::second_clock::local_time();
        }
        return ss.str();
    }
};
