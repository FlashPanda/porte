#include <cameras/Perspective.h>

namespace porte
{
	PerspectiveCamera::PerspectiveCamera(Transform trans, Film* film)
		: ProjectiveCamera(trans, film)
	{

	}

	Float PerspectiveCamera::GenerateRay(const CameraSample& sample, Ray* ray) const
	{
		return ProjectiveCamera::GenerateRay(sample, ray);
	}

	PerspectiveCamera* CreatePerspectiveCamera(Transform trans, Film* film)
	{
		return new PerspectiveCamera(trans, film);
	}
}