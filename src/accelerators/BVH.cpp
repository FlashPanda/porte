#include <accelerators/BVH.h>

namespace porte
{
	struct BVHPrimitiveInfo {
		BVHPrimitiveInfo() {}
		BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f& bounds)
			: primitiveNumber(primitiveNumber),
			bounds(bounds),
			centroid(.5f * bounds.pMin + .5f * bounds.pMax) {}
		size_t primitiveNumber;
		Bounds3f bounds;
		Vector3f centroid;
	};

	struct BVHBuildNode
	{
		void InitLeaf(int32 first, int32 n, const Bounds3f& b)
		{
			firstPrimOffset = first;
			nPrimitives = n;
			bounds = b;
			children[0] = children[1] = nullptr;
			// 统计计数，也是实现的细节。
			//++leafNodes;
			//++totalLeafNodes;
			//totalPrimitives += n;
		}

		void InitInterior(int32 axis, BVHBuildNode* c0, BVHBuildNode* c1)
		{
			children[0] = c0;
			children[1] = c1;
			bounds = Union(c0->bounds, c1->bounds);
			splitAxis = axis;
			nPrimitives = 0;
			// 统计计数
			//++interiorNodes;
		}
		Bounds3f bounds;
		BVHBuildNode* children[2];
		int32 splitAxis, firstPrimOffset, nPrimitives;	// 分割轴，第一个prim的偏移，一共有多少数量的prim
		// 这个分割轴大概是给按空间划分的bvh准备的。
	};

	// 这个结构是用来提升遍历性能的。
	struct LinearBVHNode
	{
		Bounds3f bounds;
		union
		{
			int32 primitivesOffset;		// 叶子节点，prim的偏移
			int32 secondChildOffset;	// 内部节点，保存右子节点的偏移。因为左子节点肯定在父节点之后，不需要保存。
		};
		uint16 nPrimitives;	// prim的数量，内部节点是0，叶子节点才有意义
		uint8 axis; // 内部节点用到的按照哪个轴来分割的轴
		uint8 pad[1];	// 确保32字节的总空间。补足这个空间是为了在分配空间的时候，总是cache-line对齐的，不会偏到其他的cache-line去。
	};

	struct BucketInfo
	{
		int32 count = 0;
		Bounds3f bounds;
	};

	BVHAccel::~BVHAccel() { FreeAligned(nodes); }

	BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
		int32 maxPrimsInNode,
		SplitMethod splitMethod)
		: mMaxPrimsInNode(std::min(255, maxPrimsInNode)),
		mSplitMethod(splitMethod),
		mPrimitives(std::move(p))
	{
		if (mPrimitives.empty())
			return;

		std::vector<BVHPrimitiveInfo> primitiveInfo(mPrimitives.size());
		for (size_t i = 0; i < mPrimitives.size(); ++i)
			primitiveInfo[i] = { i, mPrimitives[i]->WorldBound() };

		int32 totalNodes = 0;
		std::vector<std::shared_ptr<Primitive>> orderedPrims;
		orderedPrims.reserve(mPrimitives.size());
		BVHBuildNode* root;
		if (splitMethod == SplitMethod::HLBVH)
			root = HLBVHBuild(primitiveInfo, &totalNodes, orderedPrims);
		else
			root = RecursiveBuild(primitiveInfo, 0, mPrimitives.size(), &totalNodes, orderedPrims);

		mPrimitives.swap(orderedPrims);
		primitiveInfo.resize(0);

		nodes = AllocAligned<LinearBVHNode>(totalNodes);
		int32 offset = 0;
		FlattenBVHTree(root, &offset);
		assert(totalNodes == offset);
	}

	Bounds3f BVHAccel::WorldBound() const
	{
		return nodes ? nodes[0].bounds : Bounds3f();
	}

	bool BVHAccel::Intersect(const Ray& ray, SurfaceInteraction* isect) const
	{
		if (!nodes) return false;

		bool hit = false;
		Vector3f invDir({ 1 / ray.d[0], 1 / ray.d[1], 1 / ray.d[2] });	// 这个是为了计算做的简化。因为包围盒的面都是垂直轴的。
		int32 dirIsNeg[3] = { invDir[0] < 0, invDir[1] < 0, invDir[2] < 0 };

		// 跟着ray穿过BVH的节点，找到与prim的交点。
		int32 toVisitOffset = 0, currentNodeIndex = 0;
		int32 nodesToVisit[64] = { 0 };
		while (true)
		{
			const LinearBVHNode* node = &nodes[currentNodeIndex];

			// 检查ray是否击中BVH节点。
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
			{
				if (node->nPrimitives > 0)
				{	// 相交的是叶子节点
					for (int32 i = 0; i < node->nPrimitives; ++i)
					{
						// 需要和每一个prim检查相交节点，是为了防止别的节点的交点比之前检测到的更近。
						// 所以，即便是已经找到交点，也要把改检查的节点都检查到。
						if (mPrimitives[node->primitivesOffset + i]->Intersect(ray, isect))
							hit = true;
					}
					if (toVisitOffset == 0) break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					// 如果方向是负的，那就先访问右子节点，再访问左子节点。
					// 因为右子树的prim位于分割点的上方。书上的原文是
					// on the upper side of the partition point, 但是我不太理解这个上方是什么意思。
					// 从一般的角度讲，右子节点里存放的是离坐标轴原点远的。所以如果方向是负的，那么会先
					// 接触到离原点远的节点，也就是右子节点。
					// 这是属于细节上的优化。
					if (dirIsNeg[node->axis])
					{
						nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
						currentNodeIndex = node->secondChildOffset;
					}
					else
					{
						nodesToVisit[toVisitOffset++] = node->secondChildOffset;
						currentNodeIndex = currentNodeIndex + 1;
					}
				}
			}
			else
			{
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
		}

		return hit;
	}

	bool BVHAccel::IntersectP(const Ray& ray) const
	{
		if (!nodes) return false;

		Vector3f invDir({ 1 / ray.d[0], 1 / ray.d[1], 1 / ray.d[2] });	// 这个是为了计算做的简化。因为包围盒的面都是垂直轴的。
		int32 dirIsNeg[3] = { invDir[0] < 0, invDir[1] < 0, invDir[2] < 0 };

		// 跟着ray穿过BVH的节点，找到与prim的交点。
		int32 toVisitOffset = 0, currentNodeIndex = 0;
		int32 nodesToVisit[64] = { 0 };
		while (true)
		{
			const LinearBVHNode* node = &nodes[currentNodeIndex];

			// 检查ray是否击中BVH节点。
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
			{
				if (node->nPrimitives > 0)
				{	// 相交的是叶子节点
					for (int32 i = 0; i < node->nPrimitives; ++i)
					{
						// 需要和每一个prim检查相交节点，是为了防止别的节点的交点比之前检测到的更近。
						// 所以，即便是已经找到交点，也要把改检查的节点都检查到。
						if (mPrimitives[node->primitivesOffset + i]->IntersectP(ray))
							return true;
					}
					if (toVisitOffset == 0) break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					// 如果方向是负的，那就先访问右子节点，再访问左子节点。
					// 因为右子树的prim位于分割点的上方。书上的原文是
					// on the upper side of the partition point, 但是我不太理解这个上方是什么意思。
					// 从一般的角度讲，右子节点里存放的是离坐标轴原点远的。所以如果方向是负的，那么会先
					// 接触到离原点远的节点，也就是右子节点。
					// 这是属于细节上的优化。
					if (dirIsNeg[node->axis])
					{
						nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
						currentNodeIndex = node->secondChildOffset;
					}
					else
					{
						nodesToVisit[toVisitOffset++] = node->secondChildOffset;
						currentNodeIndex = currentNodeIndex + 1;
					}
				}
			}
			else
			{
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
		}

		return false;
	}

	BVHBuildNode* BVHAccel::RecursiveBuild(std::vector<BVHPrimitiveInfo>& primitiveInfo,
		int32 start, int32 end, int32* totalNodes,
		std::vector<std::shared_ptr<Primitive>>& orderedPrims)
	{
		BVHBuildNode* node = new BVHBuildNode();
		(*totalNodes)++;

		// 计算这个BVH节点的所有prim的包围盒
		Bounds3f bounds;
		for (int32 i = start; i < end; ++i)
			bounds = Union(bounds, primitiveInfo[i].bounds);

		int32 nPrimitives = end - start;
		if (nPrimitives == 1)
		{	// 叶子节点
			int32 firstPrimOffset = orderedPrims.size();
			for (int32 i = start; i < end; ++i)
			{
				int32 primNum = primitiveInfo[i].primitiveNumber;
				orderedPrims.push_back(mPrimitives[primNum]);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else
		{
			// 计算prim的质心的包围盒，选择要分割的轴。
			Bounds3f centroidBounds;
			for (int32 i = start; i < end; ++i)
				centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
			int32 dim = centroidBounds.MaximumExtent();

			// 把prim分成两个部分，然后构建子节点
			int32 mid = (start + end) / 2;
			// 如果说质心的包围盒不是一个范围，说明这是一个叶子节点
			if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim])
			{	// 创建叶子节点
				int32 firstPrimOffset = orderedPrims.size();
				for (int32 i = start; i < end; ++i)
				{
					int32 primNum = primitiveInfo[i].primitiveNumber;
					orderedPrims.push_back(mPrimitives[primNum]);
				}
				node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
				return node;
			}
			else
			{
				// 基于分割方案来分割
				switch (mSplitMethod)
				{
				case SplitMethod::Middle:
				{

				}
				break;
				case SplitMethod::EqualCounts:
				{

				}
				break;
				case SplitMethod::SAH: // SURFACE AREA HEURISTIC，表面积启发式
				default:
				{
					// 用近似SAH方法分割prim
					if (nPrimitives <= 2)
					{
						// 把prim分成两部分，这很简单
						mid = (start + end) / 2;
						// https://en.cppreference.com/w/cpp/algorithm/nth_element
						std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
							&primitiveInfo[end - 1] + 1,
							[dim](const BVHPrimitiveInfo& a,
								const BVHPrimitiveInfo& b)
							{return a.centroid[dim] < b.centroid[dim]; });
					}
					else
					{
						constexpr int32 nBuckets = 12;
						BucketInfo buckets[nBuckets];

						// 初始化分区桶信息
						// 对每一个prim，确定其质心所在的桶，并且更新桶边界确保其包裹prim的边界。
						for (int32 i = start; i < end; ++i)
						{
							// 这个offset计算的是质心距离centroidBounds.pMin的偏移程度，它在pMin到pMax之间的哪个位置
							int32 b = nBuckets * centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							assert(b >= 0); assert(b < nBuckets);
							buckets[b].count++;
							buckets[b].bounds = Union(buckets[b].bounds, primitiveInfo[i].bounds);
						}

						// 用SAH计算分割之后每个桶的消耗，最后一个桶的消耗不用计算，但是我不理解为啥最后一个桶不用计算.
						// 应该这么想，分割成12个桶，一共11条分割线，对每一条分割线的左右进行计算。
						// 相交检测的估算值设置成1，遍历的估算值设置成1/8
						// 这个值在设计的时候就有讲究了，估计也就是在pbrt里设置成这样。
						float cost[nBuckets - 1];
						for (int32 i = 0; i < nBuckets - 1; ++i)
						{
							Bounds3f b0, b1;
							int32 count0 = 0, count1 = 0;
							for (int32 j = 0; j <= i; ++j)
							{
								b0 = Union(b0, buckets[j].bounds);
								count0 += buckets[j].count;
							}
							for (int32 j = i + 1; j < nBuckets; ++j)
							{
								b1 = Union(b1, buckets[j].bounds);
								count1 += buckets[j].count;
							}
							cost[i] = 0.125f +
								(count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) /
								bounds.SurfaceArea();
							// 这就和之前的消耗计算公式有点不同，按照之前的注释，遍历消耗设置成1/8，
							// 表示的是遍历的消耗。相交的消耗已经设置成了1.所以，count0和count1就
							// 是多少个prim的消耗和。概率就是表面积之比，这个很直观。
						}

						// 找到消耗最小的分割方案
						float minCost = cost[0];
						int32 minCostSplitBucket = 0;
						for (int32 i = 1; i < nBuckets - 1; ++i)
						{
							if (cost[i] < minCost)
							{
								minCost = cost[i];
								minCostSplitBucket = i;
							}
						}

						// 确定好分割方式之后，创建的可能是内部节点，也可能是一个叶子节点。
						// 如果叶子节点的消耗会比较小的话。
						float leafCost = nPrimitives;
						if (minCost < leafCost || nPrimitives > mMaxPrimsInNode)
						{	// 分割的消耗小于叶子节点的消耗。或者prim的数量超过设定的最大数量
							BVHPrimitiveInfo* pmid = std::partition(&primitiveInfo[start],
								&primitiveInfo[end - 1] + 1,
								[=](const BVHPrimitiveInfo& pi) {
									int32 b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							assert(b >= 0); assert(b < nBuckets);
							return b <= minCostSplitBucket;
								});
							mid = pmid - &primitiveInfo[0];
						}
						else
						{
							// 创建叶子节点
							int32 firstPrimOffset = orderedPrims.size();
							for (int32 i = start; i < end; ++i)
							{
								int32 primNum = primitiveInfo[i].primitiveNumber;
								orderedPrims.push_back(mPrimitives[primNum]);
							}
							node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
							return node;
						}
					}
					break;
				}
				}

				node->InitInterior(dim,
					RecursiveBuild(primitiveInfo, start, mid, totalNodes, orderedPrims),
					RecursiveBuild(primitiveInfo, mid, end, totalNodes, orderedPrims));
			}
		}

		return node;
	}

	BVHBuildNode* BVHAccel::HLBVHBuild(const std::vector<BVHPrimitiveInfo>& primitiveInfo,
		int* totalNodes,
		std::vector<std::shared_ptr<Primitive>>& orderedPrims) const
	{
		// TODO
		return nullptr;
	}

	int32 BVHAccel::FlattenBVHTree(BVHBuildNode* node, int32* offset)
	{
		LinearBVHNode* linearNode = &nodes[*offset];
		linearNode->bounds = node->bounds;
		int32 myOffset = (*offset)++;
		if (node->nPrimitives > 0)
		{
			assert(node->children[0] != nullptr && node->children[1] != nullptr);
			assert(node->nPrimitives < 65536);
			linearNode->primitivesOffset = node->firstPrimOffset;
			linearNode->nPrimitives = node->nPrimitives;
		}
		else
		{
			// 内部节点
			linearNode->axis = node->splitAxis;
			linearNode->nPrimitives = 0;
			FlattenBVHTree(node->children[0], offset);
			linearNode->secondChildOffset = FlattenBVHTree(node->children[1], offset);
		}
		return myOffset;
	}

	std::shared_ptr<BVHAccel> CreateBVHAccelerator(std::vector<std::shared_ptr<Primitive>> prims)
	{
		BVHAccel::SplitMethod splitMethod;

		splitMethod = BVHAccel::SplitMethod::SAH;
		int32 maxPrimsInNode = 4;
		return std::make_shared<BVHAccel>(std::move(prims), maxPrimsInNode, splitMethod);
	}
}