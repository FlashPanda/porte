#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_MATERIAL_H
#define PORTE_CORE_MATERIAL_H

// core/material.h*
#include <core/porte.h>
#include <core/Memory.h>

namespace porte {

// 传输模式，radiance还是inportance
enum class TransportMode { Radiance, Importance };

class Material {
  public:
    virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
    virtual ~Material();
    static void Bump(const std::shared_ptr<Texture<Float>> &d,
                     SurfaceInteraction *si);
};

}  // namespace porte

#endif  // PORTE_CORE_MATERIAL_H
