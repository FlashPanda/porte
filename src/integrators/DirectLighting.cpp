#include <integrators/DirectLighting.h>

namespace porte
{
	DirectLightingIntegrator::DirectLightingIntegrator(LightStrategy strategy, int32 maxDepth) :
		mStrategy(strategy), mMaxDepth(maxDepth)
	{
	}

	DirectLightingIntegrator::~DirectLightingIntegrator() {}

	void DirectLightingIntegrator::Render(const Scene* pScene)
	{

	}

	Vector3f DirectLightingIntegrator::Li(const Ray& ray, const Scene& scene,
		Sampler& sampler, int32 depth) const
	{
		return Vector3f(0.f);
	}

	DirectLightingIntegrator* CreateDirectLightingIntegrator(
		LightStrategy strategy, int32 maxDepth,
		std::shared_ptr<Sampler> sampler,
		std::shared_ptr<SceneNodeCamera> camera)
	{
		DirectLightingIntegrator* pIt = new DirectLightingIntegrator(strategy, maxDepth);
		pIt->SetSampler(sampler);
		pIt->SetCamera(camera);

		return pIt;
	}
}