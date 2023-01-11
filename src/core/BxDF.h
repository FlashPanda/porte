#pragma once
#include <core/MathUtil.h>
#include <core/Interaction.h>

namespace porte
{
	inline float AbsCosTheta(const Vector3f& w) { return std::abs(w[2]); }

	// 反射类型大概可以归结为4种：漫射（diffuse），光泽反射（gloosy），镜面反射（specular），回射（retro-reflective）
	// 回射被归到光泽反射中。
	
	// 各种BxDF的声明
	enum BxDFType {
		// 无效
		BSDF_INVALID = 0,

		// 反射
		BSDF_REFLECTION = 1 << 0,

		// 透射
		BSDF_TRANSMISSION = 1 << 1,

		// 漫射
		BSDF_DIFFUSE = 1 << 2,

		// 光泽反射
		BSDF_GLOSSY = 1 << 3,

		// 镜面反射
		BSDF_SPECULAR = 1 << 4,

		// 所有
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
		// 区别在于这个wi是输出的变量
		virtual Vector3f Sample_f(const Vector3f& wo, Vector3f* wi,
									const Vector2f& sample, float* pdf,
									BxDFType* sampledType = nullptr) const;
		virtual float Pdf(const Vector3f& wo, const Vector3f& wi) const;

		virtual std::string ToString() const = 0;

		const BxDFType mType;
	};

	// 理想的漫射（朗伯漫射）
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
			ns(si.n),	// TODO: 改成shading normal
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
		// 想自己控制对象的释放，故定义成私有析构函数。但是这有必要吗？
		~BSDF() {}
		const Vector3f ns, ng;		// todo: ns是shading normal
		const Vector3f ss, ts;		// 切线与副切线
		static constexpr int32 MaxBxDFs = 8;	// 最多8个BxDF
		BxDF* bxdfs[MaxBxDFs];
		int32 nBxDFs = 0;	// bxdf的数量
	};
}