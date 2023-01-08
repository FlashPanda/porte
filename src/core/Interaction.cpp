#include "Interaction.h"
#include "SceneNodeMesh.h"
#include "Primitive.h"

namespace panda
{
	SurfaceInteraction::SurfaceInteraction(const Vector3Df& inP, const Vector3Df& inErr,
		const Vector2Df& inUV, const Vector3Df& inWo,
		const Vector3Df& dpdu, const Vector3Df& dpdv,
		const Vector3Df& dndu, const Vector3Df& dndv,
		const Shape* inShape) 
		: Interaction(p, Normalize(CrossProduct(dpdu, dpdv)), inErr, inWo), 
		uv(inUV), 
		dpdu(dpdu),
		dpdv(dpdv),
		dndu(dndu),
		dndv(dndv),
		pShape(inShape)
	{}


	void SurfaceInteraction::ComputeScatteringFunctions(const Ray& ray)
	{
		primitive->ComputeScatteringFunctions(this);
	}

	Vector3Df SurfaceInteraction::Le(const Vector3Df& w) const
	{
		return Vector3Df(0.f);
	}
}