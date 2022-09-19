#include "coroutine.h"

void OnTestShareStackMem(int co_id) {
    while(true) {
        char buff[128];
        FISH_LOGDEBUG("co_id = " << co_id << ", buff ptr = " << (size_t)buff);
    }
}

void schedule() {
    auto co1 = fish::Coroutine::Create(std::bind(OnTestShareStackMem, 1));
    auto co2 = fish::Coroutine::Create(std::bind(OnTestShareStackMem, 2));
    while(true) {
        co1->Resume();
        co2->Resume();
    }
}

int main (int argc, char *argv[]) {
    fish::Coroutine::StackMem share_mem;
    auto co = fish::Coroutine::Create(std::bind(schedule), &share_mem);
    co->Resume();
    return 0;
}
