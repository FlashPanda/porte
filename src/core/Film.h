#pragma once
#include <memory>
#include <core/MathUtil.h>
#include <core/Filter.h>

namespace porte
{
	inline void XYZToRGB(const float xyz[3], float rgb[3]);
	inline void RGBToXYZ(const float rgb[3], float xyz[3]);

	struct FilmTilePixel {
		Vector3f contribSum = 0.f;
		float filterWeightSum = 0.f;
	};

	struct Pixel
	{
		Pixel() { xyz[0] = xyz[1] = xyz[2] = filterWeightSum = 0.f; }
		float xyz[3] = { 0 };
		float filterWeightSum = 0.f;
	};

	class Film
	{
	public:
		Film(const Vector2i& resolution, Filter* filter);

		const Vector2i mFullResolution;
		Filter* mFilter = nullptr;

		FilmTilePixel& GetTilePixel(const Vector2i& pt);
		

		void AddSample(const Vector2f& pFilm, Vector3f L, float smapleWeight = 1.f);

		void WriteImage();

	private:
		Pixel& GetPixel(const Vector2i& pt);

		static constexpr int32 mFilterTableWidth = 16;	// constexpr是新的关键字，表明是编译期常量。这样可以用做指定数组长度
		float mFilterTable[mFilterTableWidth * mFilterTableWidth];

		std::vector<FilmTilePixel> mPixelTiles;
		std::vector<Pixel> mPixels;
		std::string mFilename;
	};

	Film* CreateFilm(const Vector2i& res, Filter* filter);
}