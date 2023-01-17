// cameras/perspective.cpp*
#include <cameras/perspective.h>
#include <core/Sampler.h>
#include <core/Sampling.h>
#include <core/Light.h>
//#include "stats.h"

namespace porte {

PerspectiveCamera::PerspectiveCamera(const Transform &CameraToWorld,
                                     const Bounds2f &screenWindow,
                                     /*Float shutterOpen, Float shutterClose,
                                     Float lensRadius, Float focalDistance,*/
                                     Float fov, Film *film/*,
                                     const Medium *medium*/)
    : ProjectiveCamera(CameraToWorld, Perspective(fov, 1e-2f, 1000.f),
                       screenWindow, 0.f, 1.f, 0,
                       1e6, film/*, medium*/) {
    // 计算微分差值
    dxCamera =
        (RasterToCamera(Point3f(1, 0, 0)) - RasterToCamera(Point3f(0, 0, 0)));
    dyCamera =
        (RasterToCamera(Point3f(0, 1, 0)) - RasterToCamera(Point3f(0, 0, 0)));

    // 计算z=1时的图像边界
    Point2i res = film->fullResolution;
    Point3f pMin = RasterToCamera(Point3f(0, 0, 0));
    Point3f pMax = RasterToCamera(Point3f(res.x, res.y, 0));
    pMin /= pMin.z;
    pMax /= pMax.z;
    A = std::abs((pMax.x - pMin.x) * (pMax.y - pMin.y));
}

Float PerspectiveCamera::GenerateRay(const CameraSample &sample,
                                     Ray *ray) const {
    //ProfilePhase prof(Prof::GenerateCameraRay);
    // Compute raster and camera sample positions
    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = RasterToCamera(pFilm);
    *ray = Ray(Point3f(0, 0, 0), Normalize(Vector3f(pCamera)));
    // 因景深而修改ray
    if (lensRadius > 0) {
        // 镜头上采样点
        Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);

        // 焦点平面上的点
        Float ft = focalDistance / ray->d.z;
        Point3f pFocus = (*ray)(ft);

        // 更新ray，为了len的效果
        ray->o = Point3f(pLens.x, pLens.y, 0);
        ray->d = Normalize(pFocus - ray->o);
    }
    ray->time = Lerp(sample.time, shutterOpen, shutterClose);
    //ray->medium = medium;
    *ray = CameraToWorld(*ray);
    return 1;
}

Float PerspectiveCamera::GenerateRayDifferential(const CameraSample &sample,
                                                 RayDifferential *ray) const {
    //ProfilePhase prof(Prof::GenerateCameraRay);
    // Compute raster and camera sample positions
    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = RasterToCamera(pFilm);
    Vector3f dir = Normalize(Vector3f(pCamera.x, pCamera.y, pCamera.z));
    *ray = RayDifferential(Point3f(0, 0, 0), dir);
    // 因景深而修改ray
    if (lensRadius > 0) {
        // 镜头上采样点
        Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);

        // 焦点平面上的点
        Float ft = focalDistance / ray->d.z;
        Point3f pFocus = (*ray)(ft);

        // 更新ray，为了len的效果
        ray->o = Point3f(pLens.x, pLens.y, 0);
        ray->d = Normalize(pFocus - ray->o);
    }

    // 计算 _PerspectiveCamera_ 射线微分的偏移射线
    if (lensRadius > 0) {
        // 考虑镜头情况下，计算_PerspectiveCamera_的射线微分

        // 镜头上采样点
        Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);
        Vector3f dx = Normalize(Vector3f(pCamera + dxCamera));
        Float ft = focalDistance / dx.z;
        Point3f pFocus = Point3f(0, 0, 0) + (ft * dx);
        ray->rxOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->rxDirection = Normalize(pFocus - ray->rxOrigin);

        Vector3f dy = Normalize(Vector3f(pCamera + dyCamera));
        ft = focalDistance / dy.z;
        pFocus = Point3f(0, 0, 0) + (ft * dy);
        ray->ryOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->ryDirection = Normalize(pFocus - ray->ryOrigin);
    } else {
        ray->rxOrigin = ray->ryOrigin = ray->o;
        ray->rxDirection = Normalize(Vector3f(pCamera) + dxCamera);
        ray->ryDirection = Normalize(Vector3f(pCamera) + dyCamera);
    }
    ray->time = Lerp(sample.time, shutterOpen, shutterClose);
    //ray->medium = medium;
    *ray = CameraToWorld(*ray);
    ray->hasDifferentials = true;
    return 1;
}

Spectrum PerspectiveCamera::We(const Ray &ray, Point2f *pRaster2) const {
    // 插值相机矩阵，检查w是否朝向前方
    Transform c2w= CameraToWorld;
    //CameraToWorld.Interpolate(ray.time, &c2w);
    Float cosTheta = Dot(ray.d, c2w(Vector3f(0, 0, 1)));
    if (cosTheta <= 0) return 0;

    // 将ray映射到光栅网格
    Point3f pFocus = ray((lensRadius > 0 ? focalDistance : 1) / cosTheta);
    Point3f pRaster = Inverse(RasterToCamera)(Inverse(c2w)(pFocus));

    // 返回光栅位置
    if (pRaster2) *pRaster2 = Point2f(pRaster.x, pRaster.y);

    // 超出边界的点importance都是0
    Bounds2i sampleBounds = film->GetSampleBounds();
    if (pRaster.x < sampleBounds.pMin.x || pRaster.x >= sampleBounds.pMax.x ||
        pRaster.y < sampleBounds.pMin.y || pRaster.y >= sampleBounds.pMax.y)
        return 0;

    // 计算相机的镜头面积
    Float lensArea = lensRadius != 0 ? (Pi * lensRadius * lensRadius) : 1;

    // 返回图像平面上点的importance
    Float cos2Theta = cosTheta * cosTheta;
    return Spectrum(1 / (A * lensArea * cos2Theta * cos2Theta));
}

void PerspectiveCamera::Pdf_We(const Ray &ray, Float *pdfPos,
                               Float *pdfDir) const {
    
    // 插值相机矩阵，检查w是否朝向前方
    Transform c2w = CameraToWorld;
    //CameraToWorld.Interpolate(ray.time, &c2w);
    Float cosTheta = Dot(ray.d, c2w(Vector3f(0, 0, 1)));
    if (cosTheta <= 0) {
        *pdfPos = *pdfDir = 0;
        return;
    }

    // 将ray映射到光栅网格
    Point3f pFocus = ray((lensRadius > 0 ? focalDistance : 1) / cosTheta);
    Point3f pRaster = Inverse(RasterToCamera)(Inverse(c2w)(pFocus));

    // 超出边界的点importance都是0
    Bounds2i sampleBounds = film->GetSampleBounds();
    if (pRaster.x < sampleBounds.pMin.x || pRaster.x >= sampleBounds.pMax.x ||
        pRaster.y < sampleBounds.pMin.y || pRaster.y >= sampleBounds.pMax.y) {
        *pdfPos = *pdfDir = 0;
        return;
    }

    // 计算相机的镜头面积
    Float lensArea = lensRadius != 0 ? (Pi * lensRadius * lensRadius) : 1;
    *pdfPos = 1 / lensArea;
    *pdfDir = 1 / (A * cosTheta * cosTheta * cosTheta);
}

Spectrum PerspectiveCamera::Sample_Wi(const Interaction &ref, const Point2f &u,
                                      Vector3f *wi, Float *pdf,
                                      Point2f *pRaster,
                                      VisibilityTester *vis) const {
    // 统一采样镜头交互 _lensIntr_
    Point2f pLens = lensRadius * ConcentricSampleDisk(u);
    Point3f pLensWorld = CameraToWorld(Point3f(pLens.x, pLens.y, 0));
    Interaction lensIntr(pLensWorld, ref.time/*, medium*/);
    lensIntr.n = Normal3f(CameraToWorld(Vector3f(0, 0, 1)));

    // 填充参数并计算重要性值
    *vis = VisibilityTester(ref, lensIntr);
    *wi = lensIntr.p - ref.p;
    Float dist = wi->Length();
    *wi /= dist;

    // 计算点ref的重要性值的PDF

    // 计算透视相机的镜头面积
    Float lensArea = lensRadius != 0 ? (Pi * lensRadius * lensRadius) : 1;
    *pdf = (dist * dist) / (AbsDot(lensIntr.n, *wi) * lensArea);
    return We(lensIntr.SpawnRay(-*wi), pRaster);
}

PerspectiveCamera *CreatePerspectiveCamera(const Transform &cam2world,
                                           Film *film, Float fov/*, const Medium *medium*/) {
    // Extract common camera parameters from _ParamSet_
    //Float shutteropen = params.FindOneFloat("shutteropen", 0.f);
    //Float shutterclose = params.FindOneFloat("shutterclose", 1.f);
    //if (shutterclose < shutteropen) {
    //    Warning("Shutter close time [%f] < shutter open [%f].  Swapping them.",
    //            shutterclose, shutteropen);
    //    std::swap(shutterclose, shutteropen);
    //}
    //Float lensradius = params.FindOneFloat("lensradius", 0.f);
    //Float focaldistance = params.FindOneFloat("focaldistance", 1e6);
    //Float frame = params.FindOneFloat(
    //    "frameaspectratio",
        //Float(film->fullResolution.x) / Float(film->fullResolution.y));
    Float frame = 1.f;
    Bounds2f screen;
    //if (frame > 1.f) {
    //    screen.pMin.x = -frame;
    //    screen.pMax.x = frame;
    //    screen.pMin.y = -1.f;
    //    screen.pMax.y = 1.f;
    //} else {
        screen.pMin.x = -1.f;
        screen.pMax.x = 1.f;
        screen.pMin.y = -1.f / frame;
        screen.pMax.y = 1.f / frame;
    //}
    //int swi;
    //const Float *sw = params.FindFloat("screenwindow", &swi);
    //if (sw) {
    //    if (swi == 4) {
    //        screen.pMin.x = sw[0];
    //        screen.pMax.x = sw[1];
    //        screen.pMin.y = sw[2];
    //        screen.pMax.y = sw[3];
    //    } else
    //        Error("\"screenwindow\" should have four values");
    //}
    //Float fov = params.FindOneFloat("fov", 90.);
    //Float halffov = params.FindOneFloat("halffov", -1.f);
    //if (halffov > 0.f)
    //    // hack for structure synth, which exports half of the full fov
    //    fov = 2.f * halffov;
    return new PerspectiveCamera(cam2world, screen, /*shutteropen, shutterclose,
                                 lensradius, focaldistance,*/ fov, film/*, medium*/);
}

}  // namespace pbrt
