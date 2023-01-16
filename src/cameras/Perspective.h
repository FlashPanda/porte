#pragma once

#include <core/porte.h>
#include <core/Camera.h>
#include <core/Transform.h>

namespace porte
{
	class PerspectiveCamera : public ProjectiveCamera
	{
	public:
		PerspectiveCamera(Transform trans, Film* film);
		Float GenerateRay(const CameraSample& sample, Ray*) const;
	};

	PerspectiveCamera* CreatePerspectiveCamera(Transform trans, Film* film);
}