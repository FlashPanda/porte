#pragma once
#include <core/MathUtil.h>
#include <core/Interaction.h>

namespace porte
{
	inline float AbsCosTheta(const Vector3f& w) { return std::abs(w[2]); }

	// �������ʹ�ſ��Թ��Ϊ4�֣����䣨diffuse���������䣨gloosy�������淴�䣨specular�������䣨retro-reflective��
	// ���䱻�鵽�������С�
	
	// ����BxDF������
	enum BxDFType {
		// ��Ч
		BSDF_INVALID = 0,

		// ����
		BSDF_REFLECTION = 1 << 0,

		// ͸��
		BSDF_TRANSMISSION = 1 << 1,

		// ����
		BSDF_DIFFUSE = 1 << 2,

		// ������
		BSDF_GLOSSY = 1 << 3,

		// ���淴��
		BSDF_SPECULAR = 1 << 4,

		// ����
		BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
		BSDF_TRANSMISSION,
	};

	class BxDF
	{
	public:
		BxDF(BxDFType type) : mType(type) {}
		virtual ~BxDF() {}
		bool MatchesFlags(BxDFType t) const {return (mType & t) == mType; }
		virtual Vector3f f(const Vector3f& wo, const Vector3f& wi) const = 0;
		// �����������wi������ı���
		virtual Vector3f Sample_f(const Vector3f& wo, Vector3f* wi,
									const Vector2f& sample, float* pdf,
									BxDFType* sampledType = nullptr) const;
		virtual float Pdf(const Vector3f& wo, const Vector3f& wi) const;

		virtual std::string ToString() const = 0;

		const BxDFType mType;
	};

	// ��������䣨�ʲ����䣩
	class LambertianReflection : public BxDF
	{
	public:
		LambertianReflection(const Vector3f& ref) : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), mReflection(ref)
		{}

	private:
		const Vector3f mReflection;
	};

	// BSDF
	class BSDF
	{
	public:
		BSDF(const SurfaceInteraction& si, float eta = 1.f)
			: eta(eta),
			ns(si.n),	// TODO: �ĳ�shading normal
			ng(si.n),
			ss(Normalize(si.dpdu)),
			ts(CrossProduct(ns, ss))
		{}

		void Add(BxDF* b)
		{
			assert(nBxDFs <= MaxBxDFs);
			bxdfs[nBxDFs++] = b;
		}

		int32 NumComponents(BxDFType flags = BSDF_ALL) const;

		Vector3f WorldToLocal(const Vector3f& v) const
		{
			return Vector3f({DotProduct(v, ss), DotProduct(v, ts), DotProduct(v, ns)});
		}

		Vector3f LocalToWorld(const Vector3f& v) const
		{
			return Vector3f({ss[0] * v[0] + ts[0] * v[1] + ns[0] * v[2],
				ss[1] * v[0] + ts[1] * v[1] + ns[1] * v[2],
				ss[2] * v[0] + ts[2] * v[1] + ns[2] * v[2]});
		}

		Vector3f f(const Vector3f& woW, const Vector3f& wiW,
			BxDFType flags = BSDF_ALL) const;

		Vector3f Sample_f(const Vector3f& wo, Vector3f* wi, const Vector2f& u,
			float* pdf, BxDFType type = BSDF_ALL,
			BxDFType* sampledType = nullptr) const;

		float Pdf(const Vector3f& wo, const Vector3f& wi,
			BxDFType flags = BSDF_ALL) const;

		std::string ToString() const;

		const float eta;
	private:
		// ���Լ����ƶ�����ͷţ��ʶ����˽�������������������б�Ҫ��
		~BSDF() {}
		const Vector3f ns, ng;		// todo: ns��shading normal
		const Vector3f ss, ts;		// �����븱����
		static constexpr int32 MaxBxDFs = 8;	// ���8��BxDF
		BxDF* bxdfs[MaxBxDFs];
		int32 nBxDFs = 0;	// bxdf������
	};
}