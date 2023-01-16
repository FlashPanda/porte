#include <core/Camera.h>
#include <core/Sampling.h>
#include <core/Sampler.h>

namespace porte {

Camera::~Camera() { delete film; }

Camera::Camera(const Transform& CameraToWorld, Float shutterOpen,
               Float shutterClose, Film *film)
    : CameraToWorld(CameraToWorld),
      shutterOpen(shutterOpen),
      shutterClose(shutterClose),
      film(film)
{
}

Float Camera::GenerateRayDifferential(const CameraSample &sample,
                                      RayDifferential *rd) const {
    Float wt = GenerateRay(sample, rd);
    if (wt == 0) return 0;

    // x方向上偏移导致的ray的改变
    Float wtx;
    for (Float eps : { .05, -.05 }) {
        CameraSample sshift = sample;
        sshift.pFilm.x += eps;
        Ray rx;
        wtx = GenerateRay(sshift, &rx);
        rd->rxOrigin = rd->o + (rx.o - rd->o) / eps;
        rd->rxDirection = rd->d + (rx.d - rd->d) / eps;
        if (wtx != 0)
            break;
    }
    if (wtx == 0)
        return 0;

    // y方向上偏移导致的ray的改变
    Float wty;
    for (Float eps : { .05, -.05 }) {
        CameraSample sshift = sample;
        sshift.pFilm.y += eps;
        Ray ry;
        wty = GenerateRay(sshift, &ry);
        rd->ryOrigin = rd->o + (ry.o - rd->o) / eps;
        rd->ryDirection = rd->d + (ry.d - rd->d) / eps;
        if (wty != 0)
            break;
    }
    if (wty == 0)
        return 0;

    rd->hasDifferentials = true;
    return wt;
}

Spectrum Camera::We(const Ray &ray, Point2f *raster) const {
    //LOG(FATAL) << "Camera::We() is not implemented!";
    return Spectrum(0.f);
}

void Camera::Pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const {
    //LOG(FATAL) << "Camera::Pdf_We() is not implemented!";
}

Spectrum Camera::Sample_Wi(const Interaction &ref, const Point2f &u,
                           Vector3f *wi, Float *pdf, Point2f *pRaster,
                           VisibilityTester *vis) const {
    //LOG(FATAL) << "Camera::Sample_Wi() is not implemented!";
    return Spectrum(0.f);
}

}  // namespace porte
