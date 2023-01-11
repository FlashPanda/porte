#pragma once

#include <core/MathUtil.h>
#include <iostream>

namespace porte
{
	class Ray
	{
	public:
		Ray() : tMax(FloatInfinity) {}
		Ray(const Vector3f& o, const Vector3f d, float tMax = FloatInfinity) :
			o(o), d(d), tMax(tMax) {}

		friend std::ostream& operator<<(std::ostream& os, const Ray& r);

		Vector3f o;	// ԭ��
		Vector3f d;	// ����
		mutable float tMax;		// t�����ֵ�������mutable��Ϊ��ͻ��const���ơ�Ҳ����˵��const��ray��Ҳ���޸�tMax��ֵ��
	};
}