// core/light.cpp*
#include <core/Light.h>
#include <core/Scene.h>
#include <core/Sampling.h>
#include <core/Spectrum.h>
#include <core/Primitive.h>
//#include "stats.h"
//#include "paramset.h"

namespace porte {

//STAT_COUNTER("Scene/Lights", numLights);
//STAT_COUNTER("Scene/AreaLights", numAreaLights);

// Light Method Definitions
Light::Light(int flags, const Transform &LightToWorld,
             int nSamples)
    : flags(flags),
      nSamples(std::max(1, nSamples)),
      //mediumInterface(mediumInterface),
      LightToWorld(LightToWorld),
      WorldToLight(Inverse(LightToWorld)) {
    //++numLights;
}

Light::~Light() {}

bool VisibilityTester::Unoccluded(const Scene &scene) const {
    return !scene.IntersectP(p0.SpawnRayTo(p1));
}

Spectrum VisibilityTester::Tr(const Scene &scene, Sampler &sampler) const {
    Ray ray(p0.SpawnRayTo(p1));
    Spectrum Tr(1.f);
    while (true) {
        SurfaceInteraction isect;
        bool hitSurface = scene.Intersect(ray, &isect);
        // 处理不透明表面，沿着光线路径
        if (hitSurface && isect.primitive->GetMaterial() != nullptr)
            return Spectrum(0.0f);


        //if (ray.medium) Tr *= ray.medium->Tr(ray, sampler);

        if (!hitSurface) break;
        //ray = isect.SpawnRayTo(p1);
    }
    return Tr;
}

Spectrum Light::Le(const RayDifferential &ray) const { return Spectrum(0.f); }

AreaLight::AreaLight(const Transform &LightToWorld, const MediumInterface &medium,
                     int nSamples)
    : Light((int)LightFlags::Area, LightToWorld, /*medium,*/ nSamples) {
    //++numAreaLights;
}

}  // namespace porte
