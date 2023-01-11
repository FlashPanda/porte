#pragma once
#include <core/MathUtil.h>
#include <functional>

/************************************************************************/
/* ������                                                                     */
/************************************************************************/

namespace porte
{
	class Scene;
	class Sampler;
	class SceneNodeCamera;
	class Ray;
	class Integrator
	{
		public:
			virtual ~Integrator();
			virtual void Render(const Scene* pScene) = 0;
	};

	class SamplerIntegrator : public Integrator
	{
		public:
			SamplerIntegrator();
			virtual ~SamplerIntegrator();
			virtual void Render(const Scene* pScene);

			void SetSampler(std::shared_ptr<Sampler> sampler)
			{
				mSampler = sampler;
			}

			void SetCamera(std::shared_ptr<SceneNodeCamera> camera)
			{
				mCamera = camera;
			}

			virtual Vector3f Li(const Ray& ray, const Scene& scene,
				Sampler& sampler, int32 depth = 0) const = 0;

			void RenderTile(std::function<void(Vector2Di)> func, Vector2Di tile);

	protected:
		std::shared_ptr<SceneNodeCamera> mCamera;

	private:
		std::shared_ptr<Sampler> mSampler;
	};

	class PathIntegrator : public SamplerIntegrator
	{
		public:
			PathIntegrator();
			virtual ~PathIntegrator();
			virtual void Render(const Scene* pScene);


			virtual Vector3f Li(const Ray& ray, const Scene& scene,
				Sampler& sampler, int32 depth = 0) const;
	};


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
