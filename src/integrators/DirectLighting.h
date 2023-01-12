#pragma once

#include <core/Integrator.h>

namespace porte
{
	enum class LightStrategy
	{
		// 遍历所有的光源，根据光源的nSamples数据采一定数量的样本，将结果求和。
		UniformSampleAll,

		// 随机选择一个光源，采一个样本。
		UniformSampleOne
	};
	class DirectLightingIntegrator : public SamplerIntegrator
	{
	public:
		DirectLightingIntegrator(LightStrategy strategy, int32 maxDepth);
		virtual ~DirectLightingIntegrator();
		virtual void Render(const Scene* pScene);

		virtual Vector3f Li(const Ray& ray, const Scene& scene,
			Sampler& sampler, int32 depth = 0) const;

	private:
		const LightStrategy mStrategy;
		const int32 mMaxDepth;
		std::vector<int32> mNLightSamples;
	};

	DirectLightingIntegrator* CreateDirectLightingIntegrator(
		LightStrategy strategy, int32 maxDepth,
		std::shared_ptr<Sampler> sampler,
		std::shared_ptr<SceneNodeCamera> camera);
}