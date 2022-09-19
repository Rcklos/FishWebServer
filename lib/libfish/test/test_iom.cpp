#include "common.h"
#include "iomanager.h"
#include <memory>
#include <queue>

struct Task {
    typedef std::shared_ptr<std::string> Sptr;
    typedef std::shared_ptr<Task> Ptr;
    fish::Coroutine::Ptr co;
    Sptr &str;

    Task(fish::Coroutine::Ptr co, Sptr &str): co(co), str(str) {}
};

void co_main(fish::IoManager &iom, std::queue<Task::Ptr> *lst) {
    while(true) {
        std::string ans;
        std::cin >> ans;
        Task::Sptr s = std::make_shared<std::string>(ans);
        Task::Ptr p = lst->front();
        lst->pop();
        p->str = s;
        p->co->Resume();
    }
}

void co_handle(std::queue<Task::Ptr> *lst, int co_id, Task::Sptr str) {
    while(true) {
        while(str == nullptr) {
            lst->push(std::make_shared<Task>(fish::Coroutine::GetCurCoroutine(), str));
            fish::Coroutine::Yield();
        }
        FISH_LOGDEBUG("您输入的是: " << *str);
        str = nullptr;
    }
}

int main() {
    fish::IoManager iom;
    std::queue<Task::Ptr> co_lst;
    for(int i = 0; i < 3; i++) {
        auto co = fish::Coroutine::Create(std::bind(co_handle, &co_lst, i + 1, nullptr));
        iom.Schedule(co);
    }
    auto m_co = fish::Coroutine::Create(std::bind(co_main, iom, &co_lst));
    iom.Schedule(m_co);
    iom.Start();
}
