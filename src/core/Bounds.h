#pragma once

#include <core/MathUtil.h>
#include <iostream>

namespace porte
{
	class Ray;

	// 包围体
	template <typename T>
	class Bounds2
	{
	public:
		Bounds2()
		{
			T minNum = std::numeric_limits<T>::lowest();
			T maxNum = std::numeric_limits<T>::max();

			pMin = Vector<T, 2>({ maxNum, maxNum });
			pMax = Vector<T, 2>({ minNum, minNum });
		}

		explicit Bounds2(const Vector<T, 2>& p) : pMin(p), pMax(p) {}
		Bounds2(const Vector<T, 2>& p1, const Vector<T, 2>& p2)
		{
			pMin = Vector<T, 2>({ (std::min)(p1[0], p2[0]), (std::min)(p1[1], p2[1]) });
			pMax = Vector<T, 2>({ (std::max)(p1[0], p2[0]), (std::max)(p1[1], p2[1]) });
		}

		Vector<T, 2> Diagonal() const { return pMax - pMin; }

		T Area() const
		{
			Vector<T, 2> d = pMax - pMin;
			return d[0] * d[1];
		}

		// 长轴是x轴还是y轴
		int32 MaximumExtent() const
		{
			Vector<T, 2> diag = Diagonal();
			if (diag[0] > diag[1])
				return 0;
			else
				return 1;
		}

		inline const Vector<T, 2>& operator[] (int32 i) const
		{
			assert(i == 0 || i == 1);
			return (i == 0) ? pMin : pMax;
		}

		inline Vector<T, 2>& operator[] (int32 i)
		{
			assert(i == 0 || i == 1);
			return (i == 0) ? pMin : pMax;
		}

		bool operator== (const Bounds2<T>& b) const
		{
			return b.pMin == pMin && b.pMax == pMax;
		}

		bool operator!=(const Bounds2<T>& b) const
		{
			return b.pMin != pMin || b.pMax != pMax;
		}

		Vector<T, 2> Lerp(const Vector2f& t) const
		{
			return Vector<T, 2>({
				::Lerp(t[0], pMin[0], pMax[0]),
				::Lerp(t[1], pMin[1], pMax[1])
				})
		}

		Vector<T, 2> Offset(const Vector<T, 2>& p) const {
			Vector<T, 2> o = p - pMin;
			if (pMax[0] > pMin[0]) o[0] /= pMax[0] - pMin[0];
			if (pMax[1] > pMin[1]) o[1] /= pMax[1] - pMin[1];
			return o;
		}

		friend std::ostream& operator<<(std::ostream& os, const Bounds2<T>& b)
		{
			os << "[ " << b.pMin << " - " << b.pMax << " ]";
			return os;
		}

		Vector<T, 2> pMin, pMax;
	};

	template <typename T>
	class Bounds3
	{
	public:
		Bounds3()
		{
			T minNum = std::numeric_limits<T>::lowest();
			T maxNum = std::numeric_limits<T>::max();
			pMin = Vector<T, 3>({ maxNum, maxNum, maxNum });
			pMax = Vector<T, 3>({ minNum, minNum, minNum });
		}

		explicit Bounds3(const Vector<T, 3>& p) : pMin(p), pMax(p) {}
		Bounds3(const Vector<T, 3>& p1, const Vector<T, 3>& p2) :
			pMin({ (std::min)(p1[0], p2[0]), (std::min)(p1[1], p2[1]), (std::min)(p1[2], p2[2]) }),
			pMax({ (std::max)(p1[0], p2[0]), (std::max)(p1[1], p2[1]), (std::max)(p1[2], p2[2]) })
		{}

		inline const Vector<T, 3>& operator[](int32 i) const;
		inline Vector<T, 3>& operator[] (int32 i);

		bool operator==(const Bounds3<T>& b) const {
			return b.pMin == pMin && b.pMax == pMax;
		}

		bool operator!=(const Bounds3<T>& b) const
		{
			return b.pMin != pMin || b.pMax != pMax;
		}

		Vector<T, 3> Diagonal() const { return pMax - pMin; }

		T SurfaceArea() const
		{
			Vector<T, 3> d = Diagonal();
			return 2 * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
		}

		T Volume() const
		{
			Vector<T, 3> d = Diagonal();
			return d[0] * d[1] * d[2];
		}

		int32 MaximumExtent() const
		{
			Vector<T, 3> d = Diagonal();
			if (d[0] > d[1] && d[0] > d[2])
				return 0;
			else if (d[1] > d[2])
				return 1;
			else
				return 2;
		}

		Vector<T, 3> Lerp(const Vector<T, 3>& t) const
		{
			return Vector<T, 3>({
				::Lerp(t[0], pMin[0], pMax[0]),
				::Lerp(t[1], pMin[1], pMax[1]),
				::Lerp(t[2], pMin[2], pMax[2])
				});
		}

		Vector<T, 3> Offset(const Vector<T, 3>& p) const
		{
			Vector<T, 3> o = p - pMin;
			if (pMax[0] > pMin[0]) o[0] /= pMax[0] - pMin[0];
			if (pMax[1] > pMin[1]) o[1] /= pMax[1] - pMin[1];
			if (pMax[2] > pMin[2]) o[2] /= pMax[2] - pMin[2];
			return o;
		}

		bool IntersectP(const Ray& ray, float* hitt0 = nullptr, float* hitt1 = nullptr) const;

		inline bool IntersectP(const Ray& ray, const Vector3f& invDir,
			const int32 dirIsNeg[3]) const;


		friend std::ostream& operator<<(std::ostream& os, const Bounds3<T>& b)
		{
			os << "[ " << b.pMin << " - " << b.pMax << " ]";
			return os;
		}

		Vector<T, 3> pMin, pMax;
	};

	typedef Bounds2<float> Bounds2f;
	typedef Bounds2<int32> Bounds2i;
	typedef Bounds3<float> Bounds3f;
	typedef Bounds3<int32> Bounds3i;

	template <typename T>
	Bounds3<T> Union(const Bounds3<T>& b, const drjit::Array<T, 3>& p)
	{
		Bounds3<T> ret;
		ret.pMin = Min(b.pMin, p);
		ret.pMax = Max(b.pMax, p);
		return ret;
	}

	template <typename T>
	Bounds3<T> Union(const Bounds3<T>& b1, const Bounds3<T>& b2)
	{
		Bounds3<T> ret;
		ret.pMin = Min(b1.pMin, b2.pMin);
		ret.pMax = Max(b1.pMax, b2.pMax);
		return ret;
	}
}
