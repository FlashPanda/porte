#include <samplers/Random.h>

namespace porte
{
	Float RandomSampler::Get1D()
	{
		return rng.UniformFloat();
	}

	Point2f RandomSampler::Get2D()
	{
		return Point2f(rng.UniformFloat(), rng.UniformFloat());
	}

	std::unique_ptr<Sampler> RandomSampler::Clone(int seed)
	{
		RandomSampler* rs = new RandomSampler(*this);
		rs->rng.SetSequence(seed);
		return std::unique_ptr<Sampler>(rs);
	}
}