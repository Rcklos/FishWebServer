#include "lst_timer.h"
#include "util/ex_util.hpp"
FISH_NAMESPACE_START

SortedTimer::Timer::
Timer(SortedTimer::Timer::time_t expire, const std::function<void()> cb)
    : expire(expire), cb(cb), prev(nullptr), next(nullptr) {}

SortedTimer::Timer::
Timer(SortedTimer::Timer::time_t expire): expire(expire) {}

bool
SortedTimer::Timer::Cmp::operator()(const Timer::Ptr &t1, const Timer::Ptr &t2) {
    if(t1 && t2) return t1->expire < t2->expire || (t1.get() < t2.get() && t1->expire == t2->expire);
    return (!t1);
}

SortedTimer::Timer::Ptr
SortedTimer::
add_timer(SortedTimer::Timer::Ptr timer) {
    auto it = lst_set_.insert(timer).first;
    bool insert_at_front = it == lst_set_.begin();
    return timer;
}

SortedTimer::Timer::Ptr
SortedTimer::
add_timer(time_t expire, const std::function<void()> cb) {
    SortedTimer::Timer::Ptr timer(new SortedTimer::Timer(expire, cb));
    return add_timer(timer);
}

SortedTimer::Timer::Ptr
SortedTimer::
head() { return lst_set_.empty()? nullptr: *lst_set_.begin(); }

void
SortedTimer::
list_expired(std::vector<std::function<void ()>> &cbs) {
    time_t now_time = TimeStampMs(); // 精确到毫秒
    if(lst_set_.empty()) return;
    Timer::Ptr timer(new Timer(now_time));
    std::vector<Timer::Ptr> expired_timers;
    auto end = lst_set_.upper_bound(timer);
    for(auto it = lst_set_.begin(); it != end; it++)
        expired_timers.push_back(*it);

    cbs.reserve(expired_timers.size());
    for(auto& it: expired_timers) {
        cbs.push_back(it->cb);
    }
}

FISH_NAMESPACE_END
