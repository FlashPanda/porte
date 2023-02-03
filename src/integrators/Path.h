#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_INTEGRATORS_PATH_H
#define PORTE_INTEGRATORS_PATH_H

#include <core/porte.h>
#include <core/Integrator.h>
#include <core/LightDistribution.h>

namespace porte
{
	class PathIntegrator : public SamplerIntegrator
	{
	public:
		PathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i& pixelBounds, Float rrThreshold = 1,
			const std::string& lightSampleStrategy = "spatial");

		void Preprocess(const Scene& scene, Sampler& sampler);
		
		Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena, int depth) const;

	private:
		const int maxDepth;
		const Float rrThreshold;
		const std::string lightSampleStrategy;
		std::unique_ptr<LightDistribution> lightDistribution;
	};

	PathIntegrator* CreatePathIntegrator(std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera);
}

#endif