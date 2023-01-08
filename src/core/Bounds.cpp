#include "Bounds.h"

namespace panda
{
	template <typename T>
	inline const Vector<T, 3>& Bounds3<T>::operator[](int32 i) const
	{
		assert(i == 0 || i == 1);
		return (i == 0)? pMin : pMax;
	}

	template< typename T>
	inline Vector<T, 3>& Bounds3<T>::operator[] (int32 i)
	{
		assert(i == 0 || i == 1);
		return (i == 0)? pMin : pMax;
	}

	template< typename T>
	bool Bounds3<T>::IntersectP(const Ray& ray, float* hitt0, float* hitt1) const
	{
		float t0 = 0, t1 = ray.tMax;

		// 这里的计算相当巧妙，只需要分别对每一个分量计算距离值，然后除以“速度”，
		// 就可以得到t值。如果没有交点，那么t0和t1都会是tMax
		for (int32 i = 0; i < 3; ++i)
		{
			float invSpeed = 1.f / ray.d[i];
			float tNear = (pMin[i] - ray.o[i]) * invSpeed;
			float tFar = (pMax[i] - ray.o[i]) * invSpeed;

			if (tNear > tFar) std::swap(tNear, tFar);

			// 确保ray-bound相交的鲁棒性
			// tFar *= 1 + 2 * gamma(3);

			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
		}

		if (hitt0) *hitt0 = t0;
		if (hitt1) *hitt1 = t1;

		return true;
	}

	template <typename T>
	bool Bounds3<T>::IntersectP(const Ray& ray, const Vector3Df& invDir,
		const int32 dirIsNeg[3]) const
	{
		const Bounds3f& bounds = *this;

		// 光线和x板的交点，和y板的交点
		float tMin = (bounds[dirIsNeg[0]][0] - ray.o[0]) * invDir[0];
		float tMax = (bounds[1 - dirIsNeg[0]][0] - ray.o[0]) * invDir[0];
		float tyMin = (bounds[dirIsNeg[1]][1] - ray.o[1]) * invDir[1];
		float tyMax = (bounds[1 - dirIsNeg[1]][1] - ray.o[1]) * invDir[1];

		// 确保鲁棒性
		// tMax *= 1 + 2 * gamma(3);
		// tyMax *= 1 + 2 * gamma(3);
		if (tMin > tyMax || tyMin > tMax) return false;
		if (tyMin > tMin) tMin = tyMin;
		if (tyMax < tMax) tMax = tyMax;

		// 检查光线和z板相交
		float tzMin = (bounds[dirIsNeg[2]][2] - ray.o[2]) * invDir[2];
		float tzMax = (bounds[1 - dirIsNeg[2]][2] - ray.o[2]) * invDir[2];

		// 确保鲁棒性
		// tzMax *= 1 + 2 * gamma(3);
		if (tMin > tzMax || tzMin > tMax) return false;
		if (tzMin > tMin) tMin = tzMin;
		if (tzMax < tMax) tMax = tzMax;
		return (tMin < ray.tMax) && (tMax > 0);
	}
}