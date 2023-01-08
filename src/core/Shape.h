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

		// 在Shape表面采样一点，并且返回相对于表面积来说的PDF
		virtual Interaction Sample(const Vector2Df& u, float* pdf) const = 0;
		virtual float Pdf(const Interaction&) const { return 1 / Area(); }

		// 采样给定地一个点，并且返回从引用点地立体角pdf
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