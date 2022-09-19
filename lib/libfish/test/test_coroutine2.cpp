#include "coroutine.h"

void OnTestShareStackMem(int co_id, int *num) {
    while(true) {
        (*num)++;
        FISH_LOGDEBUG("now number is " << *num);
        fish::Coroutine::Yield();
    }
}

void schedule() {
    int num = 0;
    auto co1 = fish::Coroutine::Create(std::bind(OnTestShareStackMem, 1, &num));
    auto co2 = fish::Coroutine::Create(std::bind(OnTestShareStackMem, 2, &num));
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
