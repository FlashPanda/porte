#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_CONSTANT_H
#define PBRT_TEXTURES_CONSTANT_H

// textures/constant.h*
#include <core/porte.h>
#include <core/Texture.h>
//#include "paramset.h"

namespace porte {

template <typename T>
class ConstantTexture : public Texture<T> {
public:
    ConstantTexture(const T &value) : value(value) {}
    T Evaluate(const SurfaceInteraction &) const { return value; }

private:
    T value;
};

ConstantTexture<Float> *CreateConstantFloatTexture(const Transform &tex2world, Float kd);
ConstantTexture<Spectrum> *CreateConstantSpectrumTexture(const Transform &tex2world, Spectrum sp);

}  // namespace porte

#endif  // PORTE_TEXTURES_CONSTANT_H
