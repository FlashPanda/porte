#include "Shape.h"
#include "Interaction.h"
#include "Transform.h"

namespace porte
{
	Shape::~Shape() {}

	Shape::Shape(const Matrix4f* ObjToWorld, const Matrix4f* WorldToObj)
		: ObjectToWorld(ObjToWorld), WorldToObject(WorldToObj)
	{

	}

	Bounds3f Shape::WorldBound() const
	{
		return TransformBounds3f(ObjectBound(),*ObjectToWorld);
	}

	bool Shape::IntersectP(const Ray& ray) const
	{
		return Intersect(ray, nullptr, nullptr);
	}

	Interaction Shape::Sample(const Interaction& ref, const Vector2f& u,
		float* pdf) const
	{
		Interaction intr = Sample(u, pdf);
		Vector3f wi = intr.p - ref.p;

		if (GetLengthSquare(wi) == 0)
			*pdf = 0;
		else
		{
			wi = Normalize(wi);

			// 从面积的测度转换成立体角的测度
			*pdf *= GetLengthSquare(ref.p - intr.p) / AbsDotProduct(intr.n, -wi);
			if (std::isinf(*pdf)) *pdf = 0.f;
		}

		return intr;
	}

	float Shape::Pdf(const Interaction& ref, const Vector3f& wi) const
	{
		// TODO: Intersect sample ray with area light geometry
		return 0.f;
	}

}