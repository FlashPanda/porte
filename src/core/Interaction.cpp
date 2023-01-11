#include "Interaction.h"
#include "SceneNodeMesh.h"
#include "Primitive.h"

namespace porte
{
	SurfaceInteraction::SurfaceInteraction(const Vector3f& inP, const Vector3f& inErr,
		const Vector2f& inUV, const Vector3f& inWo,
		const Vector3f& dpdu, const Vector3f& dpdv,
		const Vector3f& dndu, const Vector3f& dndv,
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

	Vector3f SurfaceInteraction::Le(const Vector3f& w) const
	{
		return Vector3f(0.f);
	}
}