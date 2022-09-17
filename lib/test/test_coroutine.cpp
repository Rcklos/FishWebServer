/*================================================================*
        Copyright (C) 2021 All rights reserved, www.lentme.cn.
      	文件名称：test_coroutine.cc
      	创 建 者：fish
      	创建日期：2022年9月18日
 *================================================================*/

#include "coroutine.h"

void fun() {
    while(true) {
        FISH_LOGDEBUG("before Yield fun_co");
        fish::Coroutine::Yield();
        FISH_LOGDEBUG("after Yield fun_co");
    }
}

void test_resume() {
    auto co = fish::Coroutine::Create(fun);
    for (int i = 0; i < 1000 * 10000; ++i) {
        FISH_LOGDEBUG("before Resume fun_co");
        co->Resume();
        FISH_LOGDEBUG("after Resume fun_co");
    }
}

void OnTestShareStackMem(int co_id) {
    while(true) {
        char buff[128];
        FISH_LOGDEBUG("co_id = " << co_id << ", buff ptr = " << (size_t)buff);
        fish::Coroutine::Yield();
    }
}

void test_share_stack_mem() {
    fish::Coroutine::StackMem share_mem;
    auto co1 = fish::Coroutine::Create(std::bind(OnTestShareStackMem, 1), &share_mem);
    auto co2 = fish::Coroutine::Create(std::bind(OnTestShareStackMem, 2), &share_mem);
    while(true) {
        co1->Resume();
        co2->Resume();
    }
}

int main() {
    test_share_stack_mem();
    return 0;
}
