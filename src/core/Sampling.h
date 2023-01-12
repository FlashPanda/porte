	#ifndef SAMPLING_H
#define SAMPLING_H
#pragma once
#include <core/MathUtil.h>

namespace porte
{
	// 球体均匀采样
	inline Vector3f UniformSampleSphere(const Vector2f& u);

	// 球体pdf
	inline float UniformSpherePdf();

	// 均匀半球样本
	inline Vector3f UniformSampleHemisphere(const Vector2f& u);

	// 半球PDF
	inline float UniformHemispherePdf();

	// cos权重的半球样本
	inline Vector3f CosineSampleHemisphere(const Vector2f& u);

	// cos权重的半球PDF
	inline float CosineHemispherePdf(float cosTheta);
}

#endif