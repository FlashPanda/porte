
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_SAMPLER_H
#define PORTE_CORE_SAMPLER_H

// core/sampler.h*
#include <core/porte.h>
#include <core/Geometry.h>
#include <core/RNG.h>
#include <inttypes.h>

namespace porte {


class Sampler {
public:
    virtual ~Sampler();
    Sampler(int64_t samplesPerPixel);
    virtual void StartPixel(const Point2i &p);
    virtual Float Get1D() = 0;
    virtual Point2f Get2D() = 0;
    CameraSample GetCameraSample(const Point2i &pRaster);
    void Request1DArray(int n);
    void Request2DArray(int n);
    virtual int RoundCount(int n) const { return n; }
    const Float *Get1DArray(int n);
    const Point2f *Get2DArray(int n);
    virtual bool StartNextSample();
    virtual std::unique_ptr<Sampler> Clone(int seed) = 0;
    virtual bool SetSampleNumber(int64_t sampleNum);
    std::string StateString() const {
        return std::string();
		//return StringPrintf("(%d,%d), sample %" PRId64, currentPixel.x,
		//	currentPixel.y, currentPixelSampleIndex);
    }
    int64_t CurrentSampleNumber() const { return currentPixelSampleIndex; }

    const int64_t samplesPerPixel;

  protected:
    Point2i currentPixel;
    int64_t currentPixelSampleIndex;
    std::vector<int> samples1DArraySizes, samples2DArraySizes;
    std::vector<std::vector<Float>> sampleArray1D;
    std::vector<std::vector<Point2f>> sampleArray2D;

  private:
    size_t array1DOffset, array2DOffset;
};

class PixelSampler : public Sampler {
  public:
    PixelSampler(int64_t samplesPerPixel, int nSampledDimensions);
    bool StartNextSample();
    bool SetSampleNumber(int64_t);
    Float Get1D();
    Point2f Get2D();

  protected:
    std::vector<std::vector<Float>> samples1D;
    std::vector<std::vector<Point2f>> samples2D;
    int current1DDimension = 0, current2DDimension = 0;
    RNG rng;
};

class GlobalSampler : public Sampler {
  public:
    bool StartNextSample();
    void StartPixel(const Point2i &);
    bool SetSampleNumber(int64_t sampleNum);
    Float Get1D();
    Point2f Get2D();
    GlobalSampler(int64_t samplesPerPixel) : Sampler(samplesPerPixel) {}
    virtual int64_t GetIndexForSample(int64_t sampleNum) const = 0;
    virtual Float SampleDimension(int64_t index, int dimension) const = 0;

  private:
    int dimension;
    int64_t intervalSampleIndex;
    static const int arrayStartDim = 5;
    int arrayEndDim;
};

}  // namespace porte

#endif  // PORTE_CORE_SAMPLER_H
