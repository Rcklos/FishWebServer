#pragma once
#include "common.h"
#include <cstdint>
#include <functional>
#include <set>
#include <memory>
#include <vector>

FISH_NAMESPACE_START

class TimerManager;

class Timer: public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    // 定时器智能指针
    typedef std::shared_ptr<Timer> Ptr;

    /**
     * @brief 取消定时
     */
    bool cancel();

    /**
     * @brief 刷新时间
     */
    bool refresh();

    /**
     * @brief 重置定时器时间
     * @param[in] expire 定时器执行间隔时间(毫秒)
     * @param[in] from_now 是否从当前时间开始计算
     */
    bool reset(uint64_t expire, bool from_now);
private:
    /**
     * @brief 构造函数
     * @param[in] expire 定时器执行间隔时间
     * @param[in] cb 回调函数
     * @param[in] recurring 是否循环
     * @param[in] manager 定时器管理器
     */
    Timer(uint64_t expire, std::function<void()> cb,
          bool recurring, TimerManager* manager);
    /**
     * @brief 构造函数
     * @param[in] next 执行的时间戳(毫秒)
     */
    Timer(uint64_t next);
private:
    /// 是否循环定时器
    bool m_recurring = false;
    /// 执行周期
    uint64_t m_expire = 0;
    /// 精确的执行时间
    uint64_t m_next = 0;
    /// 回调函数
    std::function<void()> m_cb;
    /// 定时器管理器
    TimerManager* m_manager = nullptr;
private:
    /**
     * @brief 定时器比较仿函数
     */
    struct Comparator {
        /**
         * @brief 比较定时器的智能指针的大小(按执行时间排序)
         * @param[in] lhs 定时器智能指针
         * @param[in] rhs 定时器智能指针
         */
        bool operator()(const Timer::Ptr& lhs, const Timer::Ptr& rhs) const;
    };
};

/**
 * @brief 定时器管理器
 */
class TimerManager {
friend class Timer;
public:
    /**
     * @brief 构造函数
     */
    TimerManager();

    /**
     * @brief 析构函数
     */
    virtual ~TimerManager();

    /**
     * @brief 添加定时器
     * @param[in] expire 定时器执行间隔时间
     * @param[in] cb 定时器回调函数
     * @param[in] recurring 是否循环定时器
     */
    Timer::Ptr addTimer(uint64_t expire, std::function<void()> cb, bool recurring = false);

    /**
     * @brief 添加条件定时器
     * @param[in] expire 定时器执行间隔时间
     * @param[in] cb 定时器回调函数
     * @param[in] weak_cond 条件
     * @param[in] recurring 是否循环
     */
    Timer::Ptr addConditionTimer(uint64_t expire, std::function<void()> cb ,std::weak_ptr<void> weak_cond ,bool recurring = false);

    /**
     * @brief 到最近一个定时器执行的时间间隔(毫秒)
     */
    uint64_t getNextTimer();

    /**
     * @brief 获取需要执行的定时器的回调函数列表
     * @param[out] cbs 回调函数数组
     */
    void listExpiredCb(std::vector<std::function<void()> >& cbs);

    /**
     * @brief 是否有定时器
     */
    bool hasTimer();
protected:

    /**
     * @brief 当有新的定时器插入到定时器的首部,执行该函数
     */
    virtual void onTimerInsertedAtFront() = 0;

    /**
     * @brief 将定时器添加到管理器中
     */
    void addTimer(Timer::Ptr val);
private:
    /**
     * @brief 检测服务器时间是否被调后了
     */
    bool detectClockRollover(uint64_t now_expire);
private:
    /// 定时器集合
    std::set<Timer::Ptr, Timer::Comparator> m_timers;
    /// 是否触发onTimerInsertedAtFront
    bool m_tickled = false;
    /// 上次执行时间
    uint64_t m_previouseTime = 0;
};

//
// class SortedTimer {
// public:
//     // 定义定时器
//     struct Timer {
//         typedef std::shared_Ptr<Timer> Ptr;
//         typedef uint64_t time_t;
//
//         uint64_t expire;
//         std::function<void()> cb;
//         Timer *prev;
//         Timer *next;
//
//         Timer(time_t expire, const std::function<void()> cb);
//         Timer(time_t expire);
//
//         struct Cmp {
//             bool operator() (const Timer::Ptr& t1, const Timer::Ptr& t2);
//         };
//     };
//
//
// private:
//     std::set<Timer::Ptr, Timer::Cmp> lst_set_;
// public:
//     /**
//      * @brief 添加定时器
//      * @param expire 过期时间
//      * @param cb 回调函数
//      */
//     Timer::Ptr add_timer(time_t expire, const std::function<void()> cb);
//     Timer::Ptr add_timer(Timer::Ptr timer);
//
//     /*
//      * @brief 获取最近的定时器
//      */
//     Timer::Ptr head();
//
//     /*
//      * @brief 列出已经超时的timer的回调函数
//      * ps: 我们并不关心timer本身
//      */
//     void list_expired(std::vector<std::function<void()>> &cbs);
// };
//
FISH_NAMESPACE_END
