#include "Shape.h"
#include "Interaction.h"
#include "Transform.h"

namespace panda
{
	Shape::~Shape() {}

	Shape::Shape(const Matrix4f* ObjToWorld, const Matrix4f* WorldToObj)
		: ObjectToWorld(ObjToWorld), WorldToObject(WorldToObj)
	{

	}

	Bounds3f Shape::WorldBound() const
	{
		return TransformBounds3f(ObjectBound(),*ObjectToWorld);
	}

	bool Shape::IntersectP(const Ray& ray) const
	{
		return Intersect(ray, nullptr, nullptr);
	}

	Interaction Shape::Sample(const Interaction& ref, const Vector2Df& u,
		float* pdf) const
	{
		Interaction intr = Sample(u, pdf);
		Vector3Df wi = intr.p - ref.p;

		if (GetLengthSquare(wi) == 0)
			*pdf = 0;
		else
		{
			wi = Normalize(wi);

			// 从面积的测度转换成立体角的测度
			*pdf *= GetLengthSquare(ref.p - intr.p) / AbsDotProduct(intr.n, -wi);
			if (std::isinf(*pdf)) *pdf = 0.f;
		}

		return intr;
	}

	float Shape::Pdf(const Interaction& ref, const Vector3Df& wi) const
	{
		// TODO: Intersect sample ray with area light geometry
		return 0.f;
	}

	Bounds3f Triangle::ObjectBound() const
	{
		// 这里和pbrt中不一样，我保存的是local system中的坐标，不是世界空间中的坐标。
		const Vector3Df& p0 = mesh->GetVertexArray()[v[0]];
		const Vector3Df& p1 = mesh->GetVertexArray()[v[1]];
		const Vector3Df& p2 = mesh->GetVertexArray()[v[2]];

		return Union(Bounds3f(p0, p1), p2);
	}

	Bounds3f Triangle::WorldBound() const
	{
		// 这里和pbrt中不一样，我保存的是local system中的坐标，不是世界空间中的坐标。
		const Vector3Df& p0 = mesh->GetVertexArray()[v[0]];
		const Vector3Df& p1 = mesh->GetVertexArray()[v[1]];
		const Vector3Df& p2 = mesh->GetVertexArray()[v[2]];

		const Vector3Df& wp0 = TransformPoint(p0, *ObjectToWorld);
		const Vector3Df& wp1 = TransformPoint(p1, *ObjectToWorld);
		const Vector3Df& wp2 = TransformPoint(p2, *ObjectToWorld);
		return Union(Bounds3f(wp0, wp1), wp2);
	}

	// 先计算与平面的交点，再判断是否在三角形内部。
	// 注意这个ray是世界空间的，而我的mesh是局部空间的。
	bool Triangle::Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect) const
	{
	#pragma region 我自己的实现
		//// 计算是否和包围盒相交
		//if(!WorldBound().IntersectP(ray))
		//	return false;

		//// 世界空间中的坐标
		//const Vector3Df& p0 = mesh->GetVertexArray()[v[0]];
		//const Vector3Df& p1 = mesh->GetVertexArray()[v[1]];
		//const Vector3Df& p2 = mesh->GetVertexArray()[v[2]];

		//Vector3Df wp0 = TransformPoint(p0, *ObjectToWorld);
		//Vector3Df wp1 = TransformPoint(p1, *ObjectToWorld);
		//Vector3Df wp2 = TransformPoint(p2, *ObjectToWorld);

		//// 计算平面法线
		//Vector3Df normal = CrossProduct(wp1 - wp0, wp2 - wp0);
		//normal = Normalize(normal);

		//if (DotProduct(normal, ray.d) == 0)	// 法线与光线方向平行，意味着没有交点
		//	return false;

		//float t = (DotProduct(normal, wp0) - DotProduct(normal, ray.o)) / DotProduct(normal, ray.d);
		//if (t < 0)
		//	return false;
		//
		//// 交点位置
		//Vector3Df isectPoint = ray.o + t * ray.d;

		//// 通过edge函数，判断是否在三角形内部。
		//// 如果在边上，那么可以算是在内部，也可以不算。

		//if ((e0 < 0 || e1 || 0 && e2 || 0) && (e0 > 0 || e1 > 0 || e2 > 0))
		//	return false;
	#pragma endregion 


		// pbrt的实现是
		// 用一个仿射变换将ray转换成，端点在原点，然后出射方向为z轴。

		// 将局部空间坐标转换为世界空间中的坐标
		const Vector3Df& p0 = mesh->GetVertexArray()[v[0]];
		const Vector3Df& p1 = mesh->GetVertexArray()[v[1]];
		const Vector3Df& p2 = mesh->GetVertexArray()[v[2]];

		Vector3Df wp0 = TransformPoint(p0, *ObjectToWorld);
		Vector3Df wp1 = TransformPoint(p1, *ObjectToWorld);
		Vector3Df wp2 = TransformPoint(p2, *ObjectToWorld);

		// 三角形位置平移到以射线原点为坐标系原点
		Vector3Df p0t = wp0 - ray.o;
		Vector3Df p1t = wp1 - ray.o;
		Vector3Df p2t = wp2 - ray.o;

		// 找到方向的最长轴，转换之后要把它当成z轴。防止z值是0，导致计算错误。
		// 相应的方向向量以及顶点向量就像是乘上一个置换矩阵。
		int32 kz = MaxDimension(Abs(ray.d));
		int32 kx = (kz + 1) % 3;
		int32 ky = (kx + 1) % 3;
		Vector3Df d = Permute(ray.d, kx, ky, kz);
		p0t = Permute(p0t, kx, ky, kz);
		p1t = Permute(p1t, kx, ky, kz);
		p2t = Permute(p2t, kx, ky, kz);

		// 应用一个shear变换，就是把ray的方向转换成+z方向的变换。
		float Sx = -d[0] / d[2];
		float Sy = -d[1] / d[2];
		float Sz = 1.f / d[2];
		p0t[0] += Sx * p0t[2];
		p0t[1] += Sy * p0t[2];
		p1t[0] += Sx * p1t[2];
		p1t[1] += Sy * p1t[2];
		p2t[0] += Sx * p2t[2];
		p2t[1] += Sy * p2t[2];

		// 把三角形投影到xy平面，然后看点(0,0)是否在三角形内部。
		// 计算edge function
		float e0 = p1t[0] * p2t[1] - p1t[1] * p2t[0];
		float e1 = p2t[0] * p0t[1] - p2t[1] * p0t[0];
		float e2 = p0t[0] * p1t[1] - p0t[1] * p1t[0];

		// 用double精度减少误差
		if (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f) {
			double p2txp1ty = (double)p2t[0] * (double)p1t[1];
			double p2typ1tx = (double)p2t[1] * (double)p1t[0];
			e0 = (float)(p2typ1tx - p2txp1ty);
			double p0txp2ty = (double)p0t[0] * (double)p2t[1];
			double p0typ2tx = (double)p0t[1] * (double)p2t[0];
			e1 = (float)(p0typ2tx - p0txp2ty);
			double p1txp0ty = (double)p1t[0] * (double)p0t[1];
			double p1typ0tx = (double)p1t[1] * (double)p0t[0];
			e2 = (float)(p1typ0tx - p1txp0ty);
		}

		// 检查
		if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
			return false;
		float det = e0 + e1 + e2;	// 和等于0意味着在边上，判定为不与三角形相交。
		if (det == 0) return false;

		// 计算交点的方式：
		// 首先明确，因为d转换成z轴的关系，交点的z值就是交点的参数t的值。
		// 第一步，把shear变换应用到三角形各点的z分量上。
		// 第二步，交点z值就是重心坐标，e可以当成是面积大小，而z就是各个面积大小的比例。
		// 实现上，还考虑到了浮点除法的耗时比较高，先检查是否合法，再计算t值。

		// 合法性检测，看t值是否在合法区间内
		p0t[2] *= Sz;
		p1t[2] *= Sz;
		p2t[2] *= Sz;
		float tScaled = e0 * p0t[2] + e1 * p1t[2] + e2 * p2t[2];
		if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
			return false;
		else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det))
			return false;

		// 确定t值合法之后，在算t的具体值
		float invDet = 1 / det;
		float t = tScaled * invDet;

		// 确定交点位置
		float b0 = e0 * invDet;
		float b1 = e1 * invDet;
		float b2 = e2 * invDet;

		// 计算三角形的偏导
		Vector3Df dpdu, dpdv;
		Vector2Df uv[3];
		GetUVs(uv);
		Vector2Df duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
		Vector3Df dp02 = p0 - p2, dp12 = p1 - p2;
		float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
		// uv是否退化
		bool degenerateUV = std::abs(determinant) < 1e-8;
		if (!degenerateUV)
		{
			float invdet = 1 / determinant;
			dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
			dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
		}
		
		if (degenerateUV || GetLengthSquare(CrossProduct(dpdu, dpdv)) == 0)
		{
			// 如果偏导矩阵行列式的值是0
			Vector3Df ng = CrossProduct(p2 - p0, p1 - p0);
			if (GetLengthSquare(ng) == 0)
				return false;

			CoordinateSystem(Normalize(ng), &dpdu, &dpdv);
		}

		// TODO: Error bounds

		Vector3Df pHit = b0 * wp0 + b1 * wp1 + b2 * wp2;
		Vector2Df uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];
		
		*isect = SurfaceInteraction(pHit, Vector3Df(), 
			uvHit, -ray.d,
			dpdu, dpdv,
			Vector3Df(0.f), Vector3Df(0.f),
			this);

		// 插值获得法线
		const std::vector<Vector3Df> normals = mesh->GetNormalArray();
		Vector3Df ns = b0 * normals[v[0]] + b1 * normals[v[1]] + b2 * normals[v[2]];
	
		isect->n = ns;
		*tHit = t;
		return true;
	}

	bool Triangle::IntersectP(const Ray& ray) const
	{
		float tHit;
		SurfaceInteraction isect;
		return Intersect(ray, &tHit, &isect);
	}

	float Triangle::Area() const
	{
		const Vector3Df& p0 = mesh->GetVertexArray()[v[0]];
		const Vector3Df& p1 = mesh->GetVertexArray()[v[1]];
		const Vector3Df& p2 = mesh->GetVertexArray()[v[2]];

		return GetLength(CrossProduct(p1 - p0, p2 - p0)) * 0.5f;
	}

	std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(const Matrix4f* ObjToWorld, const Matrix4f* WorldToObj, const std::shared_ptr<SceneObjectMesh>& mesh, int32 triNumber)
	{
		return std::vector< std::shared_ptr<Shape>>();
	}
}