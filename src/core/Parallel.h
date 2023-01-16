#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_PARALLEL_H
#define PORTE_CORE_PARALLEL_H

#include <core/porte.h>
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
}

#endif