#ifndef MATH_UTIL_H
#define MATH_UTIL_H
#pragma once
#include <stdint.h>
#include <cmath>
#include <drjit/array.h>
#include <drjit/quaternion.h>
#include <drjit/matrix.h>
#include <drjit/transform.h>

typedef int8_t		int8;
typedef uint8_t		uint8;
typedef int16_t		int16;
typedef uint16_t	uint16;
typedef int32_t		int32;
typedef	uint32_t	uint32;
typedef int64_t		int64;
typedef	uint64_t	uint64;

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef TWO_PI
#define TWO_PI 3.14159265358979323846f * 2.0f
#endif

#ifndef HALF_PI
#define HALF_PI 3.14159265358979323846f / 2.0f
#endif

#ifndef INV_PI
#define INV_PI 0.31830988618379067154f
#endif

#ifndef INV_2_PI
#define INV_2_PI 0.15915494309189533577f
#endif

#ifndef INV_4_PI
#define INV_4_PI 0.07957747154594766788f
#endif

#ifndef PI_OVER_2
#define PI_OVER_2 1.57079632679489661923f
#endif

#ifndef PI_OVER_4
#define PI_OVER_4 1.57079632679489661923f
#endif

#ifndef SQRT_2
#define SQRT_2 1.41421356237309504880f
#endif

#ifndef ONE_QUARTER_PI
#define ONE_QUARTER_PI 3.14159265358979323846f / 4.0f
#endif

#ifndef MIN_VECTOR_LENGTH
#define MIN_VECTOR_LENGTH  1e-10
#endif

#define MaxFloat std::numeric_limits<float>::max()
#define FloatInfinity std::numeric_limits<float>::infinity()

#ifdef USING_DOUBLE
using Float = double;
#else
using Float = float;
#endif

namespace porte
{
	inline float AngleToRadian(float angle)
	{
		return 0.01745329251994329576923690768489f * angle;
	}

	inline float RadianToAngle(float radian)
	{
		return radian * 57.295779513082320876798154814105f;
	}

	inline float GammaCorrect(float value) {
		if (value <= 0.0031308f) return 12.92f * value;
		return 1.055f * std::pow(value, (float)(1.f / 2.4f)) - 0.055f;
	}

	inline float Lerp(float t, float v1, float v2)
	{
		return (1 - t) * v1 + t * v2;
	}

	template <typename T, typename U, typename V>
	inline T Clamp(T val, U low, V high) {
		if (val < low)
			return low;
		else if (val > high)
			return high;
		else
			return val;
	}

	using Vector2i = drjit::Array<int32, 2>;
	using Vector2f = drjit::Array<Float, 2>;

	using Vector3i = drjit::Array<int32, 3>;
	using Vector3f = drjit::Array<Float, 3>;

	using Vector4f = drjit::Array<Float, 4>;
	using Vector4i = drjit::Array<int32, 4>;

	using Quaternion = drjit::Quaternion<Float>;
	using Matrix2f = drjit::Matrix<Float, 2>;
	using Matrix3f = drjit::Matrix<Float, 3>;
	using Matrix4f = drjit::Matrix<Float, 4>;

	Matrix4f Translate(const Vector3f& v)
	{
		return drjit::translate<Matrix4f>(v);
	}

	Matrix4f Scale(const Vector3f& v)
	{
		return drjit::scale<Matrix4f>(v);
	}

	
	Matrix4f RotateMatrix(const Vector3f& axis, Float angle)
	{
		return drjit::rotate<Matrix4f>(axis, angle);
	}

	Quaternion RotateQuaternion(const Vector3f& axis, Float angle)
	{
		return drjit::rotate<Quaternion>(axis, angle);
	}

	// 逆矩阵
	Matrix4f Inverse(const Matrix4f& m)
	{
		return drjit::inverse(m);
	}

	// 行列式
	Float Determinant(const Matrix4f& m)
	{
		return drjit::det(m);
	}

	// 右手坐标系，看向-z方向，映射到[0,1]
	Matrix4f Perspective(const Float xfov, const Float aspect, const Float near, const Float far)
	{
		Float recip = 1.f / (near - far);
		Float tan = drjit::tan(drjit::deg_to_rad(xfov * .5f)),
			cot = 1.f / tan;

		Matrix4f trafo = drjit::diag(Vector4f(cot, aspect * cot, far * recip, 0.f);
		trafo(2, 3) = near * far * recip;
		trafo(3, 2) = -1;
		return trafo;
	}

	// 观察，右手坐标系，-z方向
	Matrix4f LookAt(const Vector3f& pos, const Vector3f& target, const Vector3f& up)
	{
		Vector3f dirInverse = drjit::normalize(pos - target);
		Vector3f right = drjit::normalize(drjit::cross(up, dirInverse));
		Vector3f newUp = drjit::cross(dirInverse, right);

		using Vector1f = drjit::Array<Float, 1>;

		Vector1f z(0);
		Matrix4f result = Matrix4f(
			drjit::concat(right, z),
			drjit::concat(newUp, z),
			drjit::concat(dirInverse, z),
			drjit::concat(pos, Vector1f(1))
		);

		return result;
	}
}


#endif