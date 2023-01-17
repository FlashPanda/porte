#include <lights/Point.h>
#include <core/Sampling.h>

namespace porte
{
	PointLight::PointLight(const Transform& LocalToWorld, const Spectrum& I)
		: Light((int)LightFlags::DeltaPosition, LocalToWorld),
		pLight(LocalToWorld(Point3f(0.f, 0.f, 0.f))),
		I(I)
	{
	}

	Vector3f PointLight::Sample_Li(const Interaction& ref, const Vector2f& u, Vector3f* wi,
		float* pdf, VisibilityTester* vis) const
	{
		// 点光源的sample很简单，方向，pdf值都简单
		*wi = drjit::normalize(pLight - ref.p);
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

	std::shared_ptr<PointLight> CreatePointLight(const Transform& local2world, const Vector3f& color,
		float intensity)
	{
		Vector3f I = color * intensity;
		return std::make_shared<PointLight>(local2world, I);
	}
}