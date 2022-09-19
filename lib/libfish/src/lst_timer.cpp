#include "lst_timer.h"
#include "util/ex_util.hpp"
FISH_NAMESPACE_START

bool Timer::Comparator::operator()(const Timer::Ptr& lhs
                        ,const Timer::Ptr& rhs) const {
    if(!lhs && !rhs) {
        return false;
    }
    if(!lhs) {
        return true;
    }
    if(!rhs) {
        return false;
    }
    if(lhs->m_next < rhs->m_next) {
        return true;
    }
    if(rhs->m_next < lhs->m_next) {
        return false;
    }
    return lhs.get() < rhs.get();
}


Timer::Timer(uint64_t expire, std::function<void()> cb,
             bool recurring, TimerManager* manager)
    :m_recurring(recurring)
    ,m_expire(expire)
    ,m_cb(cb)
    ,m_manager(manager) {
    m_next = TimeStampMs() + m_expire;
}

Timer::Timer(uint64_t next)
    :m_next(next) {
}

bool Timer::cancel() {
    if(m_cb) {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh() {
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    m_next = TimeStampMs() + m_expire;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t expire, bool from_now) {
    if(expire == m_expire && !from_now) {
        return true;
    }
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if(from_now) {
        start = TimeStampMs();
    } else {
        start = m_next - m_expire;
    }
    m_expire = expire;
    m_next = start + m_expire;
    m_manager->addTimer(shared_from_this());
    return true;

}

TimerManager::TimerManager() {
    m_previouseTime = TimeStampMs();
}

TimerManager::~TimerManager() {
}

Timer::Ptr TimerManager::addTimer(uint64_t expire, std::function<void()> cb
                                  ,bool recurring) {
    Timer::Ptr timer(new Timer(expire, cb, recurring, this));
    addTimer(timer);
    return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();
    if(tmp) {
        cb();
    }
}

Timer::Ptr TimerManager::addConditionTimer(uint64_t expire, std::function<void()> cb
                                    ,std::weak_ptr<void> weak_cond
                                    ,bool recurring) {
    return addTimer(expire, std::bind(&OnTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
    m_tickled = false;
    if(m_timers.empty()) {
        return ~0ull;
    }

    const Timer::Ptr& next = *m_timers.begin();
    uint64_t now_expire = TimeStampMs();
    if(now_expire >= next->m_next) {
        return 0;
    } else {
        return next->m_next - now_expire;
    }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs) {
    uint64_t now_expire = TimeStampMs();
    std::vector<Timer::Ptr> expired;
    {
        if(m_timers.empty()) {
            return;
        }
    }
    if(m_timers.empty()) {
        return;
    }
    bool rollover = detectClockRollover(now_expire);
    if(!rollover && ((*m_timers.begin())->m_next > now_expire)) {
        return;
    }

    Timer::Ptr now_timer(new Timer(now_expire));
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
    while(it != m_timers.end() && (*it)->m_next == now_expire) {
        ++it;
    }
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());

    for(auto& timer : expired) {
        cbs.push_back(timer->m_cb);
        if(timer->m_recurring) {
            timer->m_next = now_expire + timer->m_expire;
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;
        }
    }
}

void TimerManager::addTimer(Timer::Ptr val) {
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if(at_front) {
        m_tickled = true;
    }

    // if(at_front) {
    //     onTimerInsertedAtFront();
    // }
}

bool TimerManager::detectClockRollover(uint64_t now_expire) {
    bool rollover = false;
    if(now_expire < m_previouseTime &&
            now_expire < (m_previouseTime - 60 * 60 * 1000)) {
        rollover = true;
    }
    m_previouseTime = now_expire;
    return rollover;
}

bool TimerManager::hasTimer() {
    return !m_timers.empty();
}

FISH_NAMESPACE_END
