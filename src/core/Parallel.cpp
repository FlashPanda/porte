
#include <core/Parallel.h>
#include <core/Memory.h>
#include <list>
#include <thread>
#include <condition_variable>

namespace porte
{
	static std::vector<std::thread> threads;
	static bool shutdownThreads = false;
	class ParallelForLoop;
	static ParallelForLoop* workList = nullptr;
	static std::mutex workListMutex;

	// 用来记账的变量，帮助MergeWorkerThreadStats的实现
	static std::atomic<bool> reportWorkerStats{ false };
	// 需要报告状态的worker数量
	static std::atomic<int> reporterCount;
	// 在督促worker报告自身自身状态之后，主线程在条件变量上等待，直到所有worker都完成报告
	static std::condition_variable reportDoneCondition;
	static std::mutex reportDoneMutex;

	class ParallelForLoop
	{
	public:
		ParallelForLoop(std::function<void(int64_t)> func1D, int64_t maxIndex,
			int chunkSize, uint64_t profilerState)
			: func1D(std::move(func1D)),
			maxIndex(maxIndex),
			chunkSize(chunkSize),
			profilerState(profilerState)
		{

		}

		ParallelForLoop(const std::function<void(Point2i)>& f, const Point2i& count,
			uint64_t profilerState)
			: func2D(f),
			maxIndex(count.x * count.y),
			chunkSize(1),
			profilerState(profilerState)
		{
			nX = count.x;
		}

	public:
		std::function<void(int64_t)> func1D;
		std::function<void(Point2i)> func2D;
		const int64_t maxIndex;
		const int chunkSize;
		uint64_t profilerState;
		int64_t nextIndex = 0;
		int activeWorkers = 0;
		ParallelForLoop* next = nullptr;
		int nX = -1;

		bool Finished() const
		{
			return nextIndex >= maxIndex && activeWorkers == 0;
		}
	};

	void Barrier::Wait()
	{
		/*
		std::unique_lock为锁管理模板类，是对通用mutex的封装。std::unique_lock对象以独占所有权的方式(unique owership)管理mutex对象的上锁和解锁操作，即在unique_lock对象的声明周期内，它所管理的锁对象会一直保持上锁状态；而unique_lock的生命周期结束之后，它所管理的锁对象会被解锁。
		*/
		std::unique_lock<std::mutex> lock(mutex);
		if (--count == 0)
			// 最后的线程了，通知其他所有线程都起床
			cv.notify_all();
		else
			cv.wait(lock, [this] {return count == 0; });
	}

	static std::condition_variable workListCondition;

	static void workerThreadFunc(int tIndex, std::shared_ptr<Barrier> barrier)
	{
		LOG(INFO) << "Started execution in worker thread " << tIndex;
		ThreadIndex = tIndex;

		// 主线程设置一个barrier
		barrier->Wait();

		barrier.reset();

		std::unique_lock<std::mutex> lock(workListMutex);
		while (!shutdownThreads)
		{
			if (reportWorkerStats) {
				workListCondition.wait(lock);
			}
			else if (!workList)
			{
				workListCondition.wait(lock);
			}
			else {
				ParallelForLoop& loop = *workList;

				// 运行一段代码

				// 找到下一个要迭代的块
				int64_t indexStart = loop.nextIndex;
				int64_t indexEnd =
					std::min(indexStart + loop.chunkSize, loop.maxIndex);

				loop.nextIndex = indexEnd;
				if (loop.nextIndex == loop.maxIndex) workList = loop.next;
				loop.activeWorkers++;

				lock.unlock();
				for (int64_t index = indexStart; index < indexEnd; ++index) {
					if (loop.func1D) {
						loop.func1D(index);
					}
					else {
						CHECK(loop.func2D);
						loop.func2D(Point2i(index % loop.nX, index / loop.nX));
					}
				}
				lock.lock();

				loop.activeWorkers--;
				if (loop.Finished()) workListCondition.notify_all();
			}
		}
	}

	void ParallelFor(std::function<void(int64_t)> func, int64_t count,
		int chunkSize) {
		CHECK(threads.size() > 0 || MaxThreadIndex() == 1);

		if (threads.empty() || count < chunkSize) {
			for (int64_t i = 0; i < count; ++i) func(i);
			return;
		}

		ParallelForLoop loop(std::move(func), count, chunkSize,
			0);
		workListMutex.lock();
		loop.next = workList;
		workList = &loop;
		workListMutex.unlock();

		std::unique_lock<std::mutex> lock(workListMutex);
		workListCondition.notify_all();

		while (!loop.Finished()) {
			int64_t indexStart = loop.nextIndex;
			int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);

			loop.nextIndex = indexEnd;
			if (loop.nextIndex == loop.maxIndex) workList = loop.next;
			loop.activeWorkers++;

			lock.unlock();
			for (int64_t index = indexStart; index < indexEnd; ++index) {
				if (loop.func1D) {
					loop.func1D(index);
				}
				else {
					CHECK(loop.func2D);
					loop.func2D(Point2i(index % loop.nX, index / loop.nX));
				}
			}
			lock.lock();

			loop.activeWorkers--;
		}
	}

	thread_local int ThreadIndex;

	int MaxThreadIndex() {
		return NumSystemCores();
	}

	void ParallelFor2D(std::function<void(Point2i)> func, const Point2i& count) {
		CHECK(threads.size() > 0 || MaxThreadIndex() == 1);

		if (threads.empty() || count.x * count.y <= 1) {
			for (int y = 0; y < count.y; ++y)
				for (int x = 0; x < count.x; ++x) func(Point2i(x, y));
			return;
		}

		ParallelForLoop loop(std::move(func), count, 0);
		{
			std::lock_guard<std::mutex> lock(workListMutex);
			loop.next = workList;
			workList = &loop;
		}

		std::unique_lock<std::mutex> lock(workListMutex);
		workListCondition.notify_all();

		while (!loop.Finished()) {
			int64_t indexStart = loop.nextIndex;
			int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);

			loop.nextIndex = indexEnd;
			if (loop.nextIndex == loop.maxIndex) workList = loop.next;
			loop.activeWorkers++;

			lock.unlock();
			for (int64_t index = indexStart; index < indexEnd; ++index) {
				if (loop.func1D) {
					loop.func1D(index);
				}
				else {
					CHECK(loop.func2D);
					loop.func2D(Point2i(index % loop.nX, index / loop.nX));
				}
			}
			lock.lock();

			loop.activeWorkers--;
		}
	}

	int NumSystemCores() {
		return std::max(1u, std::thread::hardware_concurrency());
	}

	void ParallelInit() {
		CHECK_EQ(threads.size(), 0);
		int nThreads = MaxThreadIndex();
		ThreadIndex = 0;

		std::shared_ptr<Barrier> barrier = std::make_shared<Barrier>(nThreads);

		for (int i = 0; i < nThreads - 1; ++i)
			threads.push_back(std::thread(workerThreadFunc, i + 1, barrier));

		barrier->Wait();
	}

	void ParallelCleanup() {
		if (threads.empty()) return;

		{
			std::lock_guard<std::mutex> lock(workListMutex);
			shutdownThreads = true;
			workListCondition.notify_all();
		}

		for (std::thread& thread : threads) thread.join();
		threads.erase(threads.begin(), threads.end());
		shutdownThreads = false;
	}

	void MergeWorkerThreadStats() {
		std::unique_lock<std::mutex> lock(workListMutex);
		std::unique_lock<std::mutex> doneLock(reportDoneMutex);
		reportWorkerStats = true;
		reporterCount = threads.size();

		workListCondition.notify_all();

		reportDoneCondition.wait(lock, []() { return reporterCount == 0; });

		reportWorkerStats = false;
	}
}