// core/shape.cpp*
#include <core/Shape.h>
//#include "stats.h"
//#include "lowdiscrepancy.h"

namespace porte {

// Shape Method Definitions
Shape::~Shape() {}

//STAT_COUNTER("Scene/Shapes created", nShapesCreated);
Shape::Shape(const Transform *ObjectToWorld, const Transform *WorldToObject,
             bool reverseOrientation)
    : ObjectToWorld(ObjectToWorld),
      WorldToObject(WorldToObject),
      reverseOrientation(reverseOrientation),
      transformSwapsHandedness(ObjectToWorld->SwapsHandedness()) {
    //++nShapesCreated;
}

Bounds3f Shape::WorldBound() const { return (*ObjectToWorld)(ObjectBound()); }

Interaction Shape::Sample(const Interaction &ref, const Point2f &u,
                          Float *pdf) const {
    Interaction intr = Sample(u, pdf);
    Vector3f wi = intr.p - ref.p;
    if (wi.LengthSquared() == 0)
        *pdf = 0;
    else {
        wi = Normalize(wi);
        // �����ת��������ǵĶ���
        *pdf *= DistanceSquared(ref.p, intr.p) / AbsDot(intr.n, -wi);
        if (std::isinf(*pdf)) *pdf = 0.f;
    }
    return intr;
}

Float Shape::Pdf(const Interaction &ref, const Vector3f &wi) const {
    // �������ߺ����Դ�������ཻ
    Ray ray = ref.SpawnRay(wi);
    Float tHit;
    SurfaceInteraction isectLight;
    // ���������޼�shape��alpha�������Ǵ���San Miguel�������ķǳ����ֶΣ��ⷽ��
    // ��������һ�����ɼ������Դ��
    if (!Intersect(ray, &tHit, &isectLight, false)) return 0;

    // �ѹ�Դ����Ȩ��ת��������Ƕ���
    Float pdf = DistanceSquared(ref.p, isectLight.p) /
                (AbsDot(isectLight.n, -wi) * Area());
    if (std::isinf(pdf)) pdf = 0.f;
    return pdf;
}

Float Shape::SolidAngle(const Point3f &p, int nSamples) const {
    Interaction ref(p, Normal3f(), Vector3f(), Vector3f(0, 0, 1), 0,
                    MediumInterface{});
    double solidAngle = 0;
    for (int i = 0; i < nSamples; ++i) {
        Point2f u{RadicalInverse(0, i), RadicalInverse(1, i)};
        Float pdf;
        Interaction pShape = Sample(ref, u, &pdf);
        if (pdf > 0 && !IntersectP(Ray(p, pShape.p - p, .999f))) {
            solidAngle += 1 / pdf;
        }
    }
    return solidAngle / nSamples;
}

}  // namespace porte
