#pragma once

#include <string>
#include <map>
#include "tiny_obj_loader.h"
#include "SceneObjectMesh.h"
#include "SceneNodeMesh.h"

namespace panda
{
	bool ObjLoader(std::string file, std::shared_ptr<SceneObjectMesh>& pMesh, std::shared_ptr<SceneNodeMesh>& pNodeMesh);
}