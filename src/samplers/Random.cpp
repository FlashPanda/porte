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
}