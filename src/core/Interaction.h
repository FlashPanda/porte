#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_INTERACTION_H
#define PORTE_CORE_INTERACTION_H

// core/interaction.h*
#include <core/porte.h>
#include <core/Geometry.h>
#include <core/Transform.h>
//#include "medium.h"
#include <core/Material.h>

namespace porte {

struct Interaction {
    Interaction() : time(0) {}
    Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError,
                const Vector3f &wo, Float time/*,
                const MediumInterface &mediumInterface*/)
        : p(p),
          time(time),
          pError(pError),
          wo(Normalize(wo)),
          n(n)/*,
          mediumInterface(mediumInterface)*/ {}

    bool IsSurfaceInteraction() const { return n != Normal3f(); }

    Ray SpawnRay(const Vector3f &d) const {
        Point3f o = OffsetRayOrigin(p, pError, n, d);
        return Ray(o, d, Infinity, time/*, GetMedium(d)*/);
    }

    Ray SpawnRayTo(const Point3f &p2) const {
        Point3f origin = OffsetRayOrigin(p, pError, n, p2 - p);
        Vector3f d = p2 - p;
        return Ray(origin, d, 1 - ShadowEpsilon, time/*, GetMedium(d)*/);
    }

    Ray SpawnRayTo(const Interaction &it) const {
        Point3f origin = OffsetRayOrigin(p, pError, n, it.p - p);
        Point3f target = OffsetRayOrigin(it.p, it.pError, it.n, origin - it.p);
        Vector3f d = target - origin;
        return Ray(origin, d, 1 - ShadowEpsilon, time/*, GetMedium(d)*/);
    }

    Interaction(const Point3f &p, const Vector3f &wo, Float time/*,
                const MediumInterface &mediumInterface*/)
        : p(p), time(time), wo(wo)/*, mediumInterface(mediumInterface)*/ {}

    Interaction(const Point3f &p, Float time/*,
                const MediumInterface &mediumInterface*/)
        : p(p), time(time)/*, mediumInterface(mediumInterface)*/ {}

    bool IsMediumInteraction() const { return !IsSurfaceInteraction(); }

    //const Medium *GetMedium(const Vector3f &w) const {
    //    return Dot(w, n) > 0 ? mediumInterface.outside : mediumInterface.inside;
    //}
    //const Medium *GetMedium() const {
    //    CHECK_EQ(mediumInterface.inside, mediumInterface.outside);
    //    return mediumInterface.inside;
    //}

    Point3f p;
    Float time;
    Vector3f pError;
    Vector3f wo;
    Normal3f n;
    //MediumInterface mediumInterface;
};

//class MediumInteraction : public Interaction {
//public:
//    MediumInteraction() : phase(nullptr) {}
//    MediumInteraction(const Point3f &p, const Vector3f &wo, Float time,
//                      const Medium *medium, const PhaseFunction *phase)
//        : Interaction(p, wo, time, medium), phase(phase) {}
//    bool IsValid() const { return phase != nullptr; }
//
//    const PhaseFunction *phase;
//};


class SurfaceInteraction : public Interaction {
public:
    SurfaceInteraction() {}
    SurfaceInteraction(const Point3f &p, const Vector3f &pError,
                       const Point2f &uv, const Vector3f &wo,
                       const Vector3f &dpdu, const Vector3f &dpdv,
                       const Normal3f &dndu, const Normal3f &dndv, Float time,
                       const Shape *sh,
                       int faceIndex = 0);
    void SetShadingGeometry(const Vector3f &dpdu, const Vector3f &dpdv,
                            const Normal3f &dndu, const Normal3f &dndv,
                            bool orientationIsAuthoritative);
    void ComputeScatteringFunctions(
        const RayDifferential &ray, MemoryArena &arena,
        bool allowMultipleLobes = false,
        TransportMode mode = TransportMode::Radiance);
    void ComputeDifferentials(const RayDifferential &r) const;
    Spectrum Le(const Vector3f &w) const;

    Point2f uv;
    Vector3f dpdu, dpdv;
    Normal3f dndu, dndv;
    const Shape *shape = nullptr;
    struct {
        Normal3f n;
        Vector3f dpdu, dpdv;
        Normal3f dndu, dndv;
    } shading;
    const Primitive *primitive = nullptr;
    BSDF *bsdf = nullptr;
    BSSRDF *bssrdf = nullptr;
    mutable Vector3f dpdx, dpdy;
    mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

    int faceIndex = 0;
};

}  // namespace porte

#endif  // PORTE_CORE_INTERACTION_H
