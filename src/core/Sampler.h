#pragma once

#include <core/MathUtil.h>
#include <core/RNG.h>

namespace porte
{
	class Sampler
	{
	public:
		virtual ~Sampler();
		Sampler(int32 samplesPerPixel);
		virtual void StartPixel(const Vector2Di& p);
		virtual float Get1D() = 0;
		virtual Vector2f Get2D() = 0;
		CameraSample GetCameraSample(const Vector2Di& pRaster);
		virtual bool StartNextSample();
		int32 CurrentSampleNumber() const { return mCurrentPixelSampleIndex; }
	
		const int32 mSamplePerPixel = 1;

	protected:
		Vector2Di mCurrentPixel;
		int32 mCurrentPixelSampleIndex = 0;
	};

	class RandomSampler : public Sampler {
	public:
		RandomSampler(int32 ns, int32 seed = 0) : Sampler(ns), rng(seed) {}

		virtual float Get1D();
		virtual Vector2f Get2D();

	private:
		RNG rng;
	};

}