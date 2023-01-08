#pragma once

#include "Vector.hpp"
#include <iostream>

namespace panda
{
	class Ray
	{
	public:
		Ray() : tMax(FloatInfinity) {}
		Ray(const Vector3Df& o, const Vector3Df d, float tMax = FloatInfinity) :
			o(o), d(d), tMax(tMax) {}

		friend std::ostream& operator<<(std::ostream& os, const Ray& r);

		Vector3Df o;	// ԭ��
		Vector3Df d;	// ����
		mutable float tMax;		// t�����ֵ�������mutable��Ϊ��ͻ��const���ơ�Ҳ����˵��const��ray��Ҳ���޸�tMax��ֵ��
	};
}