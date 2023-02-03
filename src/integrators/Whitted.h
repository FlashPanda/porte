#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_INTEGRATORS_WHITTED_H
#define PORTE_INTEGRATORS_WHITTED_H

#include <core/porte.h>
#include <core/Integrator.h>
#include <memory>

namespace porte
{
	class WhittedIntegrator : public SamplerIntegrator
	{
	public:
		WhittedIntegrator(std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i& pixelBounds)
			: SamplerIntegrator(camera, sampler, pixelBounds) {}
		Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena,
			int depth = 0) const;
	};

	WhittedIntegrator* CreateWhittedIntegrator(std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera);
}
#endif