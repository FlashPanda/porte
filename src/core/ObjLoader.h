#pragma once
#include <core/porte.h>
#include <string>
#include <map>
#include <tinyobjloader/tiny_obj_loader.h>
#include <core/Shape.h>

namespace porte
{
	std::vector<std::shared_ptr<Shape>> ObjLoader(std::string fil,
		const Transform* ObjectToWorld, const Transform* WorldToObject);
}