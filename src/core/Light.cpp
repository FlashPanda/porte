#include "Light.h"
#include "Interaction.h"
#include "Sampling.h"
#include "Scene.h"
#include "Transform.h"

namespace porte
{
	Vector3f VisibilityTester::Tr(const Scene& scene, Sampler& sampler) const {
		return Vector3f(0.f);
	}

	bool VisibilityTester::Unoccluded(const Scene& scene) const
	{
		return !scene.IntersectP(p0.SpawnRayTo(p1));
	}

	Light::Light(int32 flags, const Matrix4f& LocalToWorld, int32 nSamples)
		: flags(flags),
		nSamples(std::max(1, nSamples)),
		LocalToWorld(LocalToWorld),
		WorldToLocal(InverseMatrix(LocalToWorld))
	{}

	Light::~Light()
	{

	}

	Vector3f Light::Le(const Ray& ray) const { return Vector3f(0.f); }

	PointLight::PointLight(const Matrix4f& LocalToWorld, const Vector3f& I)
		: Light((int32)LightFlags::DeltaPosition, LocalToWorld),
		pLight(TransformPoint(Vector3f(0.f), LocalToWorld)),
		I(I)
	{
	}

	Vector3f PointLight::Sample_Li(const Interaction& ref, const Vector2f& u, Vector3f* wi,
		float* pdf, VisibilityTester* vis) const
	{
		// 点光源的sample很简单，方向，pdf值都简单
		*wi = Normalize(pLight - ref.p);
		*pdf = 1.f;
		*vis = VisibilityTester(ref, Interaction(pLight));
		return I / GetLengthSquare(pLight - ref.p);
	}

	Vector3f PointLight::Power() const { return 4 * PI * I; }

	float PointLight::Pdf_Li(const Interaction&, const Vector3f&) const
	{
		return 0;
	}

	Vector3f PointLight::Sample_Le(const Vector2f& u1, const Vector2f& u2, float time, Ray* ray, Vector3f* nLight, float* pdfPos, float* pdfDir) const
	{
		*ray = Ray(pLight, UniformSampleSphere(u1));
		*nLight = ray->d;
		*pdfPos = 1;
		*pdfDir = UniformSpherePdf();
		return I;
	}

	void PointLight::Pdf_Le(const Ray&, const Vector3f&, float* pdfPos, float* pdfDir) const
	{
		*pdfPos = 0;
		*pdfDir = UniformSpherePdf();
	}

	std::shared_ptr<PointLight> CreatePointLight(const Matrix4f& local2world, const Vector3f& color,
		float intensity)
	{
		Vector3f I = color * intensity;
		return std::make_shared<PointLight>(local2world, I);
	}
}