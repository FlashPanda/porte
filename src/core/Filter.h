#pragma once
#include "MathUtil.h"
#include "Vector.hpp"


namespace panda
{
	class Filter
	{
	public:
		virtual ~Filter() {}
		Filter(const Vector2Df& radius) : mRadius(radius), mInvRadius(Vector2Df{1.f / radius[0], 1.f / radius[1]}) {}
		virtual float Evaluate(const Vector2Df& p) const  = 0;

		const Vector2Df mRadius, mInvRadius;
	};

	// Box Filter
	class BoxFilter : public Filter
	{
	public:
		virtual ~BoxFilter() {}
		BoxFilter(const Vector2Df& radius) : Filter(radius) {}
		float Evaluate(const Vector2Df& p) const;
	};

	Filter* CreateBoxFilter();
}