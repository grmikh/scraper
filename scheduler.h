#ifndef SCRAPER_SCHEDULER_H
#define SCRAPER_SCHEDULER_H

#include <queue>
#include <thread>
#include <functional>

using namespace std;

struct function_timer
{
    std::function<void()> func;
    std::chrono::system_clock::time_point time;

    function_timer()
    { }

    function_timer(std::function<void()>&& f, std::chrono::system_clock::time_point& t)
            : func(f),
              time(t)
    { }

    //Note: we want our priority_queue to be ordered in terms of
    //smallest time to largest, hence the > in operator<. This isn't good
    //practice - it should be a separate struct -  but I've done this for brevity.
    bool operator<(const function_timer& rhs) const
    {
        return time > rhs.time;
    }

    void get()
    {
        func();
    }
};

class Scheduler
{
private:
    std::priority_queue<function_timer> tasks;
    std::unique_ptr<std::thread> thread;
    bool go_on;

    Scheduler& operator=(const Scheduler& rhs) = delete;
    Scheduler(const Scheduler& rhs) = delete;

public:

    Scheduler()
            :go_on(true),
             thread(new std::thread([this]() { ThreadLoop(); }))
    { }

    ~Scheduler()
    {
        go_on = false;
        thread->join();
    }

    void ThreadLoop()
    {
        while(go_on)
        {
            auto now = std::chrono::system_clock::now();
            while(!tasks.empty() && tasks.top().time <= now) {
                auto f = tasks.top();
                f.get();
                tasks.pop();
            }

            if(tasks.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else {
                std::this_thread::sleep_for(tasks.top().time - std::chrono::system_clock::now());
            }
        }
    }

    void ScheduleAt(std::chrono::system_clock::time_point&& time, std::function<void()>&& func)
    {
        tasks.emplace(std::move(func), time);
    }

    void ScheduleEvery(std::chrono::system_clock::duration interval, std::function<void()> func)
    {
        std::function<void()> waitFunc = [this,interval,func]()
        {
            func();
            this->ScheduleEvery(interval, func);
        };
        ScheduleAt(std::chrono::system_clock::now() + interval, std::move(waitFunc));
    }
};

#endif //SCRAPER_SCHEDULER_H
