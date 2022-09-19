#include "scheduler.h"

void fun() {
    while(true) {
        FISH_LOGDEBUG("fun before Yield");
        fish::Coroutine::Yield();
        FISH_LOGDEBUG("fun after Yield");
    }
}

int main() {
    fish::Scheduler sche;
    auto co = fish::Coroutine::Create(fun);
    sche.Schedule(co);
    sche.Start();
}
