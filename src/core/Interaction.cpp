// core/interaction.cpp*
#include <core/Interaction.h>
#include <core/Transform.h>
#include <core/Primitive.h>
#include <core/Shape.h>
#include <core/Light.h>
#include <core/Spectrum.h>

namespace porte {

SurfaceInteraction::SurfaceInteraction(
    const Point3f &p, const Vector3f &pError, const Point2f &uv,
    const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
    const Normal3f &dndu, const Normal3f &dndv, Float time, const Shape *shape,
    int faceIndex)
    : Interaction(p, Normal3f(Normalize(Cross(dpdu, dpdv))), pError, wo, time),
      uv(uv),
      dpdu(dpdu),
      dpdv(dpdv),
      dndu(dndu),
      dndv(dndv),
      shape(shape),
      faceIndex(faceIndex) {
    shading.n = n;
    shading.dpdu = dpdu;
    shading.dpdv = dpdv;
    shading.dndu = dndu;
    shading.dndv = dndv;

    if (shape &&
        (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        n *= -1;
        shading.n *= -1;
    }
}

void SurfaceInteraction::SetShadingGeometry(const Vector3f &dpdus,
                                            const Vector3f &dpdvs,
                                            const Normal3f &dndus,
                                            const Normal3f &dndvs,
                                            bool orientationIsAuthoritative) {
    // 计算着色法线
    shading.n = Normalize((Normal3f)Cross(dpdus, dpdvs));
    if (shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness))
        shading.n = -shading.n;
    if (orientationIsAuthoritative)
        n = Faceforward(n, shading.n);
    else
        shading.n = Faceforward(shading.n, n);

    // 计算着色偏导
    shading.dpdu = dpdus;
    shading.dpdv = dpdvs;
    shading.dndu = dndus;
    shading.dndv = dndvs;
}

void SurfaceInteraction::ComputeScatteringFunctions(const RayDifferential &ray,
                                                    MemoryArena &arena,
                                                    bool allowMultipleLobes,
                                                    TransportMode mode) {
    ComputeDifferentials(ray);
    primitive->ComputeScatteringFunctions(this, arena, mode,
                                          allowMultipleLobes);
}

void SurfaceInteraction::ComputeDifferentials(
    const RayDifferential &ray) const {
    if (ray.hasDifferentials) {
        // 估计pt和（u，v）中的屏幕空间变化

        // 计算辅助交点
        Float d = Dot(n, Vector3f(p.x, p.y, p.z));
        Float tx =
            -(Dot(n, Vector3f(ray.rxOrigin)) - d) / Dot(n, ray.rxDirection);
        if (std::isinf(tx) || std::isnan(tx)) goto fail;
        Point3f px = ray.rxOrigin + tx * ray.rxDirection;
        Float ty =
            -(Dot(n, Vector3f(ray.ryOrigin)) - d) / Dot(n, ray.ryDirection);
        if (std::isinf(ty) || std::isnan(ty)) goto fail;
        Point3f py = ray.ryOrigin + ty * ray.ryDirection;
        dpdx = px - p;
        dpdy = py - p;

        // 计算辅助点的uv偏移量

        // 选择用于光线偏移计算的两个维度
        int dim[2];
        if (std::abs(n.x) > std::abs(n.y) && std::abs(n.x) > std::abs(n.z)) {
            dim[0] = 1;
            dim[1] = 2;
        } else if (std::abs(n.y) > std::abs(n.z)) {
            dim[0] = 0;
            dim[1] = 2;
        } else {
            dim[0] = 0;
            dim[1] = 1;
        }

        // 偏移计算
        Float A[2][2] = {{dpdu[dim[0]], dpdv[dim[0]]},
                         {dpdu[dim[1]], dpdv[dim[1]]}};
        Float Bx[2] = {px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]]};
        Float By[2] = {py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]};
        if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx)) dudx = dvdx = 0;
        if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy)) dudy = dvdy = 0;
    } else {
    fail:
        dudx = dvdx = 0;
        dudy = dvdy = 0;
        dpdx = dpdy = Vector3f(0, 0, 0);
    }
}

Spectrum SurfaceInteraction::Le(const Vector3f &w) const {
    //const AreaLight *area = primitive->GetAreaLight();
    //return area ? area->L(*this, w) : Spectrum(0.f);
    return Spectrum(0.f);
}

}  // namespace porte
