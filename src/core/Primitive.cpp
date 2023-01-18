// core/primitive.cpp*
#include <core/Primitive.h>
#include <core/Light.h>
#include <core/Interaction.h>
//#include "stats.h"

namespace porte {

//STAT_MEMORY_COUNTER("Memory/Primitives", primitiveMemory);

// Primitive Method Definitions
Primitive::~Primitive() {}
const AreaLight *Aggregate::GetAreaLight() const {
    //LOG(FATAL) <<
    //    "Aggregate::GetAreaLight() method"
    //    "called; should have gone to GeometricPrimitive";
    return nullptr;
}

const Material *Aggregate::GetMaterial() const {
    //LOG(FATAL) <<
    //    "Aggregate::GetMaterial() method"
    //    "called; should have gone to GeometricPrimitive";
    return nullptr;
}

void Aggregate::ComputeScatteringFunctions(SurfaceInteraction *isect,
                                           MemoryArena &arena,
                                           TransportMode mode,
                                           bool allowMultipleLobes) const {
    //LOG(FATAL) <<
    //    "Aggregate::ComputeScatteringFunctions() method"
    //    "called; should have gone to GeometricPrimitive";
}

// TransformedPrimitive Method Definitions
TransformedPrimitive::TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
                                           const AnimatedTransform &PrimitiveToWorld)
    : primitive(primitive), PrimitiveToWorld(PrimitiveToWorld) {
    //primitiveMemory += sizeof(*this);
}

bool TransformedPrimitive::Intersect(const Ray &r,
                                     SurfaceInteraction *isect) const {
    // 计算转换后的ray
    Transform InterpolatedPrimToWorld;
    PrimitiveToWorld.Interpolate(r.time, &InterpolatedPrimToWorld);
    Ray ray = Inverse(InterpolatedPrimToWorld)(r);
    if (!primitive->Intersect(ray, isect)) return false;
    r.tMax = ray.tMax;
    // 相交的数据转换到世界坐标
    if (!InterpolatedPrimToWorld.IsIdentity())
        *isect = InterpolatedPrimToWorld(*isect);
    //CHECK_GE(Dot(isect->n, isect->shading.n), 0);
    return true;
}

bool TransformedPrimitive::IntersectP(const Ray &r) const {
    Transform InterpolatedPrimToWorld;
    PrimitiveToWorld.Interpolate(r.time, &InterpolatedPrimToWorld);
    Transform InterpolatedWorldToPrim = Inverse(InterpolatedPrimToWorld);
    return primitive->IntersectP(InterpolatedWorldToPrim(r));
}

GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                                       const std::shared_ptr<Material> &material/*,
                                       const std::shared_ptr<AreaLight>& areaLight,
                                       const MediumInterface &mediumInterface*/)
    : shape(shape),
    material(material)/*,
    areaLight(areaLight),
    mediumInterface(mediumInterface) */{
    //primitiveMemory += sizeof(*this);
}

Bounds3f GeometricPrimitive::WorldBound() const { return shape->WorldBound(); }

bool GeometricPrimitive::IntersectP(const Ray &r) const {
    return shape->IntersectP(r);
}

bool GeometricPrimitive::Intersect(const Ray &r,
                                   SurfaceInteraction *isect) const {
    Float tHit;
    if (!shape->Intersect(r, &tHit, isect)) return false;
    r.tMax = tHit;
    isect->primitive = this;
    //CHECK_GE(Dot(isect->n, isect->shading.n), 0.);

    //if (mediumInterface.IsMediumTransition())
    //    isect->mediumInterface = mediumInterface;
    //else
    //    isect->mediumInterface = MediumInterface(r.medium);
    return true;
}

const AreaLight *GeometricPrimitive::GetAreaLight() const {
    return areaLight.get();
}

const Material *GeometricPrimitive::GetMaterial() const {
    return material.get();
}

void GeometricPrimitive::ComputeScatteringFunctions(
    SurfaceInteraction *isect, MemoryArena &arena, TransportMode mode,
    bool allowMultipleLobes) const {
    //ProfilePhase p(Prof::ComputeScatteringFuncs);
    if (material)
        material->ComputeScatteringFunctions(isect, arena, mode,
                                             allowMultipleLobes);
    //CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
}

}  // namespace porte
