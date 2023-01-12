#pragma once
#include <core/MathUtil.h>
#include <core/Bounds.h>

namespace porte
{
	class Interaction;
	class SurfaceInteraction;
	class Shape
	{
	public:
		Shape(const Matrix4f* ObjToWorld, const Matrix4f* WorldToObj);
		virtual ~Shape();
		virtual Bounds3f ObjectBound() const = 0;
		virtual Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect) const = 0;
		virtual bool IntersectP(const Ray& ray) const;

		virtual float Area() const = 0;

		// ��Shape�������һ�㣬���ҷ�������ڱ������˵��PDF
		virtual Interaction Sample(const Vector2f& u, float* pdf) const = 0;
		virtual float Pdf(const Interaction&) const { return 1 / Area(); }

		// ����������һ���㣬���ҷ��ش����õ�������pdf
		virtual Interaction Sample(const Interaction& ref, const Vector2f& u,
			float* pdf) const;
		virtual float Pdf(const Interaction& ref, const Vector3f& wi) const;

		const Matrix4f* ObjectToWorld;
		const Matrix4f* WorldToObject;
	};



}