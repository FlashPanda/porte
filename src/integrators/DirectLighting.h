#pragma once

#include <core/Integrator.h>

namespace porte
{
	enum class LightStrategy
	{
		// �������еĹ�Դ�����ݹ�Դ��nSamples���ݲ�һ���������������������͡�
		UniformSampleAll,

		// ���ѡ��һ����Դ����һ��������
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