#pragma once
#include <core/MathUtil.h>
#include <core/Film.h>
#include <core/Ray.h>

namespace porte
{
	struct CameraSample {
		Vector2f pFilm;
		Vector2f pLens;
		Float time;
	};

	class Camera
	{
	public:
		Camera(Film* film);
		virtual ~Camera();

		virtual Float GenerateRay(const CameraSample& sample, Ray* ray) const = 0;
		//virtual Float GenerateRayDifferential(const CameraSample& sample,
		//	RayDifferential* rd) const;

		Film* mFilm;
	};

	class ProjectiveCamera : public Camera
	{
	public:
		ProjectiveCamera(Film* film);

		Float GenerateRay(const CameraSample& sample, Ray* ray) const;

	protected:
		Matrix4f CameraToScreen, RasterToCamera;
		Matrix4f ScreenToRaster, RasterToScreen;
	};
}