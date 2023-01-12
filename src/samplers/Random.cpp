#include <samplers/Random.h>

namespace porte
{
	Float RandomSampler::Get1D()
	{
		return rng.UniformFloat();
	}

	Vector2f RandomSampler::Get2D()
	{
		return Vector2f({ rng.UniformFloat(), rng.UniformFloat() });
	}
}