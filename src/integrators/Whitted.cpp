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

		// ׷�ٹ⻬����ķ��������
		// TODO

		return L;
	}
}