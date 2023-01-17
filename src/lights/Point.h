#pragma once
#include <core/porte.h>
#include <core/Light.h>

namespace porte
{
	class PointLight : public Light
	{
	public:
		PointLight(const Transform& LocalToWorld, const Spectrum& I);
		Spectrum Sample_Li(const Interaction& ref, const Vector2f& u, Vector3f* wi,
			Float* pdf, VisibilityTester* vis) const;
		Spectrum Power() const;
		Float Pdf_Li(const Interaction&, const Vector3f&) const;
		Spectrum Sample_Le(const Point2f& u1, const Point2f& u2, float time, Ray* ray, Normal3f* nLight,
			Float* pdfPos, Float* pdfDir) const;
		void Pdf_Le(const Ray&, const Normal3f&, Float* pdfPos, Float* pdfDir) const;

	private:
		const Point3f pLight;		// Œª÷√
		const Spectrum I;
	};

	std::shared_ptr<PointLight> CreatePointLight(const Transform& local2world, const Spectrum& color);
}