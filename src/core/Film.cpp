// core/film.cpp*
#include <core/film.h>
#include <ext/lodepng.h>
//#include "imageio.h"
//#include "stats.h"

namespace porte {

//STAT_MEMORY_COUNTER("Memory/Film pixels", filmPixelMemory);

Film::Film(const Point2i &resolution, const Bounds2f &cropWindow,
           std::unique_ptr<Filter> filt, Float diagonal,
           const std::string &filename, Float scale, Float maxSampleLuminance)
    : fullResolution(resolution),
      diagonal(diagonal * .001),
      filter(std::move(filt)),
      filename(filename),
      scale(scale),
      maxSampleLuminance(maxSampleLuminance) {
    // 计算图像边界
    croppedPixelBounds =
        Bounds2i(Point2i(std::ceil(fullResolution.x * cropWindow.pMin.x),
                         std::ceil(fullResolution.y * cropWindow.pMin.y)),
                 Point2i(std::ceil(fullResolution.x * cropWindow.pMax.x),
                         std::ceil(fullResolution.y * cropWindow.pMax.y)));
    //LOG(INFO) << "Created film with full resolution " << resolution <<
    //    ". Crop window of " << cropWindow << " -> croppedPixelBounds " <<
    //    croppedPixelBounds;

    // Allocate film image storage
    pixels = std::unique_ptr<Pixel[]>(new Pixel[croppedPixelBounds.Area()]);
    //filmPixelMemory += croppedPixelBounds.Area() * sizeof(Pixel);

    // Precompute filter weight table
    int offset = 0;
    for (int y = 0; y < filterTableWidth; ++y) {
        for (int x = 0; x < filterTableWidth; ++x, ++offset) {
            Point2f p;
            p.x = (x + 0.5f) * filter->radius.x / filterTableWidth;
            p.y = (y + 0.5f) * filter->radius.y / filterTableWidth;
            filterTable[offset] = filter->Evaluate(p);
        }
    }
}

Bounds2i Film::GetSampleBounds() const {
    Bounds2f floatBounds(Floor(Point2f(croppedPixelBounds.pMin) +
                               Vector2f(0.5f, 0.5f) - filter->radius),
                         Ceil(Point2f(croppedPixelBounds.pMax) -
                              Vector2f(0.5f, 0.5f) + filter->radius));
    return (Bounds2i)floatBounds;
}

Bounds2f Film::GetPhysicalExtent() const {
    Float aspect = (Float)fullResolution.y / (Float)fullResolution.x;
    Float x = std::sqrt(diagonal * diagonal / (1 + aspect * aspect));
    Float y = aspect * x;
    return Bounds2f(Point2f(-x / 2, -y / 2), Point2f(x / 2, y / 2));
}

std::unique_ptr<FilmTile> Film::GetFilmTile(const Bounds2i &sampleBounds) {
    // Bound image pixels that samples in _sampleBounds_ contribute to
    Vector2f halfPixel = Vector2f(0.5f, 0.5f);
    Bounds2f floatBounds = (Bounds2f)sampleBounds;
    Point2i p0 = (Point2i)Ceil(floatBounds.pMin - halfPixel - filter->radius);
    Point2i p1 = (Point2i)Floor(floatBounds.pMax - halfPixel + filter->radius) +
                 Point2i(1, 1);
    Bounds2i tilePixelBounds = Intersect(Bounds2i(p0, p1), croppedPixelBounds);
    return std::unique_ptr<FilmTile>(new FilmTile(
        tilePixelBounds, filter->radius, filterTable, filterTableWidth,
        maxSampleLuminance));
}

void Film::Clear() {
    for (Point2i p : croppedPixelBounds) {
        Pixel &pixel = GetPixel(p);
        for (int c = 0; c < 3; ++c)
            pixel.splatXYZ[c] = pixel.xyz[c] = 0;
        pixel.filterWeightSum = 0;
    }
}

void Film::MergeFilmTile(std::unique_ptr<FilmTile> tile) {
    //ProfilePhase p(Prof::MergeFilmTile);
    //VLOG(1) << "Merging film tile " << tile->pixelBounds;
    std::lock_guard<std::mutex> lock(mutex);
    for (Point2i pixel : tile->GetPixelBounds()) {
        // 将pixel合并到Film::pixel里
        const FilmTilePixel &tilePixel = tile->GetPixel(pixel);
        Pixel &mergePixel = GetPixel(pixel);
        Float xyz[3];
        tilePixel.contribSum.ToXYZ(xyz);
        for (int i = 0; i < 3; ++i) mergePixel.xyz[i] += xyz[i];
        mergePixel.filterWeightSum += tilePixel.filterWeightSum;
    }
}

void Film::SetImage(const Spectrum *img) const {
    int nPixels = croppedPixelBounds.Area();
    for (int i = 0; i < nPixels; ++i) {
        Pixel &p = pixels[i];
        img[i].ToXYZ(p.xyz);
        p.filterWeightSum = 1;
        p.splatXYZ[0] = p.splatXYZ[1] = p.splatXYZ[2] = 0;
    }
}

void Film::AddSplat(const Point2f &p, Spectrum v) {
    //ProfilePhase pp(Prof::SplatFilm);

    //if (v.HasNaNs()) {
    //    LOG(ERROR) << StringPrintf("Ignoring splatted spectrum with NaN values "
    //                               "at (%f, %f)", p.x, p.y);
    //    return;
    //} else if (v.y() < 0.) {
    //    LOG(ERROR) << StringPrintf("Ignoring splatted spectrum with negative "
    //                               "luminance %f at (%f, %f)", v.y(), p.x, p.y);
    //    return;
    //} else if (std::isinf(v.y())) {
    //    LOG(ERROR) << StringPrintf("Ignoring splatted spectrum with infinite "
    //                               "luminance at (%f, %f)", p.x, p.y);
    //    return;
    //}

    Point2i pi = Point2i(Floor(p));
    if (!InsideExclusive(pi, croppedPixelBounds)) return;
    if (v.y() > maxSampleLuminance)
        v *= maxSampleLuminance / v.y();
    Float xyz[3];
    v.ToXYZ(xyz);
    Pixel &pixel = GetPixel(pi);
    for (int i = 0; i < 3; ++i) pixel.splatXYZ[i].Add(xyz[i]);
}

void Film::WriteImage(std::string filename, Float splatScale)
{
    // 把图片转换成RGB，计算最终的像素值
    //LOG(INFO) <<
    //    "Converting image to RGB and computing final weighted pixel values";
    std::unique_ptr<Float[]> rgb(new Float[3 * croppedPixelBounds.Area()]);
    int offset = 0;
    for (Point2i p : croppedPixelBounds) {
        // 将XYZ颜色转换成RGB颜色
        Pixel &pixel = GetPixel(p);
        XYZToRGB(pixel.xyz, &rgb[3 * offset]);

        // 根据权重和来标准化像素
        Float filterWeightSum = pixel.filterWeightSum;
        if (filterWeightSum != 0) {
            Float invWt = (Float)1 / filterWeightSum;
            rgb[3 * offset] = std::max((Float)0, rgb[3 * offset] * invWt);
            rgb[3 * offset + 1] =
                std::max((Float)0, rgb[3 * offset + 1] * invWt);
            rgb[3 * offset + 2] =
                std::max((Float)0, rgb[3 * offset + 2] * invWt);
        }

        // 添加splat值
        Float splatRGB[3];
        Float splatXYZ[3] = {pixel.splatXYZ[0], pixel.splatXYZ[1],
                             pixel.splatXYZ[2]};
        XYZToRGB(splatXYZ, splatRGB);
        rgb[3 * offset] += splatScale * splatRGB[0];
        rgb[3 * offset + 1] += splatScale * splatRGB[1];
        rgb[3 * offset + 2] += splatScale * splatRGB[2];

        // 像素值缩放
        rgb[3 * offset] *= scale;
        rgb[3 * offset + 1] *= scale;
        rgb[3 * offset + 2] *= scale;
        ++offset;
    }

    
    //LOG(INFO) << "Writing image " << filename << " with bounds " <<
    //    croppedPixelBounds;
    //pbrt::WriteImage(filename, &rgb[0], croppedPixelBounds, fullResolution);

	// 8-bit formats; apply gamma
	Vector2i resolution = croppedPixelBounds.Diagonal();
	std::unique_ptr<uint8_t[]> rgb8(
		new uint8_t[3 * resolution.x * resolution.y]);
	uint8_t* dst = rgb8.get();
	for (int y = 0; y < resolution.y; ++y) {
		for (int x = 0; x < resolution.x; ++x) {
#define TO_BYTE(v) (uint8_t) Clamp(255.f * GammaCorrect(v) + 0.5f, 0.f, 255.f)
			dst[0] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 0]);
			dst[1] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 1]);
			dst[2] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 2]);
#undef TO_BYTE
			dst += 3;
		}
	}

	unsigned int error = lodepng_encode24_file(
        filename.c_str(), rgb8.get(), resolution.x, resolution.y);
    if (error != 0)
    {
        std::cout << "Error writing PNG \""
            << filename.c_str() << "\": " << lodepng_error_text(error) << std::endl;
    }
}

Film *CreateFilm(Vector2i res, std::unique_ptr<Filter> filter) {
    int xres = res.x;
    int yres = res.y;

    Bounds2f crop;
    crop.pMin.x = 0.f;
    crop.pMax.x = 1.f;
    crop.pMin.y = 0.f;
    crop.pMax.y = 1.f;

    return new Film(Point2i(xres, yres), crop, std::move(filter), 35.f,
        "filename", 1, Infinity);
}

}  // namespace porte
