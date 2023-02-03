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

			// �ҵ���һ��·���ϵĶ��㣬�ۻ�����

			// ��¼���ߺͳ����ཻ��λ��
			SurfaceInteraction isect;
			bool foundIntersection = scene.Intersect(ray, &isect);

			// ����λ���Ƿ�Ҫ��ӷ���ֵ
			if (bounces == 0 || specularBounce)
			{
				// ��·�����㴦��ӷ���ֵ�����Ǵӻ����л�ȡ
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

			// ���û�ҵ����㣬���ǳ��������ȣ��ͽ���
			if (!foundIntersection || bounces >= maxDepth) break;

			// �����ǽ��ʵı߽練��
			isect.ComputeScatteringFunctions(ray, arene, true);
			if (!isect.bsdf)
			{
				VLOG(2) << "Skipping intersection due to null bsdf";
				ray = isect.SpawnRay(ray.d);
				bounces--;	// ������һ��bounce
				continue;
			}

			const Distribution1D* distrib = lightDistribution->Lookup(isect.p);

			// ������Դ������ֵ������·������ֵ
			// �൱��һ��ֱ�ӹ���
			// ����Ҫ�������淴��
			if (isect.bsdf->NumComponents(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0)
			{
				// ����һ��ֱ�ӹ���
				Spectrum Ld = beta * UniformSampleOneLight(isect, scene, arene, sampler, false, distrib);
				VLOG(2) << "Sampled direct lighting Ld = " << Ld;
				L += Ld;
			}

			// ����BSDF�����һ���µ�·��
			Vector3f wo = -ray.d, wi;
			Float pdf;
			BxDFType flags;
			Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf,
				BSDF_ALL, &flags);
			VLOG(2) << "Sampled BSDF, f = " << f << ", pdf = " << pdf;
			if (f.IsBlack() || pdf == 0.f) break;
			beta *= f * AbsDot(wi, isect.shading.n) / pdf;
			// ���ݼ���
			VLOG(2) << "Updated beta = " << beta;
			CHECK_GE(beta.y(), 0.f);
			DCHECK(!std::isinf(beta.y()));
			specularBounce = (flags & BSDF_SPECULAR) != 0;
			// TODO ��������뿪���ʵ�etaScale����
			ray = isect.SpawnRay(wi);

			// TODO: �α���ɢ��

			// �ö���˹�����ս�·��
			Spectrum rrBeta = beta * etaScale;	// ��������Щ���࣬���ǿ��ǵ����ܻ���refraction�Ĳ��֡�
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