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

	// һ���Ե�barrier������Ϊ��ȷ�������̶߳�����ͬһ���ص�
	// ��shared_ptrά����������֤�������̶߳������˲Ż��ͷ��������
	class Barrier
	{
	public:
		Barrier(int count) : count(count) { CHECK_GT(count, 0); }
		~Barrier() { CHECK_EQ(count, 0); }
		void Wait();

	private:
		std::mutex mutex;

		/*
		* std::condition_variable����C++11�ṩ������������������ͬʱ����һ���̻߳����̡߳�
		  һ��ģ��������߳�����֧��std::mutex��std::lock_guard/std::unique_lock�޸Ĺ��������
		  ֪ͨcondition_variable���������̻߳�ȡͬһ��std::mutex(std::unique_lock������)��
		  ������std::condition_variable��wait, wait_for, or wait_until��
		  wait�������ͷŻ�������ͬʱ������̡߳������������յ�֪ͨ����ʱ���ڻ�����ٻ���ʱ��
		  �̱߳����ѣ�������Ҳ��ԭ�ӵ����»�ȡ����Ҫע����ǣ��������ٻ��ѣ�
		  �߳�Ӧ�ü�������������ȴ����Ա�֤ҵ�����ȷ�ԡ�
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