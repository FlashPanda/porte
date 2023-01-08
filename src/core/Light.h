#pragma once
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Interaction.h"

namespace panda
{
	class Scene;
	class Sampler;
	// 光源类型
	enum class LightFlags : int32
	{
		DeltaPosition = 1,	// 点光源
		DeltaDirection = 1 << 1,	// 方向光
		Area = 1 << 2,	// 面光源
		Infinite = 1 << 3	// 无限远光源
	};

	inline bool IsDeltaLight(int32 flags)
	{
		return flags & (int32)LightFlags::DeltaPosition ||
			flags & (int32)LightFlags::DeltaDirection;
	}

	// 可见性测试，测试两个点之间是否可以直接到达
	class VisibilityTester
	{
	public:
		VisibilityTester() {}
		VisibilityTester(const Interaction& p0, const Interaction& p1) : p0(p0), p1(p1) {}

		const Interaction& P0() const { return p0; }
		const Interaction& P1() const { return p1; }
		bool Unoccluded(const Scene& scene) const;
		Vector3Df Tr(const Scene& scene, Sampler& sampler) const;

	private:
		Interaction p0, p1;
	};

	class Light
	{
	public:
		virtual ~Light();
		Light(int32 flags, const Matrix4f& LocalToWorld, int32 nSamples = 1);
		virtual Vector3Df Sample_Li(const Interaction& ref, const Vector2Df& u,
			Vector3Df* wi, float* pdf, VisibilityTester* vis) const = 0;
		virtual Vector3Df Power() const = 0;
		virtual void Preprocess(const Scene& scene) {}
		virtual Vector3Df Le(const Ray& r) const;
		virtual float Pdf_Li(const Interaction& ref, const Vector3Df& wi) const = 0;
		virtual Vector3Df Sample_Le(const Vector2Df& u1, const Vector2Df& u2, float time,
			Ray* ray, Vector3Df* nLight, float* pdfPos, float* pdfDir) const = 0;
		virtual void Pdf_Le(const Ray& ray, const Vector3Df& nLight, float* pdfPos, float* pdfDir) const = 0;

		const int32 flags;
		const int32 nSamples;
		
	protected:
		const Matrix4f LocalToWorld, WorldToLocal;
	};

	class PointLight : public Light
	{
	public:
		PointLight(const Matrix4f& LocalToWorld, const Vector3Df& I);
		Vector3Df Sample_Li(const Interaction& ref, const Vector2Df& u, Vector3Df* wi,
			float* pdf, VisibilityTester* vis) const;
		Vector3Df Power() const;
		float Pdf_Li(const Interaction&, const Vector3Df&) const;
		Vector3Df Sample_Le(const Vector2Df& u1, const Vector2Df& u2, float time, Ray* ray, Vector3Df* nLight,
			float* pdfPos, float* pdfDir) const;
		void Pdf_Le(const Ray&, const Vector3Df&, float* pdfPos, float* pdfDir) const;

	private:
		const Vector3Df pLight;		// 位置
		const Vector3Df I;	// 亮度
	};

	std::shared_ptr<PointLight> CreatePointLight(const Matrix4f& local2world, const Vector3Df& color, 
		float intensity);
}