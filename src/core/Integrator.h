#pragma once
#include <core/MathUtil.h>
#include <functional>

/************************************************************************/
/* »ý·ÖÆ÷                                                                     */
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

			virtual Vector3f Li(const Ray& ray, const Scene& scene,
				Sampler& sampler, int32 depth = 0) const = 0;

			void RenderTile(std::function<void(Vector2i)> func, Vector2i tile);

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
}
