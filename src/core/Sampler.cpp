#include "Sampler.h"

namespace panda
{
	Sampler::~Sampler() {}

	Sampler::Sampler(int32 samplesPerPixel) : mSamplePerPixel(samplesPerPixel) {}

	void Sampler::StartPixel(const Vector2Di& p)
	{
		mCurrentPixel = p;
		mCurrentPixelSampleIndex = 0;
	}

	CameraSample Sampler::GetCameraSample(const Vector2Di& pRaster)
	{
		CameraSample cs;
		Vector2Df vt;
		vt[0] = pRaster[0];
		vt[1] = pRaster[1];
		cs.pFilm = vt + Get2D();
		return cs;
	}

	bool Sampler::StartNextSample()
	{
		return ++mCurrentPixelSampleIndex < mSamplePerPixel;
	}

	float RandomSampler::Get1D()
	{
		return rng.UniformFloat();
	}

	Vector2Df RandomSampler::Get2D()
	{
		return Vector2Df({rng.UniformFloat(), rng.UniformFloat()});
	}
}