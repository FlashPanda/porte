// textures/constant.cpp*
#include "textures/constant.h"

namespace porte {

ConstantTexture<Float> *CreateConstantFloatTexture(const Transform &tex2world, Float kd) {
    return new ConstantTexture<Float>(1.f);
}

ConstantTexture<Spectrum> *CreateConstantSpectrumTexture(
    const Transform &tex2world, Spectrum sp) {
    return new ConstantTexture<Spectrum>(sp);
}

}  // namespace porte
