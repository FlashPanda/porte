#pragma once

#include <core/Camera.h>

namespace porte
{
	class PerspectiveCamera : public ProjectiveCamera
	{
	public:
		PerspectiveCamera(Film* film);
		Float GenerateRay(const CameraSample& sample, Ray*) const;
	};

	PerspectiveCamera* CreatePerspectiveCamera(Film* film);
}