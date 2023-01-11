#pragma once

#include <core/MathUtil.h>
#include <iostream>

namespace porte
{
	class Ray
	{
	public:
		Ray() : tMax(FloatInfinity) {}
		Ray(const Vector3f& o, const Vector3f d, float tMax = FloatInfinity) :
			o(o), d(d), tMax(tMax) {}

		friend std::ostream& operator<<(std::ostream& os, const Ray& r);

		Vector3f o;	// 原点
		Vector3f d;	// 方向
		mutable float tMax;		// t的最大值。这里的mutable是为了突破const限制。也就是说，const的ray，也能修改tMax的值。
	};
}