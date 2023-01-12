#pragma once
#include <core/Light.h>

namespace porte
{
	class PointLight : public Light
	{
	public:
		PointLight(const Matrix4f& LocalToWorld, const Vector3f& I);
		Vector3f Sample_Li(const Interaction& ref, const Vector2f& u, Vector3f* wi,
			float* pdf, VisibilityTester* vis) const;
		Vector3f Power() const;
		float Pdf_Li(const Interaction&, const Vector3f&) const;
		Vector3f Sample_Le(const Vector2f& u1, const Vector2f& u2, float time, Ray* ray, Vector3f* nLight,
			float* pdfPos, float* pdfDir) const;
		void Pdf_Le(const Ray&, const Vector3f&, float* pdfPos, float* pdfDir) const;

	private:
		const Vector3f pLight;		// Œª÷√
		const Vector3f I;	// ¡¡∂»
	};

	std::shared_ptr<PointLight> CreatePointLight(const Matrix4f& local2world, const Vector3f& color, float intensity);
}