#include <tgbot/tgbot.h>
#include <set>
#include "boost/date_time/local_time/local_time.hpp"
#include "mylittlehttpclient.h"
#include "messagestore.h"
#include "scheduler.h"

#include <nlohmann/json.hpp>

void run(const TgBot::Bot &bot);

using json = nlohmann::json;

std::vector<std::string> getDateRange() {
    std::vector<std::string> days;
    auto today = boost::gregorian::day_clock::local_day();
    for (int i = 0; i < 8; ++i) {
        auto end = today + boost::gregorian::date_duration(i);
        days.push_back(to_iso_extended_string(end));
    }
    return days;
}

void CourtQueryTask(MessageStore &store, const TgBot::Api &api, const string &chat_id) {
    const auto days = getDateRange();
    for (const auto &day: days) {
        try {
            auto msg = store.getMessage(day);
            if (!msg.empty()) {
                [[maybe_unused]] auto sent = api.sendMessage(chat_id, msg);
            } else {
                cout << "No updates" << endl;
            }
        } catch (std::exception &e) {
            cout << e.what() << endl;
        }
    }
}

void run(const std::string &bot_token, const std::string &chat_id) {
    TgBot::Bot bot(bot_token);
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi, I will be sending you updates on court availability!");
    });
    printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());

    Scheduler sch;

    MessageStore store;
    sch.ScheduleEvery(std::chrono::seconds(600), [&] { CourtQueryTask(store, bot.getApi(), chat_id); });
    getchar();
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Bot token, booking platform URL, chat id and config file must be provided" << std::endl;
        return 1;
    }
    std::string token{argv[1]};
    std::string url{argv[2]};
    std::string chatid{argv[3]};
    std::string config_file{argv[4]};
    CourtContext::initContext(config_file);
    try {
        run(token, chatid);
    } catch (TgBot::TgException &e) {
        printf("error: %s\n", e.what());
        sleep(60 * 10); // Ugly but gateway detects too frequent requests from the same client and throws errors
        printf("restarting");
        run(token, chatid);
    }

    return 0;
}

