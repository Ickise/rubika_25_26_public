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
        Sync,
    };

    void RegisterTask(std::function<void()> task, ePhase phase);
    void StartPhase(ePhase phase);
    void WaitPhase();
    void SyncThreadUpdate();
    void WorkerThreadUpdate();

private:
    ePhase CurrentPhase = ePhase::None;

    std::vector<std::thread> threads;
    std::map<ePhase, std::vector<std::function<void()>>> tasks;

    std::queue<std::function<void()>> workerQueue;
    std::queue<std::function<void()>> syncQueue;

    std::mutex WorkerQueueMutex;
    std::mutex SyncQueueMutex;

    std::condition_variable workerCondition;
    std::condition_variable syncCondition;

    std::atomic<int> WorkerActiveTasks = 0;
    std::atomic<int> SyncActiveTasks = 0;

    const int WorkerCount = 4;
    const int SyncCount = 2;
    bool running = true;
};
