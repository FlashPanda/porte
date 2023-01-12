#include "Primitive.h"
#include "Interaction.h"
#include "Memory.h"
#include "Material.h"

namespace porte
{
	Primitive::~Primitive() {}

	GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape>& shape,
		const std::shared_ptr<Material>& material)
		: shape(shape),
		material(material)
	{
	}

	GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape>& shape,
		const std::string& matName)
		: shape(shape), matName(matName)
	{}

	Bounds3f GeometricPrimitive::WorldBound() const { return shape->WorldBound(); }

	bool GeometricPrimitive::IntersectP(const Ray& r) const {
		return shape->IntersectP(r);
	}

	bool GeometricPrimitive::Intersect(const Ray& r,
		SurfaceInteraction* isect) const {
		float tHit;
		if (!shape->Intersect(r, &tHit, isect)) return false;
		r.tMax = tHit;
		isect->primitive = this;

		return true;
	}

	const Material* GeometricPrimitive::GetMaterial() const {
		return material.get();
	}

	void GeometricPrimitive::ComputeScatteringFunctions(
		SurfaceInteraction* isect) const 
	{
		if (material)
			material->ComputeScatteringFunctions(isect);
	}

	void GeometricPrimitive::MaterialRedirection(const std::shared_ptr<Material>& material)
	{
		this->material = material;
	}

	const Material* Aggregate::GetMaterial() const
	{
		return nullptr;
	}

	void Aggregate::ComputeScatteringFunctions(SurfaceInteraction* isect) const 
	{
	}

}