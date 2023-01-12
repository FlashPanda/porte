#pragma once
#include <core/MathUtil.h>


namespace porte
{
	class Filter
	{
	public:
		virtual ~Filter() {}
		Filter(const Vector2f& radius) : mRadius(radius), mInvRadius(Vector2f{1.f / radius[0], 1.f / radius[1]}) {}
		virtual float Evaluate(const Vector2f& p) const  = 0;

		const Vector2f mRadius, mInvRadius;
	};
}