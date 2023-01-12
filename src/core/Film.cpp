#include "Film.h"
#include <algorithm>
#include <cmath>
#include <malloc.h>
#include "Image.h"
#include "Bounds.h"

namespace porte
{
	void XYZToRGB(const float xyz[3], float rgb[3])
	{
		rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
		rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
		rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];
	}
	void RGBToXYZ(const float rgb[3], float xyz[3])
	{
		xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
		xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
		xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];
	}

	Film::Film(const Vector2i& resolution, Filter* filter) :
		mFullResolution(resolution), mFilter(filter)
	{
		mPixelTiles = std::vector<FilmTilePixel>(std::max(0, resolution[0] * resolution[1]));
		mPixels = std::vector<Pixel>(std::max(0, resolution[0] * resolution[1]));

		// 预计算滤波权重表
		int32 offset = 0;
		for (int32 y = 0; y < mFilterTableWidth; ++y)
		{
			for (int32 x = 0; x < mFilterTableWidth; ++x, ++offset)
			{
				Vector2f p;
				p[0] = (x + 0.5f) * mFilter->mRadius[0] / mFilterTableWidth;
				p[1] = (y + 0.5f) * mFilter->mRadius[1] / mFilterTableWidth;
				mFilterTable[offset] = mFilter->Evaluate(p);
			}
		}

	}

	FilmTilePixel& Film::GetTilePixel(const Vector2i& pt)
	{
		int32 index = mFullResolution[0] * pt[1] + pt[0];
		if (index >= mPixels.size())
			return mPixelTiles[0];
		else
			return mPixelTiles[index];
	}

	Pixel& Film::GetPixel(const Vector2i& pt)
	{
		int32 index = mFullResolution[0] * pt[1] + pt[0];
		if (index >= mPixels.size())
			return mPixels[0];
		else
			return mPixels[index];
	}

	void Film::AddSample(const Vector2f& pFilm, Vector3f L, float sampleWeight)
	{
		Vector2f pFilmDiscrete = pFilm - Vector2f({0.5f, 0.5f});
		Vector2f p0f = Ceil(pFilmDiscrete - mFilter->mRadius);
		Vector2i p0({(int32)p0f.data[0], (int32)p0f.data[1]});
		Vector2f p1f = Floor(pFilmDiscrete + mFilter->mRadius);
		Vector2i p1 = Vector2i({(int32)p1f.data[0], (int32)p1f.data[1]}) + Vector2i({1, 1});

		p0 = Max(p0, Vector2i({ 0, 0 }));
		p1 = Min(p1, mFullResolution);

		// 预计算x和y的滤波表偏移
		int32* ifx = (int32*)_alloca(sizeof(int32) * (p1[0] - p0[0]));	// 这个函数仅在win32里有效，效果是在栈上分配内存，用完之后就回收。
		for (int32 x = p0[0]; x < p1[0]; ++x)
		{
			float fx = std::abs((x - pFilmDiscrete[0]) * mFilter->mInvRadius[0] * mFilterTableWidth);
			ifx[x - p0[0]] = std::min((int32)std::floor(fx), mFilterTableWidth - 1);
		}
		int32* ify = (int32*)_alloca(sizeof(int32) * (p1[1] - p0[1]));
		for (int32 y = p0[1]; y < p1[1]; ++y)
		{
			float fy = std::abs((y - pFilmDiscrete[1]) * mFilter->mInvRadius[1] * mFilterTableWidth);
			ify[y - p0[1]] = std::min((int32)std::floor(fy), mFilterTableWidth - 1);
		}

		for (int32 y = p0[1]; y < p1[1]; ++y)
		{
			for (int32 x = p0[0]; x < p1[1]; ++x)
			{
				// 计算在x，y像素位置的滤波值
				int32 offset = ify[y - p0[1]] * mFilterTableWidth + ifx[x - p0[0]];
				float filterWeight = mFilterTable[offset];

				// 更新像素值，用滤波后的样本贡献
				FilmTilePixel& pixel = GetTilePixel(Vector2i({x, y}));
				pixel.contribSum += L * sampleWeight * filterWeight;
				pixel.filterWeightSum += filterWeight;
			}
		}
	}

	void Film::WriteImage()
	{
		float* rgb(new float[3 * mFullResolution[0] * mFullResolution[1]]);
		int32 offset = 0;
		for (int32 y = 0; y < mFullResolution[1]; ++y)
		{
			for (int32 x = 0; x < mFullResolution[0]; ++x)
			{
				Pixel& pt = GetPixel(Vector2i({x, y}));
				XYZToRGB(pt.xyz, &rgb[3 * offset]);

				float filterWeightSum = pt.filterWeightSum;
				if (filterWeightSum != 0)
				{
					float invWt = 1.f / filterWeightSum;
					rgb[3 * offset] = std::max((float)0, rgb[3 * offset] * invWt);
					rgb[3 * offset + 1] = std::max((float)0, rgb[3 * offset + 1] * invWt);
					rgb[3 * offset + 2] = std::max((float)0, rgb[3 * offset + 2] * invWt);
				}

				++offset;
			}
		}

		// 写入RGB图像
		porte::WriteImage(mFilename, &rgb[0], Bounds2i(), mFullResolution);
	}

	Film* CreateFilm(const Vector2i& res, Filter* filter)
	{
		return new Film(res, filter);
	}
}