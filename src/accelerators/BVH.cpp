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
			// ͳ�Ƽ�����Ҳ��ʵ�ֵ�ϸ�ڡ�
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
			// ͳ�Ƽ���
			//++interiorNodes;
		}
		Bounds3f bounds;
		BVHBuildNode* children[2];
		int32 splitAxis, firstPrimOffset, nPrimitives;	// �ָ��ᣬ��һ��prim��ƫ�ƣ�һ���ж���������prim
		// ����ָ������Ǹ����ռ仮�ֵ�bvh׼���ġ�
	};

	// ����ṹ�����������������ܵġ�
	struct LinearBVHNode
	{
		Bounds3f bounds;
		union
		{
			int32 primitivesOffset;		// Ҷ�ӽڵ㣬prim��ƫ��
			int32 secondChildOffset;	// �ڲ��ڵ㣬�������ӽڵ��ƫ�ơ���Ϊ���ӽڵ�϶��ڸ��ڵ�֮�󣬲���Ҫ���档
		};
		uint16 nPrimitives;	// prim���������ڲ��ڵ���0��Ҷ�ӽڵ��������
		uint8 axis; // �ڲ��ڵ��õ��İ����ĸ������ָ����
		uint8 pad[1];	// ȷ��32�ֽڵ��ܿռ䡣��������ռ���Ϊ���ڷ���ռ��ʱ������cache-line����ģ�����ƫ��������cache-lineȥ��
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
		Vector3f invDir({ 1 / ray.d[0], 1 / ray.d[1], 1 / ray.d[2] });	// �����Ϊ�˼������ļ򻯡���Ϊ��Χ�е��涼�Ǵ�ֱ��ġ�
		int32 dirIsNeg[3] = { invDir[0] < 0, invDir[1] < 0, invDir[2] < 0 };

		// ����ray����BVH�Ľڵ㣬�ҵ���prim�Ľ��㡣
		int32 toVisitOffset = 0, currentNodeIndex = 0;
		int32 nodesToVisit[64] = { 0 };
		while (true)
		{
			const LinearBVHNode* node = &nodes[currentNodeIndex];

			// ���ray�Ƿ����BVH�ڵ㡣
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
			{
				if (node->nPrimitives > 0)
				{	// �ཻ����Ҷ�ӽڵ�
					for (int32 i = 0; i < node->nPrimitives; ++i)
					{
						// ��Ҫ��ÿһ��prim����ཻ�ڵ㣬��Ϊ�˷�ֹ��Ľڵ�Ľ����֮ǰ��⵽�ĸ�����
						// ���ԣ��������Ѿ��ҵ����㣬ҲҪ�Ѹļ��Ľڵ㶼��鵽��
						if (mPrimitives[node->primitivesOffset + i]->Intersect(ray, isect))
							hit = true;
					}
					if (toVisitOffset == 0) break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					// ��������Ǹ��ģ��Ǿ��ȷ������ӽڵ㣬�ٷ������ӽڵ㡣
					// ��Ϊ��������primλ�ڷָ����Ϸ������ϵ�ԭ����
					// on the upper side of the partition point, �����Ҳ�̫�������Ϸ���ʲô��˼��
					// ��һ��ĽǶȽ������ӽڵ����ŵ�����������ԭ��Զ�ġ�������������Ǹ��ģ���ô����
					// �Ӵ�����ԭ��Զ�Ľڵ㣬Ҳ�������ӽڵ㡣
					// ��������ϸ���ϵ��Ż���
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

		Vector3f invDir({ 1 / ray.d[0], 1 / ray.d[1], 1 / ray.d[2] });	// �����Ϊ�˼������ļ򻯡���Ϊ��Χ�е��涼�Ǵ�ֱ��ġ�
		int32 dirIsNeg[3] = { invDir[0] < 0, invDir[1] < 0, invDir[2] < 0 };

		// ����ray����BVH�Ľڵ㣬�ҵ���prim�Ľ��㡣
		int32 toVisitOffset = 0, currentNodeIndex = 0;
		int32 nodesToVisit[64] = { 0 };
		while (true)
		{
			const LinearBVHNode* node = &nodes[currentNodeIndex];

			// ���ray�Ƿ����BVH�ڵ㡣
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
			{
				if (node->nPrimitives > 0)
				{	// �ཻ����Ҷ�ӽڵ�
					for (int32 i = 0; i < node->nPrimitives; ++i)
					{
						// ��Ҫ��ÿһ��prim����ཻ�ڵ㣬��Ϊ�˷�ֹ��Ľڵ�Ľ����֮ǰ��⵽�ĸ�����
						// ���ԣ��������Ѿ��ҵ����㣬ҲҪ�Ѹļ��Ľڵ㶼��鵽��
						if (mPrimitives[node->primitivesOffset + i]->IntersectP(ray))
							return true;
					}
					if (toVisitOffset == 0) break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					// ��������Ǹ��ģ��Ǿ��ȷ������ӽڵ㣬�ٷ������ӽڵ㡣
					// ��Ϊ��������primλ�ڷָ����Ϸ������ϵ�ԭ����
					// on the upper side of the partition point, �����Ҳ�̫�������Ϸ���ʲô��˼��
					// ��һ��ĽǶȽ������ӽڵ����ŵ�����������ԭ��Զ�ġ�������������Ǹ��ģ���ô����
					// �Ӵ�����ԭ��Զ�Ľڵ㣬Ҳ�������ӽڵ㡣
					// ��������ϸ���ϵ��Ż���
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

		// �������BVH�ڵ������prim�İ�Χ��
		Bounds3f bounds;
		for (int32 i = start; i < end; ++i)
			bounds = Union(bounds, primitiveInfo[i].bounds);

		int32 nPrimitives = end - start;
		if (nPrimitives == 1)
		{	// Ҷ�ӽڵ�
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
			// ����prim�����ĵİ�Χ�У�ѡ��Ҫ�ָ���ᡣ
			Bounds3f centroidBounds;
			for (int32 i = start; i < end; ++i)
				centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
			int32 dim = centroidBounds.MaximumExtent();

			// ��prim�ֳ��������֣�Ȼ�󹹽��ӽڵ�
			int32 mid = (start + end) / 2;
			// ���˵���ĵİ�Χ�в���һ����Χ��˵������һ��Ҷ�ӽڵ�
			if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim])
			{	// ����Ҷ�ӽڵ�
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
				// ���ڷָ�����ָ�
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
				case SplitMethod::SAH: // SURFACE AREA HEURISTIC�����������ʽ
				default:
				{
					// �ý���SAH�����ָ�prim
					if (nPrimitives <= 2)
					{
						// ��prim�ֳ������֣���ܼ�
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

						// ��ʼ������Ͱ��Ϣ
						// ��ÿһ��prim��ȷ�����������ڵ�Ͱ�����Ҹ���Ͱ�߽�ȷ�������prim�ı߽硣
						for (int32 i = start; i < end; ++i)
						{
							// ���offset����������ľ���centroidBounds.pMin��ƫ�Ƴ̶ȣ�����pMin��pMax֮����ĸ�λ��
							int32 b = nBuckets * centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							assert(b >= 0); assert(b < nBuckets);
							buckets[b].count++;
							buckets[b].bounds = Union(buckets[b].bounds, primitiveInfo[i].bounds);
						}

						// ��SAH����ָ�֮��ÿ��Ͱ�����ģ����һ��Ͱ�����Ĳ��ü��㣬�����Ҳ����Ϊɶ���һ��Ͱ���ü���.
						// Ӧ����ô�룬�ָ��12��Ͱ��һ��11���ָ��ߣ���ÿһ���ָ��ߵ����ҽ��м��㡣
						// �ཻ���Ĺ���ֵ���ó�1�������Ĺ���ֵ���ó�1/8
						// ���ֵ����Ƶ�ʱ����н����ˣ�����Ҳ������pbrt�����ó�������
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
							// ��ͺ�֮ǰ�����ļ��㹫ʽ�е㲻ͬ������֮ǰ��ע�ͣ������������ó�1/8��
							// ��ʾ���Ǳ��������ġ��ཻ�������Ѿ����ó���1.���ԣ�count0��count1��
							// �Ƕ��ٸ�prim�����ĺ͡����ʾ��Ǳ����֮�ȣ������ֱ�ۡ�
						}

						// �ҵ�������С�ķָ��
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

						// ȷ���÷ָʽ֮�󣬴����Ŀ������ڲ��ڵ㣬Ҳ������һ��Ҷ�ӽڵ㡣
						// ���Ҷ�ӽڵ�����Ļ�Ƚ�С�Ļ���
						float leafCost = nPrimitives;
						if (minCost < leafCost || nPrimitives > mMaxPrimsInNode)
						{	// �ָ������С��Ҷ�ӽڵ�����ġ�����prim�����������趨���������
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
							// ����Ҷ�ӽڵ�
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
			// �ڲ��ڵ�
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