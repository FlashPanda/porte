#include <integrators/Whitted.h>
#include <core/Sampler.h>
#include <core/Interaction.h>
#include <core/Light.h>
#include <core/Scene.h>
#include <core/Camera.h>

namespace porte
{
	Vector3f WhittedIntegrator::Li(const Ray& ray, const Scene& scene,
		Sampler& sampler, int32 depth) const
	{
		Vector3f L(0.f);

		SurfaceInteraction isect;
		if (!scene.Intersect(ray, &isect))
		{
			for (auto iter : scene.mLights)
			{
				L += iter->Le(ray);
			}
			return L;
		}

		// 计算交点的发射光与反射光

		// 初始化通用向量
		const Vector3f n = isect.n;	// 法线
		Vector3f wo = isect.wo;

		// 计算交点的表面散射函数
		isect.ComputeScatteringFunctions(ray);
		// 体积计算
		//if (!isect.bsdf)
		//	return Li(isect.SpawnRay(ray.d), scene, sampler, depth);

		// 计算交点的发射光
		L += isect.Le(wo);

		// 遍历每个光源，计算直接光照
		for (const auto& iter : scene.mLights)
		{
			Vector3f wi;
			float pdf;
			VisibilityTester visibility;
			Vector3f Li =
				iter->Sample_Li(isect, sampler.Get2D(), &wi, &pdf, &visibility);

			if (Li == Vector3f(0.f) || pdf == 0) continue;

			Vector3f f = isect.bsdf->f(wo, wi);
			if (!(f == Vector3f(0.f)) && visibility.Unoccluded(scene))
				L += f * Li * drjit::abs_dot(wi, n) / pdf;
				
		}

		// 追踪光滑表面的反射和折射
		// TODO

		return L;
	}

	WhittedIntegrator* CreateWhittedIntegrator(std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera)
	{
		Bounds2i pixelBounds = camera->film->GetSampleBounds();

		return new WhittedIntegrator(camera, sampler, pixelBounds);
	}
}