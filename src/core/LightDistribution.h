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
	// LightDistribution������һ��ͨ�õĽӿڣ��ṩһ�����ʷֲ�
	// �������ռ��й�Դ�ϵ�һ���ʱ��
	class LightDistribution
	{
	public:
		virtual ~LightDistribution();

		// �����ռ��е�һ��P��������������ظõ�Ĺ�Դ�����ֲ�
		// ��ϣ������Ч�ģ�
		virtual const Distribution1D* Lookup(const Point3f& p) const = 0;
	};

	std::unique_ptr<LightDistribution> CreateLightSampleDistribution(
		const std::string& name, const Scene& scene);

	// ��򵥵�LightDistributionʵ�֣��������й�Դ�ľ��ȷֲ��������ṩ�ĵ㡣
	// ��������Լ򵥵ĳ�����Ϊ��Ч�����Ƕ�һ�����������Ĺ�Դ�����ر��Ч��
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