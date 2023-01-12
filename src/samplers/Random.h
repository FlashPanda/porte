#pragma once
#include <core/Sampler.h>

namespace porte
{
	class RandomSampler : public Sampler {
	public:
		RandomSampler(int32 ns, int32 seed = 0) : Sampler(ns), rng(seed) {}

		virtual float Get1D();
		virtual Vector2f Get2D();

	private:
		RNG rng;
	};
}