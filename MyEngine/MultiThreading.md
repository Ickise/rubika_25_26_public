# Multi-Threading

The purpose of this lesson is to discover multi-threading, its benefits, its risks and upgrading our entire game engine to use that system.

1. [Threshold 0](#threshold-0)
2. [Threshold 1](#threshold-1)
3. [Threshold 2](#threshold-2)
4. [Threshold 3](#threshold-3)
5. [Threshold 4](#threshold-4)
6. [Threshold 5](#threshold-5)


## Threshold 0

Lesson in class.
- Mono thread / sequential
- Multi threading
    * Concurrency
    * Synchronisation
    * Dead Lock
    * Spin lock
- Callback
    * Function pointer
    * Lambda
- Architecture to our game engine

## Threshold 1

The purpose of this threshold is to use useful tools to help us visualizing our threads and debug them.

### Step 0

The first tool that we are going to use is [PIX](https://devblogs.microsoft.com/pix/download/). The entire project is already configured to work with PIX so, you just need to download it.

### Step 1

Let's simulate some computation during the `Update` phase of the frame and during the `Draw` phase. To do that, just add a simple line in the right place:
```cpp
    Sleep(2000);
```

Then follow those guideline to capture execution information through PIX:

1. Run your game engine
2. On pix, select the executable to attach through the `Select Target Process` window and the `Attach` section.
3. Once you have found the correct executable, click on `Attach` (uncheck the GPU Capture button)
4. PIX is now attached to the executable, you can start a capture by clicking on the `Start Timing Capture` button.
5. After few seconds, stop the capture by clicking on the same button.

Here is the online [documentation](https://devblogs.microsoft.com/pix/timing-captures-new/) about PIX if you want to know more

The different events shown in PIX capture are sent through `PROFILER_EVENT_BEGIN` and `PROFILER_EVENT_MACRO` (as well as their color and text). Feel free to add new events in your code so you can actually see them in your capture.

### Step 2

The second tool we are going to use is... your debugger! Indeed, the debugger can explore each thread that your program has started, break into any of them...

To see what your threads are doing, pause the execution (by pausing the all process or hitting a breakpoint). Then, open the Debug menu > Windows > Parallel Stacks

![img](https://learn.microsoft.com/en-us/visualstudio/debugger/media/vs-2022/debug-multithreaded-parallel-stacks-threads-view-3.png?view=vs-2017&viewFallbackFrom=visualstudio)

> here is the [documentation](https://www.jetbrains.com/help/rider/Parallel_Stacks.html) about how to open parallel stack in Rider. 

These two tools will help us a lot while implementing and debugging our system, so USE them and ask if you need anything.

## Threshold 2

The purpose of this threshold is to create a `TaskMgr` which will execute every tasks that others systems need to be executed.

### Step 0

Let's start simple and complexify this new manager along steps. Create a new manager in the `Globals` structure and call the right function at the right time, such as :

```cpp
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

private:
    ePhase CurrentPhase
};
```

As always, this is a proposition of implementation that we will follow during this class. If you need to modify it because of following steps/thresholds, needs or personnal choices, feel free to do it.

### Step 1

Our `TaskMgr` must handle two kinds of task:
- Worker tasks that are background jobs that do not need synchronization with the frame. They might be very long tasks that takes several frames to be executed.
- Sync tasks that must be executed alongside the right frame phase such as `Update` or `Draw`.

Let's start with Workers first because it is much simpler and will allow us to figure out plenty of issues without worrying about synchronization.

To execute a task on a thread, we simply need to create a `std::thread` variable and giving the function to executed through its constructor. The simplest implementation (and **wrong**) would be to create a new variable for each new worker we need. That would lead to some issues:
- Threads are not unlimited as you saw in class, so at some points, creating new threads would be useless
- If all available threads are use to execute workers tasks, there would be no thread left for executing sync tasks which will lead to very long frames, freeze... We need balance between worker tasks and sync tasks.

Let's create a new constant variable `const int WorkerCount = 4;` and an array of `std::thread` to run our tasks. Create a new method `void WorkerThreadUpdate();` that will be executed by each worker thread (we will implement it in the next steps).

### Step 2

The `WorkerThreadUpdate` function will loop infinitely (unless the engine is closed) and check if a new task must be executed. If so, it will execute it and gets back to its previous state by searching for new task to execute.

This behavior works well, very well but leads to a **spin lock** (high consumption of CPU) which is a very bad thing to do. To solve that problem, we will use a event base approach through `std::condition_variable`. Every worker thread will wait for this variable. One a new job has been registered, we will wake a worker so they can execute it.

Create a new `std::condition_variable` member dedicated to worker threads and well as a `std::queue` which will contain every task that must be executed on workers.

The `RegisterTask` is very simple. If a new task is meant to be executed on Worker, add it to the queue and `notify` a waiting thread (we could notify all of them, but that would be useless).

### Step 3

We want our `TaskMgr` to be callable from every where (every thread), it must be thread safe. As we saw in class, it is dangerous to use a variable (especially containers...) on different threads at the same time. We will need to protect thoses "shared" variables. To do so, we need to use `std::mutex` to protect our queue.

The use of a mutex is pretty straight forward. A `lock` and a `unlock`, the segment between those two functions is "protected", which means that only one protected segment can be executed at a time. If multiple threads try to execute a protected segment at the same time, one will lock and the others one will wait until the `unlock` function is called. Because of that, we can create **dead lock**, a situation where several threads are waiting infinitely because of a wrong use of mutex. To reduce the risk of forgetting to call the `unlock` function I advice you to use guard such as `std::unique_lock<std::mutex> guard(mutex);`.

Add a member `std::mutex WorkerQueueMutex` to protect the worker queue and modify the `RegisterTask` to use that mutex when it is needed (for every read and write). Don't forget to use that mutex everytime you need to use the worker queue.

### Step 4

We now have a clean way to register jobs to execute from any thread. It is now time to execute them.

Let's implement the `WorkerLoop` funcion:
1. Make an infinite loop

2. Use the `std::condition_variable` you have created earlier to wait for notification.
- the wait function need a mutex to works so, you will need to add another mutex linked to this variable to work
- It also need a function a function that returns true when it is time to stop waiting (which will be called on notificiation reception). We want our thread to wake up when the application is close and if a job must be executed. We could check the queue size (and not forgetting to use the linked mutex) which works well but we will use something else.

Create a `std::atomic<int> WorkerActiveTasks` variable that will count the number of active tasks (running and queued). Increment it on register and decrement it after execution.

The `atonic` type is a synchronization type (equivalent to a mutex and an int) which does everything automatically, so you don't need to bother to us it. Keep it mind that an `atomic` is heavier than the regular type, so use it only when it is necessary and use an atomic for base type and mutex for complex type.

3. On thread wake up, check if the app must be close. In this case returns.

4. Dequeue a job from the worker queue (**don't forget the `mutex`).

5. Execute it

6. Decrement the counter that you have added on point 2

### Step 5 

We have almost finished with workers. The last thing to do is to handle the shut part.

On shut (`ExitApp` boolean is true), so we need to notify every worker thread to wake up and to wait for their completion. To do so, use the `condition_variable` and the `join` function of a thread.

### Step 6

Every thing is in place to run tasks asynchronosly. Let's test it. In your main function, add this simple code
```cpp
	for (int i = 0; i < 100; ++i)
	{
		gData.TaskMgr->RegisterTask([i]()
			{
				PROFILER_EVENT_BEGIN(PROFILER_COLOR_DARK_BLUE, "Task %d", i);

				Sleep(2000);

				PROFILER_EVENT_END();
			},
			TaskMgr::ePhae::Worker);
	}
```

Run it with PIX attached and you see the execution of those jobs alongside the main execution. Modify the `WorkerCount` variable to see the evolution of your system.

## Treshold 3

Let's get back to our `TaskMgr` and to the second half of it : the synchronized tasks. We need to be able to program tasks that will be executed during a specific part of the frame.

### Step 0

The implementation of this feature is very similar to what we did until know. Reproduce what we did during [Threshold 2](#threshold-2) for sync tasks :
- dedicated threads with an specific loop function
- Condition Variables
- Queues (for update and draw)
- Mutex
- Atomics
- Shut

### Step 1

The "sync tasks loop' function will be a bit different than the `WorkerLoop`. Indeed, we need to check which atomics to look at and which queue to dequeue depending on the program's `CurrentPhase`.

### Step 2

The `StartPhase` function is very simple to implement because it does two simple things:
- Set the `CurrentPhase` variable to the right one.
- Wake up every "sync task" threads so they can work

### Step 3

It is time to deal with the `WaitPhase` function which, at the end, will use things we have already seen and use.

We could make a loop in the `WaitPhase` function, waiting for the number of active tasks to reach 0, but that would lead to a **spin lock**. Hopefully, we have seen a way to convert that behavior to an event based behavior. 

Do the same thing as we did before to notify *something* that a sync job has been executed.

### Step 4

Everything is in place to test our system. Modify the `main` function to use `StartPhase` et `WaitPhase` at the right moment.

Then, at the begining of each loop, call those two functions to populate our queues

```cpp
void PopulateUpdate()
{
	for (int i = 0; i < 10; ++i)
	{
		gData.TaskMgr->RegisterTask([i]()
			{
				PROFILER_EVENT_BEGIN(PROFILER_COLOR_DARK_BLUE, "Update %d", i);

				Sleep(100);

				PROFILER_EVENT_END();
			},
			TaskMgr_Clean::ePhase::Update);
	}
}

void PopulateDraw()
{
	for (int i = 0; i < 20; ++i)
	{
		gData.TaskMgr->RegisterTask([i]()
			{
				PROFILER_EVENT_BEGIN(PROFILER_COLOR_DARK_BLUE, "Draw %d", i);

				Sleep(50);

				PROFILER_EVENT_END();
			},
			TaskMgr::ePhase::Draw);
	}
}
```

Run it with PIX attached and you see the execution of those jobs alongside the main execution. Modify the `WorkerCount` variable to see the evolution of your system.

## Threshold 4

Our system works well, let's continue to use it smartly. We have already updated a system so it can work on a thread because it caused long frame. There is another system that would benefit such a treatment : the `TextureMgr`. For now, we are loaded very few small ressources before the game loop. It would be a very good improvement to be able to load lots of textures at the same time or when we actually need them.

### Step 0

We could simply move the `LoadTexture` code inside a lambda and call it on a worker thread. But SFML has sone limitations, it is not possible to create a `sf::Texture` on a thread but it is possible to load the image, load the resources attached such as meta data files and create the `sf::Image` related. 

### Threshold 5

Let's take a break and use the system we have just developed in real condition. 

### Step 0

Last time, we have implemented procedural generation algorithm (check [here](Procedural.md) for more information). These kind of algorithms take a long time to run. For now, we ran them synchronously through an ImGui window which led to long, very long frame. Modify the part to run them on workers :
- Protect the `RandomMgr` so that it can be called from anywhere (only the instance creation and deletion must be protected). `RandomInstance` were already designed to be thread-safe
- If you have followed my architecture, everything is thread safe so the generation can be parallelized easily.
- The "update texture" part must be run on the main thread.
