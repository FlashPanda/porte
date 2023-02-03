#include <integrators/Path.h>
#include <core/Camera.h>
#include <core/Film.h>
#include <core/Interaction.h>
#include <core/Scene.h>
#include <core/Reflection.h>
#include <core/Sampler.h>

namespace porte
{
	PathIntegrator::PathIntegrator(int maxDepth,
		std::shared_ptr<const Camera> camera,
		std::shared_ptr<Sampler> sampler,
		const Bounds2i& pixelBounds, Float rrThreshold,
		const std::string& lightSampleStrategy)
		: SamplerIntegrator(camera, sampler, pixelBounds),
		maxDepth(maxDepth),
		rrThreshold(rrThreshold),
		lightSampleStrategy(lightSampleStrategy)
	{
	}

	void PathIntegrator::Preprocess(const Scene& scene, Sampler& sampler)
	{
		lightDistribution = CreateLightSampleDistribution(lightSampleStrategy, scene);
	}

	Spectrum PathIntegrator::Li(const RayDifferential& r, const Scene& scene,
		Sampler& sampler, MemoryArena& arene, int depth) const
	{
		Spectrum L(0.f), beta(1.f);
		RayDifferential ray(r);
		bool specularBounce = false;
		int bounces;

		Float etaScale = 1.f;	// ?
		for (bounces = 0;; ++bounces)
		{
			VLOG(2) << "Path tracer bounce " << bounces << ", current L = " << L
				<< ", beta = " << beta;

			// 找到下一个路径上的顶点，累积贡献

			// 记录光线和场景相交的位置
			SurfaceInteraction isect;
			bool foundIntersection = scene.Intersect(ray, &isect);

			// 交点位置是否要添加发光值
			if (bounces == 0 || specularBounce)
			{
				// 在路径顶点处添加发光值，或是从环境中获取
				if (foundIntersection)
				{
					L += beta * isect.Le(-ray.d);
					VLOG(2) << "Added Le -> L = " << L;
				}
				else
				{
					// TODO: infinity lights
				}
			}

			// 如果没找到交点，或是超过最大深度，就结束
			if (!foundIntersection || bounces >= maxDepth) break;

			// 不考虑介质的边界反射
			isect.ComputeScatteringFunctions(ray, arene, true);
			if (!isect.bsdf)
			{
				VLOG(2) << "Skipping intersection due to null bsdf";
				ray = isect.SpawnRay(ray.d);
				bounces--;	// 不算作一次bounce
				continue;
			}

			const Distribution1D* distrib = lightDistribution->Lookup(isect.p);

			// 采样光源的照明值，计算路径贡献值
			// 相当于一次直接光照
			// 但是要跳过镜面反射
			if (isect.bsdf->NumComponents(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0)
			{
				// 进行一波直接光照
				Spectrum Ld = beta * UniformSampleOneLight(isect, scene, arene, sampler, false, distrib);
				VLOG(2) << "Sampled direct lighting Ld = " << Ld;
				L += Ld;
			}

			// 采样BSDF，获得一个新的路径
			Vector3f wo = -ray.d, wi;
			Float pdf;
			BxDFType flags;
			Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf,
				BSDF_ALL, &flags);
			VLOG(2) << "Sampled BSDF, f = " << f << ", pdf = " << pdf;
			if (f.IsBlack() || pdf == 0.f) break;
			beta *= f * AbsDot(wi, isect.shading.n) / pdf;
			// 数据检验
			VLOG(2) << "Updated beta = " << beta;
			CHECK_GE(beta.y(), 0.f);
			DCHECK(!std::isinf(beta.y()));
			specularBounce = (flags & BSDF_SPECULAR) != 0;
			// TODO 进入或是离开介质的etaScale更新
			ray = isect.SpawnRay(wi);

			// TODO: 次表面散射

			// 用俄罗斯轮盘终结路径
			Spectrum rrBeta = beta * etaScale;	// 现在是有些多余，但是考虑到可能会有refraction的部分。
			if (rrBeta.MaxComponentValue() < rrThreshold && bounces > 3) {
				Float q = std::max((Float).05, 1 - rrBeta.MaxComponentValue());
				if (sampler.Get1D() < q) break;
				beta /= 1 - q;

			}

		}

		return L;
	}

	PathIntegrator* CreatePathIntegrator(std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera)
	{
		int maxDepth = 5;
		Bounds2i pixelBounds = camera->film->GetSampleBounds();
		Float rrThreshold = 1.f;
		std::string lightStrategy("uniform");

		return new PathIntegrator(maxDepth, camera, sampler, pixelBounds, rrThreshold, lightStrategy);
	}
}