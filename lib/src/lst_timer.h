#pragma once
#include "common.h"
#include <cstdint>
#include <functional>
#include <set>
#include <memory>
#include <vector>

FISH_NAMESPACE_START

class SortedTimer {
public:
    // 定义定时器
    struct Timer {
        typedef std::shared_ptr<Timer> Ptr;
        typedef uint64_t time_t;

        uint64_t expire;
        std::function<void()> cb;
        Timer *prev;
        Timer *next;

        Timer(time_t expire, const std::function<void()> cb);
        Timer(time_t expire);

        struct Cmp {
            bool operator() (const Timer::Ptr& t1, const Timer::Ptr& t2);
        };
    };


private:
    std::set<Timer::Ptr, Timer::Cmp> lst_set_;
public:
    /**
     * @brief 添加定时器
     * @param expire 过期时间
     * @param cb 回调函数
     */
    Timer::Ptr add_timer(time_t expire, const std::function<void()> cb);
    Timer::Ptr add_timer(Timer::Ptr timer);

    /*
     * @brief 获取最近的定时器
     */
    Timer::Ptr head();

    /*
     * @brief 列出已经超时的timer的回调函数
     * ps: 我们并不关心timer本身
     */
    void list_expired(std::vector<std::function<void()>> &cbs);
};

FISH_NAMESPACE_END
