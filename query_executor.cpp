#include <iostream>
#include "boost/date_time/local_time/local_time.hpp"
//#include <boost/json/src.hpp>

//#include <boost/beast/core.hpp>
//#include <boost/beast/http.hpp>
//#include <boost/beast/ssl.hpp>
//#include <boost/beast/version.hpp>
//#include <boost/asio/connect.hpp>
//#include <boost/asio/ip/tcp.hpp>
//#include <boost/asio/ssl/error.hpp>
//#include <boost/asio/ssl/stream.hpp>
#include "query_executor.h"

//#include <curl/curl.h>
//#include <tgbot/tgbot.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

auto parseSlots(std::string_view json) -> std::set<boost::posix_time::ptime>
{
    std::set<boost::posix_time::ptime> res{};

    auto basicStringstream = std::stringstream(std::string(json));
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
//    auto parsed = boost::json::parse(json).as_object();
//    for (const auto& elem : parsed.at("Resources").as_array())
//    {
//        for (const auto& day : elem.at("Days").as_array())
//        {
//            for (const auto& session : day.at("Sessions").as_array())
//            {
//                if (session.as_object().at("Capacity").get_int64() != 0)
//                {
//                    const auto date = boost::gregorian::from_simple_string(std::string(day.at("Date").as_string().c_str()).substr(0, 10));
//                    const auto hours = session.at("StartTime").as_int64() / 60;
//                    boost::posix_time::time_duration td{ hours,0, 0, 0 };
//                    res.emplace(date, td);
//                }
//            }
//        }
//    }
    return res;
}

std::pair<std::string, std::string> getDateRange() {
    auto today = boost::gregorian::day_clock::local_day();
    auto end = today + boost::gregorian::date_duration(7);
    return std::make_pair(to_iso_extended_string(today), to_iso_extended_string(end));
}

static std::string buffer;

static int writer(char *data, size_t size, size_t nmemb,
                  std::string *writerData)
{
    if(writerData == NULL)
        return 0;

    writerData->append(data, size*nmemb);

    return size * nmemb;
}

auto getSlots(const std::set<std::string>& venues) -> std::set<std::pair<std::string, boost::posix_time::ptime>> {
    std::set<std::pair<std::string, boost::posix_time::ptime>> res_set{};
    std::string startDate, endDate;
    std::tie(startDate, endDate) = getDateRange();
//    TgBot::CurlHttpClient http_client;
    for (const auto& venue : venues)
    {
        std::stringstream ss;
        ss << "/v0/VenueBooking/" << venue << "/GetVenueSessions?resourceID=&startDate=" <<  startDate << "&endDate=" << endDate;
        const auto host = "clubspark.lta.org.uk";
        const auto url = ss.str();
//        const auto res = http_client.makeRequest(TgBot::Url(std::string(host + url)), {});
//        CURL *curl;
//        CURLcode res;
//
//        curl_global_init(CURL_GLOBAL_DEFAULT);
//        std::string buffer;
//
//        curl = curl_easy_init();
//        if(curl) {
//            curl_easy_setopt(curl, CURLOPT_URL, std::string(host + url).c_str());
//            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
//            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
//            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
//        }
//        res = curl_easy_perform(curl);
//        if(res != CURLE_OK)
//            fprintf(stderr, "curl_easy_perform() failed: %s\n",
//                    curl_easy_strerror(res));
//        curl_easy_cleanup(curl);
//        curl_global_cleanup();

//        boost::asio::io_context ioc;
//
//        // The boost::asio::ssl context is required, and holds certificates
//        boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12_client);
//
//        // This holds the root certificate used for verification
////        load_root_certificates(ctx);
//
//        // Verify the remote server's certificate
//        ctx.set_verify_mode(boost::asio::ssl::verify_none);
//
//        // These objects perform our I/O
//        boost::asio::ip::tcp::resolver resolver(ioc);
//        boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);
//
//        // Set SNI Hostname (many hosts need this to handshake successfully)
//        if(! SSL_set_tlsext_host_name(stream.native_handle(), host))
//        {
//            boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
//            throw boost::beast::system_error{ec};
//        }
//
//        // Look up the domain name
//        auto const results = resolver.resolve(host, "443");
//
//        // Make the connection on the IP address we get from a lookup
//        boost::beast::get_lowest_layer(stream).connect(results);
//
//        // Perform the boost::asio::ssl handshake
//        stream.handshake(boost::asio::ssl::stream_base::client);
//
//        // Set up an boost::beast::http GET request message
//        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, url, 11};
//        req.set(boost::beast::http::field::host, host);
//        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
//
//        // Send the boost::beast::http request to the remote host
//        boost::beast::http::write(stream, req);
//
//        // This buffer is used for reading and must be persisted
//        boost::beast::flat_buffer buffer;
//
//        // Declare a container to hold the response
//        boost::beast::http::response<boost::beast::http::dynamic_body> res;
//
//        // Receive the boost::beast::http response
//        boost::beast::http::read(stream, buffer, res);
//
//        // Write the message to standard out
////        std::cout << res << std::endl;
//        auto res_str = boost::beast::buffers_to_string(res.body().data());
//        // Gracefully close the stream
//        boost::beast::error_code ec;
//        stream.shutdown(ec);
//        for (const auto &item: parseSlots(buffer))
//        {
//            res_set.insert(make_pair(venue, item));
//        }
//        buffer = "";
    }
    return res_set;
}

auto hideInResultSet(const std::pair<std::string, boost::posix_time::ptime> & obj) -> bool
{
    const auto time = obj.second;
    const auto dow = time.date().day_of_week();
    return time.time_of_day().hours() < 18 && dow != boost::date_time::weekdays::Saturday && dow != boost::date_time::weekdays::Sunday;
}

auto getSlotsFiltered(const std::set<std::string>& venues) -> std::set<std::pair<std::string, std::string>>
{
    auto res = getSlots(venues);
    std::set<std::pair<std::string, std::string>> out;
    for ( auto it = res.begin(); it != res.end(); /* blank */ ) {
        if ( hideInResultSet(*it) ) {
            res.erase( it++ );       // Note the subtlety here
        }
        else {
            ++it;
        }
    }
    for (const auto& item : res) {
        out.insert(std::make_pair(item.first, to_iso_extended_string(item.second)));
    }
    return out;
}