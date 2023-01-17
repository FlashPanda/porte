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

    // ��shape�ı������һ���㣬��������ڱ������PDF
    virtual Interaction Sample(const Point2f &u, Float *pdf) const = 0;
    virtual Float Pdf(const Interaction &) const { return 1 / Area(); }

    // ��һ�����õ�ref����shape�ϲ���һ���㡣���ع���ref�������PDF��
    virtual Interaction Sample(const Interaction &ref, const Point2f &u,
                               Float *pdf) const;
    virtual Float Pdf(const Interaction &ref, const Vector3f &wi) const;

    // ����p����shape��չ��������ǡ�Ĭ�ϵ�ʵ����ʹ�����ؿ�����֡�
    // nSamples�����������������������˶���������
    virtual Float SolidAngle(const Point3f &p, int nSamples = 512) const;

    const Transform *ObjectToWorld, *WorldToObject;
    const bool reverseOrientation;
    const bool transformSwapsHandedness;
};

}  // namespace porte

#endif  // PORTE_CORE_SHAPE_H
