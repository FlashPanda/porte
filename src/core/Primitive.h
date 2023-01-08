#pragma once
#include "Bounds.h"
#include "Shape.h"

namespace panda
{
	class SurfaceInteraction;
	class Material;

	class Primitive {
	public:
		virtual ~Primitive();
		virtual Bounds3f WorldBound() const = 0;
		virtual bool Intersect(const Ray& r, SurfaceInteraction*) const = 0;
		virtual bool IntersectP(const Ray& r) const = 0;
		virtual const Material* GetMaterial() const = 0;
		virtual void ComputeScatteringFunctions(SurfaceInteraction* isect) const = 0;
	};

	class GeometricPrimitive : public Primitive {
	public:
		GeometricPrimitive(const std::shared_ptr<Shape>& shape,
			const std::shared_ptr<Material>& material);

		GeometricPrimitive(const std::shared_ptr<Shape>& shape,
			const std::string& matName);

		virtual Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray& r, SurfaceInteraction* isect) const;
		virtual bool IntersectP(const Ray& r) const;
		const Material* GetMaterial() const;
		void ComputeScatteringFunctions(SurfaceInteraction* isect) const;

		void MaterialRedirection(const std::shared_ptr<Material>& material);
		std::string GetMatName() {
			return matName;
		}
	private:
		std::shared_ptr<Shape> shape;
		std::shared_ptr<Material> material;
		std::string matName;
	};

	class Aggregate : public Primitive {
	public:
		const Material* GetMaterial() const;
		void ComputeScatteringFunctions(SurfaceInteraction* isect) const;
	};

	struct BVHPrimitiveInfo;
	struct BVHBuildNode;
	struct LinearBVHNode;

	class BVHAccel : public Aggregate
	{
	public:
		enum class SplitMethod
		{
			SAH,
			HLBVH,
			Middle,
			EqualCounts
		};

		BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
			int32 maxPrimsInNode = 1,
			SplitMethod splitMethod = SplitMethod::SAH);

		Bounds3f WorldBound() const;
		~BVHAccel();
		bool Intersect(const Ray& ray, SurfaceInteraction* isect) const;
		bool IntersectP(const Ray& ray) const;

	private:
		BVHBuildNode* RecursiveBuild(std::vector<BVHPrimitiveInfo>& primitiveInfo,
			int32 start, int32 end, int32* totalNodes,
			std::vector<std::shared_ptr<Primitive>>& orderedPrims);

		BVHBuildNode* HLBVHBuild(const std::vector<BVHPrimitiveInfo>& primitiveInfo,
			int* totalNodes,
			std::vector<std::shared_ptr<Primitive>>& orderedPrims) const;

		int32 FlattenBVHTree(BVHBuildNode* node, int32* offset);

	private:
		const int32 mMaxPrimsInNode;
		const SplitMethod mSplitMethod;
		std::vector<std::shared_ptr<Primitive>> mPrimitives;
		LinearBVHNode* nodes = nullptr;
	};

	std::shared_ptr<BVHAccel> CreateBVHAccelerator(std::vector<std::shared_ptr<Primitive>> prims);
}