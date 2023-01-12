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
		virtual void StartPixel(const Vector2i& p);
		virtual float Get1D() = 0;
		virtual Vector2f Get2D() = 0;
		CameraSample GetCameraSample(const Vector2i& pRaster);
		virtual bool StartNextSample();
		int32 CurrentSampleNumber() const { return mCurrentPixelSampleIndex; }
	
		const int32 mSamplePerPixel = 1;

	protected:
		Vector2i mCurrentPixel;
		int32 mCurrentPixelSampleIndex = 0;
	};
}