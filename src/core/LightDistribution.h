#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_LIGHT_DISTRIBUTION_H
#define PORTE_LIGHT_DISTRIBUTION_H

#include <core/porte.h>
#include <core/Geometry.h>
#include <core/Sampling.h>
#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace porte
{
	// LightDistribution定义了一个通用的接口，提供一个概率分布
	// 当采样空间中光源上的一点的时候
	class LightDistribution
	{
	public:
		virtual ~LightDistribution();

		// 给定空间中的一点P，这个方法将返回该点的光源采样分布
		// （希望是有效的）
		virtual const Distribution1D* Lookup(const Point3f& p) const = 0;
	};

	std::unique_ptr<LightDistribution> CreateLightSampleDistribution(
		const std::string& name, const Scene& scene);

	// 最简单的LightDistribution实现：返回所有光源的均匀分布，无视提供的点。
	// 这个方法对简单的场景极为有效，但是对一个手掌数量的光源场景特别低效。
	class UniformLightDistribution : public LightDistribution
	{
	public:
		UniformLightDistribution(const Scene& scene);
		const Distribution1D* Lookup(const Point3f& p) const;

	private:
		std::unique_ptr<Distribution1D> distrib;
	};
}

#endif