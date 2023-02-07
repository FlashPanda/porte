#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_PARALLEL_H
#define PORTE_CORE_PARALLEL_H

#include <core/porte.h>
#include <core/Geometry.h>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace porte
{
	class AtomicFloat {
	public:
		explicit AtomicFloat(Float v = 0) { bits = FloatToBits(v); }
		operator Float() const { return BitsToFloat(bits); }
		Float operator=(Float v) {
			bits = FloatToBits(v);
			return v;
		}
		void Add(Float v) {
#ifdef PORTE_FLOAT_AS_DOUBLE
			uint64_t oldBits = bits, newBits;
#else
			uint32_t oldBits = bits, newBits;
#endif
			do {
				newBits = FloatToBits(BitsToFloat(oldBits) + v);
			} while (!bits.compare_exchange_weak(oldBits, newBits));
		}

	private:
#ifdef PORTE_FLOAT_AS_DOUBLE
		std::atomic<uint64_t> bits;
#else
		std::atomic<uint32_t> bits;
#endif
	};

	// 一次性的barrier，就是为了确保所有线程都到了同一个地点
	// 用shared_ptr维护，这样保证了所有线程都结束了才会释放这个东西
	class Barrier
	{
	public:
		Barrier(int count) : count(count) { CHECK_GT(count, 0); }
		~Barrier() { CHECK_EQ(count, 0); }
		void Wait();

	private:
		std::mutex mutex;

		/*
		* std::condition_variable，是C++11提供的条件变量，可用于同时阻塞一个线程或多个线程。
		  一般的，生产者线程利用支持std::mutex的std::lock_guard/std::unique_lock修改共享变量后，
		  通知condition_variable。消费者线程获取同一个std::mutex(std::unique_lock所持有)，
		  并调用std::condition_variable的wait, wait_for, or wait_until。
		  wait操作会释放互斥量，同时挂起该线程。当条件变量收到通知、超时到期或发生虚假唤醒时，
		  线程被唤醒，互斥量也被原子地重新获取。需要注意的是，如果是虚假唤醒，
		  线程应该检查条件并继续等待，以保证业务的正确性。
		*/
		std::condition_variable cv;
		int count;
	};

	void ParallelFor(std::function<void(int64_t)> func, int64_t count,
		int chunkSize = 1);
	extern thread_local int ThreadIndex;
	void ParallelFor2D(std::function<void(Point2i)> func, const Point2i& count);
	int MaxThreadIndex();
	int NumSystemCores();

	void ParallelInit();
	void ParallelCleanup();
	void MergeWorkerThreadStats();
}

#endif