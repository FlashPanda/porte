// core/material.cpp*
#include <core/Material.h>
#include <core/Primitive.h>
#include <core/Texture.h>
#include <core/Spectrum.h>
#include <core/Reflection.h>

namespace porte {

Material::~Material() {}

void Material::Bump(const std::shared_ptr<Texture<Float>> &d,
                    SurfaceInteraction *si) {
    // todo
}

}  // namespace porte
