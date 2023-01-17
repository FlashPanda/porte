#pragma once
#include <core/porte.h>
#include <string>
#include <map>
#include <tinyobjloader/tiny_obj_loader.h>

namespace porte
{
	bool ObjLoader(std::string fil, Scene* pScene, 
		const Transform* ObjectToWorld, const Transform* WorldToObject);
}