#pragma once
#include <core/Primitive.h>

namespace porte
{
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