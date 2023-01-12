#pragma once
#include "Bounds.h"
#include "Shape.h"

namespace porte
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
}