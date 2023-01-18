#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_MATERIALS_MATTE_H
#define PORTE_MATERIALS_MATTE_H

// materials/Matte.h*
#include <core/porte.h>
#include <core/Material.h>

namespace porte {

class MatteMaterial : public Material {
  public:
    MatteMaterial(const std::shared_ptr<Texture<Spectrum>> &Kd)
        : Kd(Kd){}
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

  private:
    std::shared_ptr<Texture<Spectrum>> Kd;
};

MatteMaterial* CreateMatteMaterial(Float kd);

}  // namespace porte

#endif  // PORTE_MATERIALS_MATTE_H
