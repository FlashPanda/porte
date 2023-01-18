// materials/matte.cpp*
#include <materials/matte.h>
#include <core/Reflection.h>
#include <core/Interaction.h>
#include <core/Texture.h>
#include <core/Interaction.h>
#include <core/Spectrum.h>
#include <textures/Constant.h>

namespace porte {

void MatteMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
                                               MemoryArena &arena,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const {
    // 计算纹理，并生成BRDF
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum r = Kd->Evaluate(*si).Clamp();
    //Float sig = Clamp(sigma->Evaluate(*si), 0, 90);
    //if (!r.IsBlack()) {
    //    if (sig == 0)
            si->bsdf->Add(ARENA_ALLOC(arena, LambertianReflection)(r));
        //else
        //    si->bsdf->Add(ARENA_ALLOC(arena, OrenNayar)(r, sig));
    }

MatteMaterial *CreateMatteMaterial(Spectrum kd) {
    std::shared_ptr<Texture<Spectrum>> Kd =
        std::make_shared<ConstantTexture<Spectrum>>(kd);
    return new MatteMaterial(Kd);
}

}  // namespace pbrt
