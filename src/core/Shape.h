#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_SHAPE_H
#define PORTE_CORE_SHAPE_H

// core/shape.h*
#include <core/porte.h>
#include <core/Geometry.h>
#include <core/Interaction.h>
#include <core/Memory.h>
#include <core/Transform.h>

namespace porte {

class Shape {
  public:
    Shape(const Transform *ObjectToWorld, const Transform *WorldToObject,
          bool reverseOrientation);
    virtual ~Shape();
    virtual Bounds3f ObjectBound() const = 0;
    virtual Bounds3f WorldBound() const;
    virtual bool Intersect(const Ray &ray, Float *tHit,
                           SurfaceInteraction *isect,
                           bool testAlphaTexture = true) const = 0;
    virtual bool IntersectP(const Ray &ray,
                            bool testAlphaTexture = true) const {
        return Intersect(ray, nullptr, nullptr, testAlphaTexture);
    }
    virtual Float Area() const = 0;

    // 在shape的表面采样一个点，返回相对于表面积的PDF
    virtual Interaction Sample(const Point2f &u, Float *pdf) const = 0;
    virtual Float Pdf(const Interaction &) const { return 1 / Area(); }

    // 给一个引用点ref，在shape上采样一个点。返回关于ref的立体角PDF。
    virtual Interaction Sample(const Interaction &ref, const Point2f &u,
                               Float *pdf) const;
    virtual Float Pdf(const Interaction &ref, const Vector3f &wi) const;

    // 返回p点在shape上展开的立体角。默认的实现是使用蒙特卡洛积分。
    // nSamples参数定义了在这个情况下用了多少样本。
    virtual Float SolidAngle(const Point3f &p, int nSamples = 512) const;

    const Transform *ObjectToWorld, *WorldToObject;
    const bool reverseOrientation;
    const bool transformSwapsHandedness;
};

}  // namespace porte

#endif  // PORTE_CORE_SHAPE_H
