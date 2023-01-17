#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_CAMERA_H
#define PORTE_CORE_CAMERA_H

// core/camera.h*
#include <core/porte.h>
#include <core/Geometry.h>
#include <core/Transform.h>
#include <core/Film.h>

namespace porte {

class Camera {
  public:
    Camera(const Transform& CameraToWorld, Float shutterOpen,
           Float shutterClose, Film *film);
    virtual ~Camera();
    virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const = 0;
    virtual Float GenerateRayDifferential(const CameraSample &sample,
                                          RayDifferential *rd) const;
    virtual Spectrum We(const Ray &ray, Point2f *pRaster2 = nullptr) const;
    virtual void Pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const;
    virtual Spectrum Sample_Wi(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf, Point2f *pRaster,
                               VisibilityTester *vis) const;

    Transform CameraToWorld;
    const Float shutterOpen, shutterClose;
    Film *film;
    //const Medium *medium;
};

struct CameraSample {
    Point2f pFilm;
    Point2f pLens;
    Float time;
};

//inline std::ostream &operator<<(std::ostream &os, const CameraSample &cs) {
//    os << "[ pFilm: " << cs.pFilm << " , pLens: " << cs.pLens <<
//        StringPrintf(", time %f ]", cs.time);
//    return os;
//}

class ProjectiveCamera : public Camera {
  public:
    ProjectiveCamera(const Transform &CameraToWorld,
                     const Transform &CameraToScreen,
                     const Bounds2f &screenWindow, Float shutterOpen,
                     Float shutterClose, Float lensr, Float focald, Film *film/*,
                     const Medium *medium*/)
        : Camera(CameraToWorld, shutterOpen, shutterClose, film),
          CameraToScreen(CameraToScreen) {
        
        lensRadius = lensr;
        focalDistance = focald;

        // 透视相机到屏幕转换
        ScreenToRaster =
            Scale(film->fullResolution.x, film->fullResolution.y, 1) *
            Scale(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
                  1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1) *
            Translate(Vector3f(-screenWindow.pMin.x, -screenWindow.pMax.y, 0));
        RasterToScreen = Inverse(ScreenToRaster);
        RasterToCamera = Inverse(CameraToScreen) * RasterToScreen;
    }

  protected:
    
    Transform CameraToScreen, RasterToCamera;
    Transform ScreenToRaster, RasterToScreen;
    Float lensRadius, focalDistance;
};

}  // namespace porte

#endif  // PORTE_CORE_CAMERA_H
