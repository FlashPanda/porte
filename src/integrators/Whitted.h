#pragma once
#include <core/Integrator.h>
#include <memory>

namespace porte
{
	class Camara;
	class Sampler;

	class WhittedIntegrator : public SamplerIntegrator
	{
	public:
		// WhittedIntegrator Public Methods
		WhittedIntegrator()
			: SamplerIntegrator() {}
		Vector3f Li(const Ray& ray, const Scene& scene,
			Sampler& sampler, int32 depth = 0) const;
	};

	WhittedIntegrator* CreateWhittedIntegrator(std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera);
}