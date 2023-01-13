#pragma once

#include <core/Ray.h>
#include <drjit/transform.h>
#include <drjit/sphere.h>

namespace porte
{
	/*
	* 封装了4x4的齐次转换
	* 
	* 为vectors，points，normals提供重载的矩阵相乘操作
	*/
	template<typename Point_>
	struct Transform
	{
		static constexpr size_t Size = Point_::Size;
		using TFloat = drjit::value_t<Point_>;
		using Matrix = drjit::Matrix<TFloat, Size>;
		using Mask = drjit::mask_t<TFloat>;
		using Scalar = drjit::scalar_t<TFloat>;

		Matrix matrix = drjit::identity<Matrix>();

		/// 从matrix初始化transformation
		Transform(const Matrix& value)
			: matrix(value)
		{}

		/// transformation的串联
		__forceinline Transform operator*(const Transform& other) const
		{
			return Transform(matrix * other.matrix);
		}

		// 逆变换
		__forceinline Transform Inverse() const
		{
			return drjit::inverse(matrix);
		}

		/// 从矩阵中获取平移的部分
		Vector<TFloat, Size - 1> Translation() const
		{
			return drjit::head<Size - 1>(matrix.entry(Size - 1));
		}

		/// 相等操作
		bool operator==(const Transform& t) const
		{
			return matrix == t.matrix;
		}

		/// 不等操作
		bool operator!=(const Transform& t) const
		{
			return matrix != t.matrix;
		}

		/// 用仿射3D转换去转换一个3D vector/point/normal/ray
		template <typename T>
		__forceinline auto TransformAffine(const T& input) const
		{
			return operator*(input);
		}

		/// 转换一个point（仅处理仿射/无透视的转换）
		template <typename T, typename Expr = drjit::expr_t<TFloat, T>>
		__forceinline Point<Expr, Size - 1> TransformAffine(const Point<T, Size - 1>& arg) const
		{
			drjit::Array<Expr, Size> result = matrix.entry(Size - 1);

			for (size_t i = 0; i < Size - 1; ++i)
				result = drjit::fmadd(matrix.entry(i), arg.entry(i), result);

			return drjit::head<Size - 1>(result);	// 无相除的一步
		}

		/// 转换一个3D点
		template <typename T, typename Expr = drjit::expr_t<TFloat, T>>
		__forceinline Point<Expr, Size - 1> operator*(const Point<T, Size - 1>& arg) const
		{
			drjit::Array<Expr, Size> result = matrix.entry(Size - 1);
			
			for (size_t i = 0; i < Size - 1; ++i)
				result = drjit::fmadd(matrix.entry(i), arg.entry(i), result);

			return drjit::head<Size - 1>(result) / result.entry(Size - 1);
		}

		/// 转换一个3D向量
		template <typename T, typename Expr = drjit::expr_t<TFloat, T>>
		__forceinline Vector<Expr, Size - 1> operator*(const Vector<T, Size - 1>& arg) const
		{
			drjit::Array<Expr, Size> result = matrix.entry(0);
			result *= arg.x();

			for (size_t i = 1; i < Size - 1; ++i)
				result = drjit::fmadd(matrix.entry(i), arg.entry(i), result);

			return drjit::head<Size - 1>(result);
		}

		/// 转换一个3D
		template<typename T, typename Expr = drjit::expr_t<TFloat, T>>
		__forceinline Normal<Expr, Size - 1> operator*(const Normal<T, Size - 1>& arg) const
		{
			Matrix inverse_transpose = drjit::inverse_transpose(matrix);
			drjit::Array<Expr, Size> result = inverse_transpose.entry(0);
			result *= arg.x();

			for (size_t i = 1; i < Size - 1; ++i)
				result = drjit::fmadd(inverse_transpose.entry(i), arg.entry(i), result);

			return drjit::head<Size - 1>(result);
		}

		/// 变换一条射线（透视变换）
		Ray operator*(const Ray& ray) const
		{
			return Ray(operator*(ray.o), operator*(ray.d), ray.tMax);
		}

		/// 变换一条射线（仿射/非透视变换）
		Ray TransformAffine(const Ray& ray) const
		{
			return Ray(TransformAffine(ray.o), TransformAffine(ray.d), ray.tMax);
		}

		/// 创建一个平移变换
		static Transform Translate(const Vector<TFloat, Size - 1>& v)
		{
			return Transform(drjit::translate(Matrix)(v));
		}

		/// 创建一个缩放变换
		static Transform Scale(const Vector<TFloat, Size - 1>& v)
		{
			return Transform(drjit::scale<Matrix>(v));
		}

		/// 创建一个旋转变换，绕3D空间中的任意轴，输入参数用角度而非弧度
		template <size_t N = Size, drjit::enable_if_t<N == 4> = 0>
		static Transform Rotate(const Vector<TFloat, Size - 1>& axis, const TFloat& degrees)
		{
			Matrix tMatrix = drjit::rotate<Matrix>(axis, drjit::deg_to_rad(degrees));
			return Transform(tMatrix);
		}

		/// 创建一个2D旋转变换，输出参数是角度而非弧度
		template <size_t N = Size, drjit::enable_if_t<N == 3> = 0>
		static Transform Rotate(const TFloat& degrees)
		{
			Matrix tMatrix = drjit::rotate<Matrix>(drjit::deg_to_rad(degrees));
			return Transform(tMatrix);
		}

		/// 透视投影
		template <size_t N = SIZE, drjit::enable_if_t<N == 4> = 0>
		static Transform Perspective(TFloat xfov, TFloat aspect, TFloat near_, TFloat far_)
		{
			TFloat recip = 1.f / (near_ - far_);
			TFloat tan = drjit::tan(drjit::deg_to_rad(xfov * .5f)),
				cot = 1.f / tan;

			Matrix trafo = drjit::diag(Vector4f(cot, aspect * cot, far_ * recip, 0.f));
			trafo(2, 3) = near_ * far_ * recip;
			trafo(3, 2) = -1;
			return Transform(trafo);
		}

		/// 正交投影
		//template <size_t N = SIZE, drjit::enable_if_t<N == 4> = 0>
		//static Transform Orthographic(TFloat near_, TFloat far_)
		//{
		//	return Scale({ 1.f, 1.f, 1.f / (far_ - near_) }) * Translate({ 0.f, 0.f, -near_ });
		//}

		// 观察，右手坐标系，-z方向
		template <size_t N = SIZE, drjit::enable_if_t<N == 4> = 0>
		static Transform LookAt(const Point<TFloat, 3>& pos,
			const Point<TFloat, 3>& target,
			const Vector<TFloat, 3>& up)
		{
			using Vector1 = drjit::Array<Scalar, 1>;
			using Vector3 = drjit::Array<TFloat, 3>;

			Vector3 dirInverse = drjit::normalize(pos - target);
			Vector3 right = drjit::normalize(drjit::cross(up, dirInverse));
			Vector3 newUp = drjit::cross(dirInverse, right);

			using Vector1f = drjit::Array<Float, 1>;

			Vector1 z(0);
			Matrix result = Matrix(
				drjit::concat(right, z),
				drjit::concat(newUp, z),
				drjit::concat(dirInverse, z),
				drjit::concat(pos, Vector1(1))
			);

			return Transform(result);
		}
	};


}