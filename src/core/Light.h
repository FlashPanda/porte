#pragma once
#include <core/MathUtil.h>
#include <core/Interaction.h>

namespace porte
{
	class Scene;
	class Sampler;
	// ��Դ����
	enum class LightFlags : int32
	{
		DeltaPosition = 1,	// ���Դ
		DeltaDirection = 1 << 1,	// �����
		Area = 1 << 2,	// ���Դ
		Infinite = 1 << 3	// ����Զ��Դ
	};

	inline bool IsDeltaLight(int32 flags)
	{
		return flags & (int32)LightFlags::DeltaPosition ||
			flags & (int32)LightFlags::DeltaDirection;
	}

	// �ɼ��Բ��ԣ�����������֮���Ƿ����ֱ�ӵ���
	class VisibilityTester
	{
	public:
		VisibilityTester() {}
		VisibilityTester(const Interaction& p0, const Interaction& p1) : p0(p0), p1(p1) {}

		const Interaction& P0() const { return p0; }
		const Interaction& P1() const { return p1; }
		bool Unoccluded(const Scene& scene) const;
		Vector3f Tr(const Scene& scene, Sampler& sampler) const;

	private:
		Interaction p0, p1;
	};

	class Light
	{
	public:
		virtual ~Light();
		Light(int32 flags, const Matrix4f& LocalToWorld, int32 nSamples = 1);
		virtual Vector3f Sample_Li(const Interaction& ref, const Vector2f& u,
			Vector3f* wi, float* pdf, VisibilityTester* vis) const = 0;
		virtual Vector3f Power() const = 0;
		virtual void Preprocess(const Scene& scene) {}
		virtual Vector3f Le(const Ray& r) const;
		virtual float Pdf_Li(const Interaction& ref, const Vector3f& wi) const = 0;
		virtual Vector3f Sample_Le(const Vector2f& u1, const Vector2f& u2, float time,
			Ray* ray, Vector3f* nLight, float* pdfPos, float* pdfDir) const = 0;
		virtual void Pdf_Le(const Ray& ray, const Vector3f& nLight, float* pdfPos, float* pdfDir) const = 0;

		const int32 flags;
		const int32 nSamples;
		
	protected:
		const Matrix4f LocalToWorld, WorldToLocal;
	};
}