#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_ACCELERATORS_BVH_H
#define PORTE_ACCELERATORS_BVH_H

// accelerators/bvh.h*
#include <core/porte.h>
#include <core/Primitive.h>
#include <atomic>

namespace porte {
struct BVHBuildNode;


struct BVHPrimitiveInfo;
struct MortonPrimitive;
struct LinearBVHNode;

class BVHAccel : public Aggregate {
public:
    enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };

    BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::SAH);
    Bounds3f WorldBound() const;
    ~BVHAccel();
    bool Intersect(const Ray &ray, SurfaceInteraction *isect) const;
    bool IntersectP(const Ray &ray) const;

private:
    BVHBuildNode *recursiveBuild(
        MemoryArena &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo,
        int start, int end, int *totalNodes,
        std::vector<std::shared_ptr<Primitive>> &orderedPrims);
    BVHBuildNode *HLBVHBuild(
        MemoryArena &arena, const std::vector<BVHPrimitiveInfo> &primitiveInfo,
        int *totalNodes,
        std::vector<std::shared_ptr<Primitive>> &orderedPrims) const;
    BVHBuildNode *emitLBVH(
        BVHBuildNode *&buildNodes,
        const std::vector<BVHPrimitiveInfo> &primitiveInfo,
        MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes,
        std::vector<std::shared_ptr<Primitive>> &orderedPrims,
        std::atomic<int> *orderedPrimsOffset, int bitIndex) const;
    BVHBuildNode *buildUpperSAH(MemoryArena &arena,
                                std::vector<BVHBuildNode *> &treeletRoots,
                                int start, int end, int *totalNodes) const;
    int flattenBVHTree(BVHBuildNode *node, int *offset);

    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<std::shared_ptr<Primitive>> primitives;
    LinearBVHNode *nodes = nullptr;
};

std::shared_ptr<BVHAccel> CreateBVHAccelerator(
    std::vector<std::shared_ptr<Primitive>> prims);

}  // namespace porte

#endif  // PORTE_ACCELERATORS_BVH_H
