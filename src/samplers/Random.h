#pragma once
#include <core/porte.h>
#include <core/Sampler.h>
#include <core/Geometry.h>

namespace porte
{
	class RandomSampler : public Sampler {
	public:
		RandomSampler(int32_t ns, int32_t seed = 0) : Sampler(ns), rng(seed) {}

		virtual float Get1D();
		virtual Point2f Get2D();

	private:
		RNG rng;
	};
}