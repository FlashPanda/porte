#include "Integrator.h"
#include "Scene.h"
#include "SceneNodeCamera.h"
#include "Sampler.h"
#include "Interaction.h"
#include "Light.h"

namespace porte
{
	Integrator::~Integrator() {}

	SamplerIntegrator::SamplerIntegrator() : Integrator()
	{
	}
	SamplerIntegrator::~SamplerIntegrator() {}

	void SamplerIntegrator::Render(const Scene* pScene)
	{
		if (pScene)
		{
			Film* ppFilm = pScene->mMainCamera->GetCamereObject()->mFilm;
			// 需要采样的范围
			Vector2i sampleExtent = ppFilm->mFullResolution;
			const int32 tileSize = 16;	// 采样块大小
			Vector2i nTiles({(sampleExtent[0] + tileSize - 1) / tileSize,
				(sampleExtent[1] + tileSize - 1) / tileSize });	// 采样块数
			
			for (int32 y = 0; y < nTiles[1]; ++y)
				for (int32 x = 0; x < nTiles[0]; ++x)
					RenderTile([&](Vector2i tile) {
					int32 x0 = 0 + tile[0] * tileSize;
					int32 x1 = std::min(x0 + tileSize, sampleExtent[0]);
					int32 y0 = 0 + tile[1] * tileSize;
					int32 y1 = std::min(y0 + tileSize, sampleExtent[1]);
					Vector2i boundMin({ x0, y0 });
					Vector2i boundMax({ x1, y1 });

					// TODO：为了多线程，可以用FilmTile去做分割。

					for (int32 i = x0; i < x1; ++i)
					{
						for (int32 j = y0; j < y1; ++j)
						{
							Vector2i pixel({i, j});
							// 像素位置（i，j）
							mSampler->StartPixel(pixel);

							do
							{
								CameraSample cameraSample = mSampler->GetCameraSample(pixel);

								// 为当前样本生成ray
								Ray ray;
								int32 rayWeight = mCamera->GenerateRay(cameraSample, &ray);

								Vector3f L(0.f);
								if (rayWeight > 0) L = Li(ray, *pScene, *mSampler);

								if (L.HasNaNs())
								{
									std::cout << "pixel (" << pixel[0] << ", " << pixel[1] << "), sample "
										<< mSampler->CurrentSampleNumber() << " . Not-a-number radiance value returned! Set to black." << std::endl;
									L.Set(0.f);
								}
								else if (L.y() < -1e-5)
								{
									std::cout << "pixel (" << pixel[0] << ", " << pixel[1] << "), sample "
										<< mSampler->CurrentSampleNumber() << " . Negative luminance value! Set to black." << std::endl;
									L.Set(0.f);
								}
								else if (std::isinf(L.y()))
								{
									std::cout << "pixel (" << pixel[0] << ", " << pixel[1] << "), sample "
										<< mSampler->CurrentSampleNumber() << " . Infinite luminance value returned! Set to black." << std::endl;
									L.Set(0.f);
								}

								std::cout << "Camera sample: " << cameraSample << " -> ray: " << ray << " -> L = " << L;

								ppFilm->AddSample(cameraSample.pFilm, L, rayWeight);

							}while (mSampler->StartNextSample());
						}
					}
						}, Vector2i({ x, y }));

			ppFilm->WriteImage();
		}
	}
	void SamplerIntegrator::RenderTile(std::function<void(Vector2i)> func, Vector2i tile)
	{
		func(tile);
	}


	PathIntegrator::PathIntegrator() : SamplerIntegrator()
	{

	}

	PathIntegrator::~PathIntegrator() {}

	void PathIntegrator::Render(const Scene* pScene)
	{

	}

	Vector3f PathIntegrator::Li(const Ray& ray, const Scene& scene,
		Sampler& sampler, int32 depth) const
	{
		return Vector3f(1.f);
	}




}
