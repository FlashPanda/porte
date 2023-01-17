#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CAMERAS_PERSPECTIVE_H
#define PORTE_CAMERAS_PERSPECTIVE_H

// cameras/perspective.h*
#include <core/porte.h>
#include <core/Camera.h>
#include <core/Film.h>

namespace porte {

class PerspectiveCamera : public ProjectiveCamera {
public:
    PerspectiveCamera(const Transform &CameraToWorld,
                      const Bounds2f &screenWindow/*, Float shutterOpen,
                      Float shutterClose, Float lensRadius, Float focalDistance*/,
                      Float fov, Film *film/*, const Medium *medium*/);
    Float GenerateRay(const CameraSample &sample, Ray *) const;
    Float GenerateRayDifferential(const CameraSample &sample,
                                  RayDifferential *ray) const;
    Spectrum We(const Ray &ray, Point2f *pRaster2 = nullptr) const;
    void Pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const;
    Spectrum Sample_Wi(const Interaction &ref, const Point2f &sample,
                       Vector3f *wi, Float *pdf, Point2f *pRaster,
                       VisibilityTester *vis) const;

  private:
    Vector3f dxCamera, dyCamera;
    Float A;
};

PerspectiveCamera *CreatePerspectiveCamera(const Transform &cam2world,
                                           Film *film, Float fov/*, const Medium *medium*/);

}  // namespace porte

#endif  // PORTE_CAMERAS_PERSPECTIVE_H
