#include "TaskMgr.h"

void TaskMgr::Init()
{
    for (int i = 0; i < WorkerCount; i++)
    {
        threads.emplace_back(&TaskMgr::WorkerThreadUpdate, this);
    }
}

void TaskMgr::Shut()
{
    {
        std::unique_lock<std::mutex> guard(WorkerQueueMutex);
        running = false;
    }

    workerCondition.notify_all();

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
    else
    {
        tasks[phase].push_back(task);
    }
}

void TaskMgr::StartPhase(ePhase phase)
{
    CurrentPhase = phase;

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
                return WorkerActiveTasks > 0 || !running;
            });

            if (!running)
                return;

            task = workerQueue.front();
            workerQueue.pop();
        }

        task();

        --WorkerActiveTasks;
    }
}
