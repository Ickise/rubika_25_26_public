#pragma once
#include <functional>
#include <map>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

class TaskMgr
{
public:
    void Init();
    void Shut();

    enum class ePhase
    {
        None,
        Worker,
        Update,
        Draw,
    };

    void RegisterTask(std::function<void()> task, ePhase phase);
    void StartPhase(ePhase phase);
    void WaitPhase();
    void WorkerThreadUpdate();

private:
    ePhase CurrentPhase = ePhase::None;

    std::vector<std::thread> threads;
    std::map<ePhase, std::vector<std::function<void()>>> tasks;

    std::queue<std::function<void()>> workerQueue;
    std::mutex WorkerQueueMutex;
    std::condition_variable workerCondition;
    std::atomic<int> WorkerActiveTasks = 0;

    const int WorkerCount = 4;
    bool running = true;
};
