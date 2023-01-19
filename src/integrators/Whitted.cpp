#include <integrators/Whitted.h>
#include <core/Sampler.h>
#include <core/Interaction.h>
#include <core/Light.h>
#include <core/Scene.h>
#include <core/Camera.h>
#include <core/Reflection.h>

namespace porte
{
	//Spectrum WhittedIntegrator::Li(const Ray& ray, const Scene& scene,
	//	Sampler& sampler, MemoryArena& arena,
	//	int depth) const
	//{
	//	Spectrum L(0.f);

	//	SurfaceInteraction isect;
	//	if (!scene.Intersect(ray, &isect))
	//	{
	//		for (auto iter : scene.mLights)
	//		{
	//			L += iter->Le(ray);
	//		}
	//		return L;
	//	}

	//	// 计算交点的发射光与反射光

	//	// 初始化通用向量
	//	const Normal3f n = isect.n;	// 法线
	//	Vector3f wo = isect.wo;

	//	// 计算交点的表面散射函数
	//	isect.ComputeScatteringFunctions(ray, arena);
	//	// 体积计算
	//	//if (!isect.bsdf)
	//	//	return Li(isect.SpawnRay(ray.d), scene, sampler, depth);

	//	// 计算交点的发射光
	//	L += isect.Le(wo);

	//	// 遍历每个光源，计算直接光照
	//	for (const auto& iter : scene.mLights)
	//	{
	//		Vector3f wi;
	//		float pdf;
	//		VisibilityTester visibility;
	//		Spectrum Li =
	//			iter->Sample_Li(isect, sampler.Get2D(), &wi, &pdf, &visibility);

	//		if (Li.IsBlack() || pdf == 0) continue;

	//		Spectrum f = isect.bsdf->f(wo, wi);
	//		if (!(f.IsBlack()) && visibility.Unoccluded(scene))
	//			L += f * Li * AbsDot(wi, n) / pdf;
	//			
	//	}

	//	// 追踪光滑表面的反射和折射
	//	// TODO

	//	return L;
	//}

	Spectrum WhittedIntegrator::Li(const RayDifferential& ray, const Scene& scene,
		Sampler& sampler, MemoryArena& arena,
		int depth) const 
	{
		Spectrum L(0.f);

		// 找到最近的射线交点，或者返回背景radiance
		SurfaceInteraction isect;
		if (!scene.Intersect(ray, &isect))
		{
			for (const auto& light : scene.mLights)
				L += light->Le(ray);
			return L;
		}

		// 交点的发射或者反射光

		const Normal3f& n = isect.shading.n;
		Vector3f wo = isect.wo;

		// 计算散射函数
		isect.ComputeScatteringFunctions(ray, arena);
		if (!isect.bsdf)
			//return Li(isect.SpawnRay(ray.d), scene, sampler, arena, depth);
			return L;

		L += isect.Le(wo);

		// 增加每个光源的贡献
		for (const auto& light : scene.mLights)
		{
			Vector3f wi;
			Float pdf;
			VisibilityTester visibility;
			Spectrum Li = 
				light->Sample_Li(isect, sampler.Get2D(), &wi, &pdf, &visibility);
			if (Li.IsBlack() || pdf == 0) 
				continue;
			Spectrum f = isect.bsdf->f(wo, wi);
			if (!f.IsBlack() && visibility.Unoccluded(scene))
				L += f * Li * AbsDot(wi, n) / pdf;
		}

		

		return L;
	}

	WhittedIntegrator* CreateWhittedIntegrator(std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera)
	{
		Bounds2i pixelBounds = camera->film->GetSampleBounds();

		return new WhittedIntegrator(camera, sampler, pixelBounds);
	}
}