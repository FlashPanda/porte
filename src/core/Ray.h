#pragma once

#include "Vector.hpp"
#include <iostream>

namespace panda
{
	class Ray
	{
	public:
		Ray() : tMax(FloatInfinity) {}
		Ray(const Vector3Df& o, const Vector3Df d, float tMax = FloatInfinity) :
			o(o), d(d), tMax(tMax) {}

		friend std::ostream& operator<<(std::ostream& os, const Ray& r);

		Vector3Df o;	// 原点
		Vector3Df d;	// 方向
		mutable float tMax;		// t的最大值。这里的mutable是为了突破const限制。也就是说，const的ray，也能修改tMax的值。
	};
}