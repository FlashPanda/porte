#include "BxDF.h"
#include "Sampling.h"
#include <sstream>
#include "RNG.h"

namespace panda
{
	Vector3Df BxDF::Sample_f(const Vector3Df& wo, Vector3Df* wi,
		const Vector2Df& u, float* pdf,
		BxDFType* sampledType) const
	{
		// cos�����������Ҫ�Ļ���ת����
		*wi = CosineSampleHemisphere(u);
		if (wo[2] < 0) wi[2] *= -1;
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}

	// �����������Ϊcosine sample hemisphere��������ô��ġ�
	float BxDF::Pdf(const Vector3Df& wo, const Vector3Df& wi) const
	{
		// ��֤wo��wi������ͬ�İ���
		return wo[2] * wi[2] > 0? AbsCosTheta(wi) * INV_PI : 0;
	}

	Vector3Df BSDF::f(const Vector3Df& woW, const Vector3Df& wiW,
		BxDFType flags) const
	{
		Vector3Df wi = WorldToLocal(wiW), wo = WorldToLocal(woW);
		if (wo[2] == 0) return 0.f;

		bool reflect = DotProduct(wiW, ng) * DotProduct(woW, ng) > 0;
		Vector3Df f(0.f);

		for (int32 i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(flags) && 
				((reflect && (bxdfs[i]->mType & BSDF_REFLECTION)) || 
				 (!reflect && (bxdfs[i]->mType & BSDF_TRANSMISSION))))
				 f += bxdfs[i]->f(wo, wi);
		return f;
	}

	Vector3Df BSDF::Sample_f(const Vector3Df& woWorld, Vector3Df* wiWorld, const Vector2Df& u,
		float* pdf, BxDFType type,
		BxDFType* sampledType) const
	{
		// Ҫ�����ĸ�BxDF
		int32 matchingComps = NumComponents(type);
		if (matchingComps == 0)
		{
			*pdf = 0;
			if (sampledType) *sampledType = BxDFType(0);
			return Vector3Df(0.f);
		}

		int32 comp = std::min((int32)std::floor(u[0] * matchingComps), matchingComps - 1);

		// �����Ӧ�����BxDfָ��
		BxDF* bxdf = nullptr;
		int32 count = comp;
		for (int32 i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(type) && count-- == 0)
			{
				bxdf = bxdfs[i];
				break;
			}
		assert(bxdf != nullptr);

		// ������u����ӳ�䵽$[0,1)^2$
		Vector2Df uRemapped(std::min(u[0] * matchingComps - comp, OneMinusEpsilon),
			u[1]);
		Vector3Df wi, wo = WorldToLocal(woWorld);
		if (wo[2] == 0) return 0.f;
		*pdf = 0;
		if (sampledType) *sampledType = bxdf->mType;
		// ��һ��������ʵ����Ŀ���ǲ�һ������pdf�Լ�f��֮�󶼻�����һ�顣
		Vector3Df f = bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);
		if (*pdf == 0)
		{
			if (sampledType) *sampledType = BxDFType(0);
			return 0.f;
		}
		*wiWorld = LocalToWorld(wi);

		// ��������ƥ���BxDF���������PDF
		if (!(bxdf->mType & BSDF_SPECULAR) && matchingComps > 1)
		{
			for (int32 i = 0; i< nBxDFs; ++i)
				if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
					*pdf += bxdfs[i]->Pdf(wo, wi);
		}
		if (matchingComps > 1) *pdf /= matchingComps;

		// ������������BSDFֵ
		if (!(bxdf->mType & BSDF_SPECULAR))
		{
			bool reflect = DotProduct(*wiWorld, ng) * DotProduct(woWorld, ng) > 0;
			f = 0.;
			for (int32 i = 0; i < nBxDFs; ++i)
			{
				if (bxdfs[i]->MatchesFlags(type) && 
					((reflect && (bxdfs[i]->mType & BSDF_REFLECTION)) ||
					 (!reflect && (bxdfs[i]->mType & BSDF_TRANSMISSION))))
					 f += bxdfs[i]->f(wo, wi);
			}
		}

		return f;
	}

	int32 BSDF::NumComponents(BxDFType flags) const
	{
		int32 num = 0;
		for (int32 i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(flags)) ++num;
		return num;
	}

	float BSDF::Pdf(const Vector3Df& woWorld, const Vector3Df& wiWorld,
		BxDFType flags) const
	{
		if(nBxDFs == 0) return 0.f;
		Vector3Df wo = WorldToLocal(woWorld), wi = WorldToLocal(wiWorld);
		if (wo[2] == 0.f) return 0.f;
		float pdf = 0.f;
		int32 matchingComps = 0;
		for (int32 i = 0; i < nBxDFs; ++i)
		{
			if (bxdfs[i]->MatchesFlags(flags))
			{
				++matchingComps;
				pdf += bxdfs[i]->Pdf(wo, wi);
			}
		}

		float v = matchingComps > 0? pdf / matchingComps : 0.f;
		return v;
	}

	std::string BSDF::ToString() const
	{
		std::ostringstream oss;
		oss << "[BSDF eta : " << eta
			<< " nBxDFs: " << nBxDFs << std::endl;
		for (int32 i = 0; i < nBxDFs; ++i)
			oss << "  bxdfs[" << i 
				<< "]: " << bxdfs[i]->ToString()
				<< "  ]" << std::endl;
		return oss.str();
	}
}