#include <integrators/Whitted.h>

namespace porte
{
	Vector3f WhittedIntegrator::Li(const Ray& ray, const Scene& scene,
		Sampler& sampler, int32 depth) const
	{
		Vector3f L(0.f);

		SurfaceInteraction isect;
		if (!scene.Intersect(ray, &isect))
		{
			for (auto iter : scene.mPbrtLights)
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
		for (const auto& iter : scene.mPbrtLights)
		{
			Vector3f wi;
			float pdf;
			VisibilityTester visibility;
			Vector3f Li =
				iter->Sample_Li(isect, sampler.Get2D(), &wi, &pdf, &visibility);

			if (Li.IsBlack() || pdf == 0) continue;

			Vector3f f = isect.bsdf->f(wo, wi);
			if (!f.IsBlack() && visibility.Unoccluded(scene))
				L += f * Li * AbsDotProduct(wi, n) / pdf;
		}

		// 追踪光滑表面的反射和折射
		// TODO

		return L;
	}
}