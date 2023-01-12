#include <cameras/Perspective.h>

namespace porte
{
	PerspectiveCamera::PerspectiveCamera(Film* film)
		: ProjectiveCamera(film)
	{

	}

	Float PerspectiveCamera::GenerateRay(const CameraSample& sample, Ray* ray) const
	{
		return ProjectiveCamera::GenerateRay(sample, ray);
	}

	PerspectiveCamera* CreatePerspectiveCamera(Film* film)
	{
		return new PerspectiveCamera(film);
	}
}