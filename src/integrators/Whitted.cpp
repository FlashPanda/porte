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

	//	// ���㽻��ķ�����뷴���

	//	// ��ʼ��ͨ������
	//	const Normal3f n = isect.n;	// ����
	//	Vector3f wo = isect.wo;

	//	// ���㽻��ı���ɢ�亯��
	//	isect.ComputeScatteringFunctions(ray, arena);
	//	// �������
	//	//if (!isect.bsdf)
	//	//	return Li(isect.SpawnRay(ray.d), scene, sampler, depth);

	//	// ���㽻��ķ����
	//	L += isect.Le(wo);

	//	// ����ÿ����Դ������ֱ�ӹ���
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

	//	// ׷�ٹ⻬����ķ��������
	//	// TODO

	//	return L;
	//}

	Spectrum WhittedIntegrator::Li(const RayDifferential& ray, const Scene& scene,
		Sampler& sampler, MemoryArena& arena,
		int depth) const 
	{
		Spectrum L(0.f);

		// �ҵ���������߽��㣬���߷��ر���radiance
		SurfaceInteraction isect;
		if (!scene.Intersect(ray, &isect))
		{
			for (const auto& light : scene.mLights)
				L += light->Le(ray);
			return L;
		}

		// ����ķ�����߷����

		const Normal3f& n = isect.shading.n;
		Vector3f wo = isect.wo;

		// ����ɢ�亯��
		isect.ComputeScatteringFunctions(ray, arena);
		if (!isect.bsdf)
			//return Li(isect.SpawnRay(ray.d), scene, sampler, arena, depth);
			return L;

		L += isect.Le(wo);

		// ����ÿ����Դ�Ĺ���
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