#include "Sampler.h"

namespace porte
{
	Sampler::~Sampler() {}

	Sampler::Sampler(int32 samplesPerPixel) : mSamplePerPixel(samplesPerPixel) {}

	void Sampler::StartPixel(const Vector2i& p)
	{
		mCurrentPixel = p;
		mCurrentPixelSampleIndex = 0;
	}

	CameraSample Sampler::GetCameraSample(const Vector2i& pRaster)
	{
		CameraSample cs;
		Vector2f vt;
		vt[0] = pRaster[0];
		vt[1] = pRaster[1];
		cs.pFilm = vt + Get2D();
		return cs;
	}

	bool Sampler::StartNextSample()
	{
		return ++mCurrentPixelSampleIndex < mSamplePerPixel;
	}

}