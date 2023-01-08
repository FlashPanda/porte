#include "Integrator.h"
#include "Scene.h"
#include "SceneNodeCamera.h"
#include "Sampler.h"
#include "Interaction.h"
#include "Light.h"

namespace panda
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
			Vector2Di sampleExtent = ppFilm->mFullResolution;
			const int32 tileSize = 16;	// 采样块大小
			Vector2Di nTiles({(sampleExtent[0] + tileSize - 1) / tileSize,
				(sampleExtent[1] + tileSize - 1) / tileSize });	// 采样块数
			
			for (int32 y = 0; y < nTiles[1]; ++y)
				for (int32 x = 0; x < nTiles[0]; ++x)
					RenderTile([&](Vector2Di tile) {
					int32 x0 = 0 + tile[0] * tileSize;
					int32 x1 = std::min(x0 + tileSize, sampleExtent[0]);
					int32 y0 = 0 + tile[1] * tileSize;
					int32 y1 = std::min(y0 + tileSize, sampleExtent[1]);
					Vector2Di boundMin({ x0, y0 });
					Vector2Di boundMax({ x1, y1 });

					// TODO：为了多线程，可以用FilmTile去做分割。

					for (int32 i = x0; i < x1; ++i)
					{
						for (int32 j = y0; j < y1; ++j)
						{
							Vector2Di pixel({i, j});
							// 像素位置（i，j）
							mSampler->StartPixel(pixel);

							do
							{
								CameraSample cameraSample = mSampler->GetCameraSample(pixel);

								// 为当前样本生成ray
								Ray ray;
								int32 rayWeight = mCamera->GenerateRay(cameraSample, &ray);

								Vector3Df L(0.f);
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
						}, Vector2Di({ x, y }));

			ppFilm->WriteImage();
		}
	}
	void SamplerIntegrator::RenderTile(std::function<void(Vector2Di)> func, Vector2Di tile)
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

	Vector3Df PathIntegrator::Li(const Ray& ray, const Scene& scene,
		Sampler& sampler, int32 depth) const
	{
		return Vector3Df(1.f);
	}

	DirectLightingIntegrator::DirectLightingIntegrator(LightStrategy strategy, int32 maxDepth) :
		mStrategy(strategy), mMaxDepth(maxDepth)
	{
	}

	DirectLightingIntegrator::~DirectLightingIntegrator() {}

	void DirectLightingIntegrator::Render(const Scene* pScene)
	{

	}

	Vector3Df DirectLightingIntegrator::Li(const Ray& ray, const Scene& scene,
		Sampler& sampler, int32 depth) const
	{
		return Vector3Df(0.f);
	}

	DirectLightingIntegrator* CreateDirectLightingIntegrator(
		LightStrategy strategy, int32 maxDepth,
		std::shared_ptr<Sampler> sampler,
		std::shared_ptr<SceneNodeCamera> camera)
	{
		DirectLightingIntegrator* pIt = new DirectLightingIntegrator(strategy, maxDepth);
		pIt->SetSampler(sampler);
		pIt->SetCamera(camera);

		return pIt;
	}

	Vector3Df WhittedIntegrator::Li(const Ray& ray, const Scene& scene,
		Sampler& sampler, int32 depth) const
	{
		Vector3Df L(0.f);

		SurfaceInteraction isect;
		if (!scene.Intersect(ray, &isect))
		{
			for (auto iter : scene.mPbrtLights)
			{
				L += iter->Le(ray);
			}
			return L;
		}

		// 计算交点的发射光与反射光

		// 初始化通用向量
		const Vector3Df n = isect.n;	// 法线
		Vector3Df wo = isect.wo;

		// 计算交点的表面散射函数
		isect.ComputeScatteringFunctions(ray);
		// 体积计算
		//if (!isect.bsdf)
		//	return Li(isect.SpawnRay(ray.d), scene, sampler, depth);

		// 计算交点的发射光
		L += isect.Le(wo);

		// 遍历每个光源，计算直接光照
		for (const auto& iter : scene.mPbrtLights)
		{
			Vector3Df wi;
			float pdf;
			VisibilityTester visibility;
			Vector3Df Li =
				iter->Sample_Li(isect, sampler.Get2D(), &wi, &pdf, &visibility);

			if (Li.IsBlack() || pdf == 0) continue;

			Vector3Df f = isect.bsdf->f(wo, wi);
			if (!f.IsBlack() && visibility.Unoccluded(scene))
				L += f * Li * AbsDotProduct(wi, n) / pdf;
		}

		// 追踪光滑表面的反射和折射
		// TODO

		return L;
	}
}
