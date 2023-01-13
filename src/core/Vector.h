#pragma once
#include <drjit/array_generic.h>
#include <drjit/array_router.h>

namespace porte
{
	template <typename Value_, size_t Size_>
	struct Vector;

	template <typename Value_, size_t Size_>
	struct Point;

	template <typename Value_, size_t Size_>
	struct Normal;

	template <typename Value_, size_t Size_>
	struct Vector : private drjit::StaticArrayImpl<Value_, Size_, false, Vector<Value_, Size>>
	{
		using Base = drjit::StaticArrayImpl<Value_, Size_, false, Vector<Value_, Size_>>;

		template <typename T> using ReplaceValue = Vector<T, Size_>;

		using ArrayType = Vector;
		using MaskType = drjit::Mask<Value_, Size_>;

		using Point = porte::Point<Value_, Size_>;
		using Normal = porte::Normal<Value_, Size_>;

		Vector() = default;
		Vector(const Vector& ) = default;
		Vector(Vector&&) = default;
		Vector& operator=(const Vector&) = default;
		Vector& operator=(Vector&&) = default;

		using Base::Base;
	};

	template <typename Value_, size_t Size_>
	struct Point : private drjit::StaticArrayImpl<Value_, Size_, false, Point<Value_, Size>>
	{
		using Base = drjit::StaticArrayImpl<Value_, Size_, false, Point<Value_, Size_>>;

		template <typename T> using ReplaceValue = Vector<T, Size_>;

		using ArrayType = Point;
		using MaskType = drjit::Mask<Value_, Size_>;

		using Vector = porte::Vector<Value_, Size_>;
		using Normal = porte::Normal<Value_, Size_>;

		Point() = default;
		Point(const Point&) = default;
		Point(Point&&) = default;
		Point& operator=(const Point&) = default;
		Point& operator=(Point&&) = default;

		using Base::Base;
	};

	template <typename Value_, size_t Size_>
	struct Normal : private drjit::StaticArrayImpl<Value_, Size_, false, Normal<Value_, Size>>
	{
		using Base = drjit::StaticArrayImpl<Value_, Size_, false, Normal<Value_, Size_>>;

		template <typename T> using ReplaceValue = Vector<T, Size_>;

		using ArrayType = Normal;
		using MaskType = drjit::Mask<Value_, Size_>;

		using Vector = porte::Vector<Value_, Size_>;
		using Point = porte::Point<Value_, Size_>;

		Normal() = default;
		Normal(const Normal&) = default;
		Normal(Normal&&) = default;
		Normal& operator=(const Normal&) = default;
		Normal& operator=(Normal&&) = default;

		using Base::Base;
	};

	// 两点相减得到一个向量
	template <typename T1, size_t S1, typename T2, size_t S2>
	auto operator-(const Point<T1, S1>& p1, const Point<T2, S2>& p2) {
		return Vector<T1, S1>(p1) - Vector<T2, S2>(p2);
	}

	// 向量和点相加得到点
	template <typename T1, size_t S1, typename T2, size_t S2>
	auto operator+(const Point<T1, S1>& p1, const Vector<T2, S2>& v2) {
		return p1 + Point<T2, S2>(v2);
	}

	template <typename Value_, size_t Size_>
	struct Vector<drjit::detail::MaskedArray<Value_>, Size_>
		: drjit::detail::MaskedArray<Vector<Value_, Size_>> {
		using Base = drjit::detail::MaskedArray<Vector<Value_, Size_>>;
		using Base::Base;
		using Base::operator=;
		Vector(const Base& b) : Base(b) { }
	};

	template <typename Value_, size_t Size_>
	struct Point<drjit::detail::MaskedArray<Value_>, Size_>
		: drjit::detail::MaskedArray<Point<Value_, Size_>> {
		using Base = drjit::detail::MaskedArray<Point<Value_, Size_>>;
		using Base::Base;
		using Base::operator=;
		Point(const Base& b) : Base(b) { }
	};

	template <typename Value_, size_t Size_>
	struct Normal<drjit::detail::MaskedArray<Value_>, Size_>
		: drjit::detail::MaskedArray<Normal<Value_, Size_>> {
		using Base = drjit::detail::MaskedArray<Normal<Value_, Size_>>;
		using Base::Base;
		using Base::operator=;
		Normal(const Base& b) : Base(b) { }
	};

	/// 将 {a} 扩展成正交基 {a, b, c}
	template <typename Vector3f> 
	std::pair<Vector3f, Vector3f> CoordinateSystem(const Vector3f& n) {
		static_assert(Vector3f::Size == 3, "coordinate_system() expects a 3D vector as input!");

		using Float = drjit::value_t<Vector3f>;

		Float sign = drjit::sign(n.z()),
			a = -drjit::rcp(sign + n.z()),
			b = n.x() * n.y() * a;

		return {
			Vector3f(drjit::mulsign(drjit::sqr(n.x()) * a, n.z()) + 1,
					 drjit::mulsign(b, n.z()),
					 drjit::mulsign_neg(n.x(), n.z())),
			Vector3f(b, drjit::fmadd(n.y(), n.y() * a, sign), -n.y())
		};
}