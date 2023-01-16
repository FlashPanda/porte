
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_INTEGRATOR_H
#define PORTE_CORE_INTEGRATOR_H


//#include <core/Primitive.h>
//#include <core/Spectrum.h>
//#include <core/Light.h>
//#include <core/Reflection.h>
//#include <core/Sampler.h>
//#include <core/Material.h>

#include <core/porte.h>
#include <core/Geometry.h>

namespace porte {

class Integrator {
  public:
    virtual ~Integrator();
    virtual void Render(const Scene &scene) = 0;
};

class SamplerIntegrator : public Integrator {
  public:
    SamplerIntegrator(std::shared_ptr<const Camera> camera,
                      std::shared_ptr<Sampler> sampler,
                      const Bounds2i &pixelBounds)
        : camera(camera), sampler(sampler), pixelBounds(pixelBounds) {}
    virtual void Preprocess(const Scene &scene, Sampler &sampler) {}
    void Render(const Scene &scene);
    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                        Sampler &sampler, MemoryArena &arena,
                        int depth = 0) const = 0;
    Spectrum SpecularReflect(const RayDifferential &ray,
                             const SurfaceInteraction &isect,
                             const Scene &scene, Sampler &sampler,
                             MemoryArena &arena, int depth) const;
    Spectrum SpecularTransmit(const RayDifferential &ray,
                              const SurfaceInteraction &isect,
                              const Scene &scene, Sampler &sampler,
                              MemoryArena &arena, int depth) const;

  protected:
    std::shared_ptr<const Camera> camera;

  private:
    std::shared_ptr<Sampler> sampler;
    const Bounds2i pixelBounds;
};

}  // namespace porte

#endif  // PORTE_CORE_INTEGRATOR_H
