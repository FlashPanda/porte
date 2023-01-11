#pragma once

#include <core/MathUtil.h>
#include <core/Ray.h>

namespace porte
{
	class SceneNodeMesh;
	class BSDF;
	class Shape;
	class Primitive;
	struct Interaction
	{
		Interaction() {}
		Interaction(const Vector3f& inP, const Vector3f& inN, const Vector3f& inPErr,
			const Vector3f& inWo) :
			p(inP), pErr(inPErr), n(inN), wo(inWo)
		{
		}

		Interaction(const Vector3f& p, const Vector3f& wo)
			: p(p), wo(wo)
		{}

		Interaction(const Vector3f& p)
			: p(p) {}

		bool IsSurfaceInteraction() const {return n != Vector3f(0.f); }

		Ray SpawnRay(const Vector3f& d) const
		{
			return Ray(p, d, FloatInfinity);
		}

		Ray SpawnRayTo(const Vector3f& p2) const
		{
			Vector3f d = p2 - p;
			return Ray(p, d, FloatInfinity);
		}

		Ray SpawnRayTo(const Interaction& it) const
		{
			Vector3f d = it.p - p;
			return Ray(p, d, FloatInfinity);
		}

		Vector3f p;
		Vector3f pErr;
		Vector3f wo;	// \Omega_o
		Vector3f n;
	};

	struct SurfaceInteraction : public Interaction
	{
		SurfaceInteraction() {}
		SurfaceInteraction(const Vector3f& inP, const Vector3f& inErr, 
			const Vector2f& inUV, const Vector3f& inWo, 
			const Vector3f& dpdu, const Vector3f& dpdv,
			const Vector3f& dndu, const Vector3f& dndv,
			const Shape* inShape);

		void ComputeScatteringFunctions(const Ray& ray);

		Vector3f Le(const Vector3f& w) const;

		Vector2f uv;

		// 位置关于uv参数的偏导
		Vector3f dpdu, dpdv;

		// 法线关于uv参数的偏导
		Vector3f dndu, dndv;

		// 位置关于屏幕空间坐标的偏导
		mutable Vector3f dpdx, dpdy;

		// uv关于屏幕空间坐标的偏导
		mutable Vector2f duvdx, duvdy;

		BSDF* bsdf = nullptr;
		const Shape* pShape = nullptr;

		std::shared_ptr<SceneNodeMesh> pMeshNode;
		const Primitive* primitive = nullptr;
	};
}