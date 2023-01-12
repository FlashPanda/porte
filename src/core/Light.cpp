#include "Light.h"
#include "Interaction.h"
#include "Sampling.h"
#include "Scene.h"
#include "Transform.h"

namespace porte
{
	Vector3f VisibilityTester::Tr(const Scene& scene, Sampler& sampler) const {
		return Vector3f(0.f);
	}

	bool VisibilityTester::Unoccluded(const Scene& scene) const
	{
		return !scene.IntersectP(p0.SpawnRayTo(p1));
	}

	Light::Light(int32 flags, const Matrix4f& LocalToWorld, int32 nSamples)
		: flags(flags),
		nSamples(std::max(1, nSamples)),
		LocalToWorld(LocalToWorld),
		WorldToLocal(InverseMatrix(LocalToWorld))
	{}

	Light::~Light()
	{

	}

	Vector3f Light::Le(const Ray& ray) const { return Vector3f(0.f); }

}