#include "Sampling.h"
#include "MathUtil.h"

namespace porte
{
	// 球体均匀采样
	Vector3f UniformSampleSphere(const Vector2f& u)
	{
		float z = 1 - 2 * u[0];
		float r = std::sqrt(std::max((float)0, (float)1 - z * z));
		float phi = 2 * PI * u[1];
		return Vector3f({r * std::cos(phi), r * std::sin(phi), z});
	}

	// 球体pdf
	float UniformSpherePdf()
	{
		return INV_4_PI;
	}

	// 均匀半球样本：u[0] = cos(theta), u[1] = phi / (2 * PI)
	Vector3f UniformSampleHemisphere(const Vector2f& u)
	{
		float e = std::sqrt((std::max)(0.f, 1.f - u[0] * u[0]));
		float x = std::cosf(2 * PI * u[1]) * e;
		float y = std::sinf(2 * PI * u[1]) * e;
		float z = u[0];
		return Vector3f({ x, y, z });
	}

	// 半球PDF
	float UniformHemispherePdf()
	{
		return INV_2_PI;
	}

	// cos权重的半球样本
	// u[0] = cos(theta)*cos(theta), u[1] = phi / (2 * PI)
	Vector3f CosineSampleHemisphere(const Vector2f& u)
	{
		float e = std::sqrt((std::max)(0.f, 1 - u[0]));
		float f = 2 * PI * u[1];
		float x = e * std::cosf(f);
		float y = e * std::sinf(f);
		float z = std::sqrt((std::max)(0.f, u[0]));
		return Vector3f({ x, y, z });
	}

	// cos权重的半球PDF
	inline float CosineHemispherePdf(float cosTheta)
	{
		return cosTheta * INV_PI;
	}
}