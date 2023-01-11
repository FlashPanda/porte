#pragma once

#include "MathUtil.h"
#include <algorithm>

namespace porte
{
	static const double DoubleOneMinusEpsilon = 0x1.fffffffffffffp-1;
	static const float FloatOneMinusEpsilon = 0x1.fffffep-1;

	static const float OneMinusEpsilon = FloatOneMinusEpsilon;

	#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
	#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
	#define PCG32_MULT 0x5851f42d4c957f2dULL

	// random number generator
	// 随机数生成器
	class RNG
	{
	public:
		RNG();
		RNG(uint64 sequenceIndex) { SetSequence(sequenceIndex); }
		void SetSequence(uint64 sequenceIndex);
		uint32 UniformUInt32();
		uint32 UniformUInt32(uint32 b)
		{
			uint32 threshold = (~b + 1u) % b;
			while (true)
			{
				uint32 r = UniformUInt32();
				if (r >= threshold) return r % b;
			}
		}

		float UniformFloat()
		{
			return std::min(OneMinusEpsilon, float(UniformUInt32() * 0x1p-32f));
		}

		template <typename Iterator>
		void Shuffle(Iterator begin, Iterator end)
		{
			for (Iterator it = end - 1; it > begin; --it)
				std::iter_swap(it, begin + UniformUInt32((uint32_t)(it - begin + 1)));
		}
		void Advance(int64 idelta) {
			int64 cur_mult = PCG32_MULT, cur_plus = inc, acc_mult = 1u,
				acc_plus = 0u, delta = (uint64_t)idelta;
			while (delta > 0) {
				if (delta & 1) {
					acc_mult *= cur_mult;
					acc_plus = acc_plus * cur_mult + cur_plus;
				}
				cur_plus = (cur_mult + 1) * cur_plus;
				cur_mult *= cur_mult;
				delta /= 2;
			}
			state = acc_mult * state + acc_plus;
		}
		int64 operator-(const RNG& other) const {
			uint64 cur_mult = PCG32_MULT, cur_plus = inc, cur_state = other.state,
				the_bit = 1u, distance = 0u;
			while (state != cur_state) {
				if ((state & the_bit) != (cur_state & the_bit)) {
					cur_state = cur_state * cur_mult + cur_plus;
					distance |= the_bit;
				}

				the_bit <<= 1;
				cur_plus = (cur_mult + 1ULL) * cur_plus;
				cur_mult *= cur_mult;
			}
			return (int64)distance;
		}

	private:
		uint64 state, inc;
	};

	inline RNG::RNG() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {}
	inline void RNG::SetSequence(uint64 initseq) {
		state = 0u;
		inc = (initseq << 1u) | 1u;
		UniformUInt32();
		state += PCG32_DEFAULT_STATE;
		UniformUInt32();
	}

	inline uint32 RNG::UniformUInt32() {
		uint64 oldstate = state;
		state = oldstate * PCG32_MULT + inc;
		uint32 xorshifted = (uint32)(((oldstate >> 18u) ^ oldstate) >> 27u);
		uint32 rot = (uint32)(oldstate >> 59u);
		return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
	}
}