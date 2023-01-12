#include <core/Camera.h>

namespace porte
{
	Camera::Camera(Film* film) : mFilm(film)
	{

	}

	Camera::~Camera()
	{

	}

	ProjectiveCamera::ProjectiveCamera(Film* film)
		: Camera(film)
	{

	}

	Float ProjectiveCamera::GenerateRay(const CameraSample& sample, Ray* ray) const
	{
		return 1.f;
	}
}