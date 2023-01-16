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

		// ���㽻��ķ�����뷴���

		// ��ʼ��ͨ������
		const Vector3f n = isect.n;	// ����
		Vector3f wo = isect.wo;

		// ���㽻��ı���ɢ�亯��
		isect.ComputeScatteringFunctions(ray);
		// �������
		//if (!isect.bsdf)
		//	return Li(isect.SpawnRay(ray.d), scene, sampler, depth);

		// ���㽻��ķ����
		L += isect.Le(wo);

		// ����ÿ����Դ������ֱ�ӹ���
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

		// ׷�ٹ⻬����ķ��������
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