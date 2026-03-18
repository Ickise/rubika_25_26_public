#include "TaskMgr.h"

void TaskMgr::Init()
{
    for (int i = 0; i < WorkerCount; i++)
        threads.emplace_back(&TaskMgr::WorkerThreadUpdate, this);

    for (int i = 0; i < SyncCount; i++)
        threads.emplace_back(&TaskMgr::SyncThreadUpdate, this);
}

void TaskMgr::Shut()
{
    {
        std::unique_lock<std::mutex> lock(WorkerQueueMutex);
        running = false;
    }

    workerCondition.notify_all();
    syncCondition.notify_all();

    for (auto& t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

void TaskMgr::RegisterTask(std::function<void()> task, ePhase phase)
{
    if (phase == ePhase::Worker)
    {
        {
            std::lock_guard<std::mutex> lock(WorkerQueueMutex);
            workerQueue.push(task);
            ++WorkerActiveTasks;
        }

        workerCondition.notify_one();
    }
    else if (phase == ePhase::Sync)
    {
        {
            std::lock_guard<std::mutex> lock(SyncQueueMutex);
            syncQueue.push(task);
            ++SyncActiveTasks;
        }

        syncCondition.notify_one();
    }
    else
    {
        tasks[phase].push_back(task);
    }
}

void TaskMgr::StartPhase(ePhase phase)
{
    CurrentPhase = phase;

    if (phase == ePhase::Sync)
    {
        syncCondition.notify_all();
        return;
    }

    std::vector<std::thread> phaseThreads;
    for (auto& task : tasks[phase])
    {
        phaseThreads.emplace_back(task);
    }

    for (auto& t : phaseThreads)
    {
        if (t.joinable())
            t.join();
    }
}

void TaskMgr::WaitPhase()
{
    std::unique_lock<std::mutex> lock(SyncQueueMutex);

    syncWaitCondition.wait(lock, [this]()
    {
        return SyncActiveTasks == 0;
    });
}

void TaskMgr::SyncThreadUpdate()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(SyncQueueMutex);

            syncCondition.wait(lock, [this]
            {
                return !syncQueue.empty() || !running;
            });

            if (!running && syncQueue.empty())
                return;

            if (CurrentPhase == ePhase::Sync && !syncQueue.empty())
            {
                task = syncQueue.front();
                syncQueue.pop();
            }
            else
            {
                continue;
            }
        }

        task();

        --SyncActiveTasks;

        syncWaitCondition.notify_one();
    }
}

void TaskMgr::WorkerThreadUpdate()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(WorkerQueueMutex);

            workerCondition.wait(lock, [this]
            {
                return !workerQueue.empty() || !running;
            });

            if (!running && workerQueue.empty())
                return;

            task = workerQueue.front();
            workerQueue.pop();
        }

        task();

        --WorkerActiveTasks;
    }
}
