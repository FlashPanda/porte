#pragma once

#include "MathUtil.h"
#include "Matrix.hpp"
#include "Vector.hpp"
#include "Ray.h"

namespace panda
{
	class SceneNodeMesh;
	class BSDF;
	class Shape;
	class Primitive;
	struct Interaction
	{
		Interaction() {}
		Interaction(const Vector3Df& inP, const Vector3Df& inN, const Vector3Df& inPErr,
			const Vector3Df& inWo) :
			p(inP), pErr(inPErr), n(inN), wo(inWo)
		{
		}

		Interaction(const Vector3Df& p, const Vector3Df& wo)
			: p(p), wo(wo)
		{}

		Interaction(const Vector3Df& p)
			: p(p) {}

		bool IsSurfaceInteraction() const {return n != Vector3Df(0.f); }

		Ray SpawnRay(const Vector3Df& d) const
		{
			return Ray(p, d, FloatInfinity);
		}

		Ray SpawnRayTo(const Vector3Df& p2) const
		{
			Vector3Df d = p2 - p;
			return Ray(p, d, FloatInfinity);
		}

		Ray SpawnRayTo(const Interaction& it) const
		{
			Vector3Df d = it.p - p;
			return Ray(p, d, FloatInfinity);
		}

		Vector3Df p;
		Vector3Df pErr;
		Vector3Df wo;	// \Omega_o
		Vector3Df n;
	};

	struct SurfaceInteraction : public Interaction
	{
		SurfaceInteraction() {}
		SurfaceInteraction(const Vector3Df& inP, const Vector3Df& inErr, 
			const Vector2Df& inUV, const Vector3Df& inWo, 
			const Vector3Df& dpdu, const Vector3Df& dpdv,
			const Vector3Df& dndu, const Vector3Df& dndv,
			const Shape* inShape);

		void ComputeScatteringFunctions(const Ray& ray);

		Vector3Df Le(const Vector3Df& w) const;

		Vector2Df uv;

		// 位置关于uv参数的偏导
		Vector3Df dpdu, dpdv;

		// 法线关于uv参数的偏导
		Vector3Df dndu, dndv;

		// 位置关于屏幕空间坐标的偏导
		mutable Vector3Df dpdx, dpdy;

		// uv关于屏幕空间坐标的偏导
		mutable Vector2Df duvdx, duvdy;

		BSDF* bsdf = nullptr;
		const Shape* pShape = nullptr;

		std::shared_ptr<SceneNodeMesh> pMeshNode;
		const Primitive* primitive = nullptr;
	};
}