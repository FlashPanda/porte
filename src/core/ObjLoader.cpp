#include "ObjLoader.h"
#include <vector>

namespace panda
{
	bool ObjLoader(std::string file, std::shared_ptr<SceneObjectMesh>& pMesh, std::shared_ptr<SceneNodeMesh>& pNodeMesh)
	{
		bool  triangulate = true;	// 是否三角化
		tinyobj::attrib_t attrib;	// 所有数据都放这里
		std::vector<tinyobj::shape_t>	shapes;		// 所有的模型
		std::vector<tinyobj::material_t>	materials;	// 材质

		std::string warn;
		std::string err;

		bool bRead = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.c_str());

		if (!bRead)
		{
			std::cout << "Read file \"" << file << "\" failed! " << std::endl;
			return false;
		}
		
		// 输出读取的模型信息
		std::cout << "file full path : " << file << std::endl;
		std::cout << "vertex count : " << attrib.vertices.size() / 3 << std::endl;
		std::cout << "normal count : " << attrib.normals.size() / 3 << std::endl;
		std::cout << "uv count : " << attrib.texcoords.size() / 2 << std::endl;
		std::cout << "shape count : " << shapes.size() << std::endl;
		std::cout << "material count : " << materials.size() << std::endl;

		if (attrib.normals.size() == 0)
		{
			std::cout << "file has no normals! " << std::endl;
			return false;
 		}


		//for (auto& index : shape.mesh.indices) {
		//	mesh.Data.emplace_back(attribs.vertices[3 * index.vertex_index + 0]);
		//	mesh.Data.emplace_back(attribs.vertices[3 * index.vertex_index + 1]);
		//	mesh.Data.emplace_back(attribs.vertices[3 * index.vertex_index + 2]);
		//	++offset;

		//	if (load_normals) {
		//		if (attribs.normals.size() == 0) {
		//			std::cout << "Could not load normal vectors data in the '" << filename << "' file.";
		//			return false;
		//		}
		//		else {
		//			mesh.Data.emplace_back(attribs.normals[3 * index.normal_index + 0]);
		//			mesh.Data.emplace_back(attribs.normals[3 * index.normal_index + 1]);
		//			mesh.Data.emplace_back(attribs.normals[3 * index.normal_index + 2]);
		//		}
		//	}

		std::string meshName;
		std::vector<Vector3Df> vertices;
		std::vector<Vector3Df> normals;
		std::vector<uint32> indices;
		std::vector<Vector2Df> uvs;
		for (const tinyobj::shape_t& shape : shapes)
		{
			meshName = shape.name;

			for (const tinyobj::index_t& index : shape.mesh.indices)
			{
				Vector3Df vertex;
				vertex[0] = attrib.vertices[3 * index.vertex_index + 0];
				vertex[1] = attrib.vertices[3 * index.vertex_index + 1];
				vertex[2] = attrib.vertices[3 * index.vertex_index + 2];

				Vector3Df normal;
				normal[0] = attrib.normals[3 * index.normal_index + 0];
				normal[1] = attrib.normals[3 * index.normal_index + 1];
				normal[2] = attrib.normals[3 * index.normal_index + 2];

				if (index.texcoord_index != -1)
				{
					Vector2Df uv;
					uv[0] = attrib.texcoords[2 * index.texcoord_index + 0];
					uv[1] = attrib.texcoords[2 * index.texcoord_index + 1];
					uvs.push_back(uv);
				}

				vertices.push_back(vertex);
				normals.push_back(normal);
				indices.push_back(index.vertex_index);
			}
		}

		if (meshName.empty())
			meshName = "MainMesh";
		pMesh = std::make_shared<SceneObjectMesh>(meshName);
		pMesh->AddVertexArray(std::move(vertices));
		pMesh->AddNormalArray(std::move(normals));
		pMesh->AddIndexArray(std::move(indices));
		pMesh->AddUVArray(std::move(uvs));

		pNodeMesh = std::make_shared<SceneNodeMesh>(meshName);
		// ignore material currently.
		// pMesh->SetMaterial("materialname");

		return true;
	}
}
