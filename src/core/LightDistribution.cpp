#include <core/LightDistribution.h>
#include <core/Scene.h>
#include <core/Integrator.h>
#include <numeric>

namespace porte
{
	LightDistribution::~LightDistribution() {}

	std::unique_ptr<LightDistribution> CreateLightSampleDistribution(const std::string& name, const Scene& scene)
	{
		if (name == "uniform" || scene.mLights.size() == 1)
			return std::unique_ptr<LightDistribution>{new UniformLightDistribution(scene) };
		else
			LOG(ERROR) << "No Light Distribution is valid!";

		return std::unique_ptr<LightDistribution>{new UniformLightDistribution(scene) };
	}

	UniformLightDistribution::UniformLightDistribution(const Scene& scene)
	{
		std::vector<Float> prob(scene.mLights.size(), Float(1));
		distrib.reset(new Distribution1D(&prob[0], int(prob.size())));
	}

	const Distribution1D* UniformLightDistribution::Lookup(const Point3f& p) const
	{
		return distrib.get();
	}
}