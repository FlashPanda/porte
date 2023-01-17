
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_EFLOAT_H
#define PORTE_CORE_EFLOAT_H

#include <core/porte.h>
#include <core/StringPrint.h>

namespace porte {

class EFloat {
public:
    EFloat() {}
    EFloat(float v, float err = 0.f) : v(v) {
        if (err == 0.)
            low = high = v;
        else {
            // 保守边界。可能过于保守。
            low = NextFloatDown(v - err);
            high = NextFloatUp(v + err);
        }

        // 保存高精度引用值。
        vPrecise = v;
        Check();
    }

    EFloat(float v, long double lD, float err) : EFloat(v, err) {
        vPrecise = lD;
        Check();
    }

    EFloat operator+(EFloat ef) const {
        EFloat r;
        r.v = v + ef.v;

        r.vPrecise = vPrecise + ef.vPrecise;

        r.low = NextFloatDown(LowerBound() + ef.LowerBound());
        r.high = NextFloatUp(UpperBound() + ef.UpperBound());
        r.Check();
        return r;
    }
    explicit operator float() const { return v; }
    explicit operator double() const { return v; }
    float GetAbsoluteError() const { return high - low; }
    float UpperBound() const { return high; }
    float LowerBound() const { return low; }

    float GetRelativeError() const {
        return std::abs((vPrecise - v) / vPrecise);
    }

    EFloat operator-(EFloat ef) const {
        EFloat r;
        r.v = v - ef.v;

        r.vPrecise = vPrecise - ef.vPrecise;

        r.low = NextFloatDown(LowerBound() - ef.UpperBound());
        r.high = NextFloatUp(UpperBound() - ef.LowerBound());
        r.Check();
        return r;
    }
    EFloat operator*(EFloat ef) const {
        EFloat r;
        r.v = v * ef.v;

        r.vPrecise = vPrecise * ef.vPrecise;

        Float prod[4] = {
            LowerBound() * ef.LowerBound(), UpperBound() * ef.LowerBound(),
            LowerBound() * ef.UpperBound(), UpperBound() * ef.UpperBound()};
        r.low = NextFloatDown(
            std::min(std::min(prod[0], prod[1]), std::min(prod[2], prod[3])));
        r.high = NextFloatUp(
            std::max(std::max(prod[0], prod[1]), std::max(prod[2], prod[3])));
        r.Check();
        return r;
    }
    EFloat operator/(EFloat ef) const {
        EFloat r;
        r.v = v / ef.v;

        r.vPrecise = vPrecise / ef.vPrecise;

        if (ef.low < 0 && ef.high > 0) {
            // 跨0错误
            r.low = -Infinity;
            r.high = Infinity;
        } else {
            Float div[4] = {
                LowerBound() / ef.LowerBound(), UpperBound() / ef.LowerBound(),
                LowerBound() / ef.UpperBound(), UpperBound() / ef.UpperBound()};
            r.low = NextFloatDown(
                std::min(std::min(div[0], div[1]), std::min(div[2], div[3])));
            r.high = NextFloatUp(
                std::max(std::max(div[0], div[1]), std::max(div[2], div[3])));
        }
        r.Check();
        return r;
    }
    EFloat operator-() const {
        EFloat r;
        r.v = -v;

        r.vPrecise = -vPrecise;

        r.low = -high;
        r.high = -low;
        r.Check();
        return r;
    }
    inline bool operator==(EFloat fe) const { return v == fe.v; }
    inline void Check() const {
        if (!std::isinf(low) && !std::isnan(low) && !std::isinf(high) &&
            !std::isnan(high))
            //CHECK_LE(low, high);

        if (!std::isinf(v) && !std::isnan(v)) {
            //CHECK_LE(LowerBound(), vPrecise);
            //CHECK_LE(vPrecise, UpperBound());
        }

    }
    EFloat(const EFloat &ef) {
        ef.Check();
        v = ef.v;
        low = ef.low;
        high = ef.high;

        vPrecise = ef.vPrecise;

    }
    EFloat &operator=(const EFloat &ef) {
        ef.Check();
        if (&ef != this) {
            v = ef.v;
            low = ef.low;
            high = ef.high;

            vPrecise = ef.vPrecise;

        }
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const EFloat &ef) {
        //os << StringPrintf("v=%f (%a) - [%f, %f]",
        //                   ef.v, ef.v, ef.low, ef.high);

        //os << StringPrintf(", precise=%.30Lf", ef.vPrecise);

        return os;
    }

  private:

    float v, low, high;

    long double vPrecise;

    friend inline EFloat sqrt(EFloat fe);
    friend inline EFloat abs(EFloat fe);
    friend inline bool Quadratic(EFloat A, EFloat B, EFloat C, EFloat *t0,
                                 EFloat *t1);
};


inline EFloat operator*(float f, EFloat fe) { return EFloat(f) * fe; }

inline EFloat operator/(float f, EFloat fe) { return EFloat(f) / fe; }

inline EFloat operator+(float f, EFloat fe) { return EFloat(f) + fe; }

inline EFloat operator-(float f, EFloat fe) { return EFloat(f) - fe; }

inline EFloat sqrt(EFloat fe) {
    EFloat r;
    r.v = std::sqrt(fe.v);

    r.vPrecise = std::sqrt(fe.vPrecise);

    r.low = NextFloatDown(std::sqrt(fe.low));
    r.high = NextFloatUp(std::sqrt(fe.high));
    r.Check();
    return r;
}

inline EFloat abs(EFloat fe) {
    if (fe.low >= 0)
        return fe;
    else if (fe.high <= 0) {
        EFloat r;
        r.v = -fe.v;
        r.vPrecise = -fe.vPrecise;
        r.low = -fe.high;
        r.high = -fe.low;
        r.Check();
        return r;
    } else {
       
        EFloat r;
        r.v = std::abs(fe.v);
        r.vPrecise = std::abs(fe.vPrecise);
        r.low = 0;
        r.high = std::max(-fe.low, fe.high);
        r.Check();
        return r;
    }
}

inline bool Quadratic(EFloat A, EFloat B, EFloat C, EFloat *t0, EFloat *t1);
inline bool Quadratic(EFloat A, EFloat B, EFloat C, EFloat *t0, EFloat *t1) {
    // 二次判别式
    double discrim = (double)B.v * (double)B.v - 4. * (double)A.v * (double)C.v;
    if (discrim < 0.) return false;
    double rootDiscrim = std::sqrt(discrim);

    EFloat floatRootDiscrim(rootDiscrim, MachineEpsilon * rootDiscrim);

    // 计算二次项t的值
    EFloat q;
    if ((float)B < 0)
        q = -.5 * (B - floatRootDiscrim);
    else
        q = -.5 * (B + floatRootDiscrim);
    *t0 = q / A;
    *t1 = C / q;
    if ((float)*t0 > (float)*t1) std::swap(*t0, *t1);
    return true;
}

}  // namespace porte

#endif  // PORTE_CORE_EFLOAT_H
