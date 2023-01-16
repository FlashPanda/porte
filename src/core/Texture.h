#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_TEXTURE_H
#define PORTE_CORE_TEXTURE_H

// core/texture.h*
#include <core/porte.h>
#include <core/Spectrum.h>
#include <core/Geometry.h>
#include <core/Transform.h>
#include <core/Memory.h>

namespace porte {

class TextureMapping2D {
  public:
    virtual ~TextureMapping2D();
    virtual Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
                        Vector2f *dstdy) const = 0;
};

class UVMapping2D : public TextureMapping2D {
  public:
    UVMapping2D(Float su = 1, Float sv = 1, Float du = 0, Float dv = 0);
    Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
                Vector2f *dstdy) const;

  private:
    const Float su, sv, du, dv;
};

class SphericalMapping2D : public TextureMapping2D {
  public:
    SphericalMapping2D(const Transform &WorldToTexture)
        : WorldToTexture(WorldToTexture) {}
    Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
                Vector2f *dstdy) const;

  private:
    Point2f sphere(const Point3f &P) const;
    const Transform WorldToTexture;
};

class CylindricalMapping2D : public TextureMapping2D {
  public:
    CylindricalMapping2D(const Transform &WorldToTexture)
        : WorldToTexture(WorldToTexture) {}
    Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
                Vector2f *dstdy) const;

  private:
    Point2f cylinder(const Point3f &p) const {
        Vector3f vec = Normalize(WorldToTexture(p) - Point3f(0, 0, 0));
        return Point2f((Pi + std::atan2(vec.y, vec.x)) * Inv2Pi, vec.z);
    }
    const Transform WorldToTexture;
};

class PlanarMapping2D : public TextureMapping2D {
  public:
    Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
                Vector2f *dstdy) const;
    PlanarMapping2D(const Vector3f &vs, const Vector3f &vt, Float ds = 0,
                    Float dt = 0)
        : vs(vs), vt(vt), ds(ds), dt(dt) {}

  private:
    const Vector3f vs, vt;
    const Float ds, dt;
};

class TextureMapping3D {
  public:
    virtual ~TextureMapping3D();
    virtual Point3f Map(const SurfaceInteraction &si, Vector3f *dpdx,
                        Vector3f *dpdy) const = 0;
};

class IdentityMapping3D : public TextureMapping3D {
  public:
    IdentityMapping3D(const Transform &WorldToTexture)
        : WorldToTexture(WorldToTexture) {}
    Point3f Map(const SurfaceInteraction &si, Vector3f *dpdx,
                Vector3f *dpdy) const;

  private:
    const Transform WorldToTexture;
};

template <typename T>
class Texture {
  public:
    virtual T Evaluate(const SurfaceInteraction &) const = 0;
    virtual ~Texture() {}
};

Float Lanczos(Float, Float tau = 2);
Float Noise(Float x, Float y = .5f, Float z = .5f);
Float Noise(const Point3f &p);
Float FBm(const Point3f &p, const Vector3f &dpdx, const Vector3f &dpdy,
          Float omega, int octaves);
Float Turbulence(const Point3f &p, const Vector3f &dpdx, const Vector3f &dpdy,
                 Float omega, int octaves);

}  // namespace porte

#endif  // PORTE_CORE_TEXTURE_H
