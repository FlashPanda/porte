#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_FILTER_H
#define PORTE_CORE_FILTER_H

// core/filter.h*
#include <core/porte.h>
#include <core/Geometry.h>

namespace porte {

class Filter {
  public:
    virtual ~Filter();
    Filter(const Vector2f &radius)
        : radius(radius), invRadius(Vector2f(1 / radius.x, 1 / radius.y)) {}
    virtual Float Evaluate(const Point2f &p) const = 0;

    const Vector2f radius, invRadius;
};

}  // namespace pbrt

#endif  // PORTE_CORE_FILTER_H
