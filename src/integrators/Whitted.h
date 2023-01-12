#pragma once
#include <core/Integrator.h>

namespace porte
{
	class WhittedIntegrator : public SamplerIntegrator
	{
	public:
		// WhittedIntegrator Public Methods
		WhittedIntegrator()
			: SamplerIntegrator() {}
		Vector3f Li(const Ray& ray, const Scene& scene,
			Sampler& sampler, int32 depth = 0) const;
	};
}