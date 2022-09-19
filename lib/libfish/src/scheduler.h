#pragma once

#include <queue>
#include "coroutine.h"

FISH_NAMESPACE_START

class Scheduler {
public:
    /**
     * @brief 构造函数
     */
    Scheduler();

    /**
     * @brief 析构函数
     */
    virtual ~Scheduler();

public:
    /**
     * @brief 开始
     */
    virtual void Start();

protected:
    /**
     * @brief 主循环
     */
    void OnLoop();

    /**
     * @brief 闲置处理
     */
    virtual void OnIdle();

public:
    /**
     * @brief 获取当前调度器
     */
    static Scheduler* GetCurScheduler();

    /**
     * @brief 调度
     * @param[in] coc 协程或回调
     */
    static void Schedule(Coroutine::Ptr co);

private:
    Coroutine::Ptr                 loop_co_    = nullptr;  // 主循环协程
    Coroutine::Ptr                 idle_co_    = nullptr;  // 闲置协程
    std::queue<Coroutine::Ptr>       co_list_;
};

FISH_NAMESPACE_END
