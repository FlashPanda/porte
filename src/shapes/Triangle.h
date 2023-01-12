#pragma once
#include <core/Shape.h>
#include <vector>

namespace porte
{
	struct TriangleMesh {
		// TriangleMesh Public Methods
		TriangleMesh(const Transform& ObjectToWorld, int nTriangles,
			const int* vertexIndices, int nVertices, const Point3f* P,
			const Vector3f* S, const Normal3f* N, const Point2f* uv,
			const std::shared_ptr<Texture<Float>>& alphaMask,
			const std::shared_ptr<Texture<Float>>& shadowAlphaMask,
			const int* faceIndices);

		// TriangleMesh Data
		const int nTriangles, nVertices;
		std::vector<int> vertexIndices;
		std::unique_ptr<Point3f[]> p;
		std::unique_ptr<Normal3f[]> n;
		std::unique_ptr<Vector3f[]> s;
		std::unique_ptr<Point2f[]> uv;
	};

	class Triangle : public Shape
	{
	public:

		Bounds3f ObjectBound() const;
		Bounds3f WorldBound() const;

		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect) const;
		bool IntersectP(const Ray& ray) const;

		float Area() const;

		Interaction Sample(const Vector2f& u, float* pdf) const;

	private:
		void GetUVs(Vector2f uv[3]) const
		{
			if ()
			{
			}
			else
			{
				uv[0] = Vector2f({ 0, 0 });
				uv[1] = Vector2f({ 1, 0 });
				uv[2] = Vector2f({ 1, 1 });
			}
		}

		const int32* v;
	};

	std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(const Matrix4f* ObjToWorld, const Matrix4f* WorldToObj, int32 triNumber);
}