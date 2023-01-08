#pragma once
#include "Matrix.hpp"
#include "Bounds.h"
#include "SceneObjectMesh.h"

namespace panda
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
		virtual Interaction Sample(const Vector2Df& u, float* pdf) const = 0;
		virtual float Pdf(const Interaction&) const { return 1 / Area(); }

		// ����������һ���㣬���ҷ��ش����õ�������pdf
		virtual Interaction Sample(const Interaction& ref, const Vector2Df& u,
			float* pdf) const;
		virtual float Pdf(const Interaction& ref, const Vector3Df& wi) const;

		const Matrix4f* ObjectToWorld;
		const Matrix4f* WorldToObject;
	};

	class Triangle : public Shape
	{
	public:
		Triangle(const Matrix4f* ObjToWorld, const Matrix4f* WorldToObj,
			const std::shared_ptr<SceneObjectMesh>& mesh, int32 triNumber);

		Bounds3f ObjectBound() const;
		Bounds3f WorldBound() const;

		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect) const;
		bool IntersectP(const Ray& ray) const;

		float Area() const;

		Interaction Sample(const Vector2Df& u, float* pdf) const;

	private:
		void GetUVs(Vector2Df uv[3]) const 
		{
			if (mesh->GetUVArray().size() > 0)
			{
				uv[0] = mesh->GetUVArray()[v[0]];
				uv[1] = mesh->GetUVArray()[v[1]];
				uv[2] = mesh->GetUVArray()[v[2]];
			}
			else
			{
				uv[0] = Vector2Df({ 0, 0 });
				uv[1] = Vector2Df({ 1, 0 });
				uv[2] = Vector2Df({ 1, 1 });
			}
		}

		std::shared_ptr<SceneObjectMesh> mesh;
		const int32* v;
	};

	std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(const Matrix4f* ObjToWorld, const Matrix4f* WorldToObj, const std::shared_ptr<SceneObjectMesh>& mesh, int32 triNumber);
}