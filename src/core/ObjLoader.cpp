#include <core/ObjLoader.h>
#include <vector>
#include <core/Geometry.h>
#include <shapes/Triangle.h>
#include <core/Scene.h>

namespace porte
{
	std::vector<std::shared_ptr<Shape>> ObjLoader(std::string file,
		const Transform* ObjectToWorld, const Transform* WorldToObject)
	{
		bool  triangulate = true;	// 是否三角化
		tinyobj::attrib_t attrib;	// 所有数据都放这里
		std::vector<tinyobj::material_t>	materials;	// 材质
		std::vector<tinyobj::shape_t>	shapes;		// 所有的模型

		std::string warn;
		std::string err;

		bool bRead = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.c_str());

		if (!bRead)
		{
			std::cout << "Read file \"" << file << "\" failed! " << std::endl;
			return std::vector<std::shared_ptr<Shape>>();
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
			return std::vector<std::shared_ptr<Shape>>();
 		}

		std::string meshName;
		std::vector<Point3f> vertices;
		std::vector<Normal3f> normals;
		std::vector<int> indices;
		std::vector<Point2f> uvs;
		for (const tinyobj::shape_t& shape : shapes)
		{
			meshName = shape.name;

			for (const tinyobj::index_t& index : shape.mesh.indices)
			{
				Point3f vertex;
				vertex[0] = attrib.vertices[3 * index.vertex_index + 0];
				vertex[1] = attrib.vertices[3 * index.vertex_index + 1];
				vertex[2] = attrib.vertices[3 * index.vertex_index + 2];

				Normal3f normal;
				normal[0] = attrib.normals[3 * index.normal_index + 0];
				normal[1] = attrib.normals[3 * index.normal_index + 1];
				normal[2] = attrib.normals[3 * index.normal_index + 2];

				if (index.texcoord_index != -1)
				{
					Point2f uv;
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

		int nTriangles = vertices.size() / 3;
		int nVertices = vertices.size();

		std::vector<std::shared_ptr<Shape>> tShape = CreateTriangleMesh(ObjectToWorld, WorldToObject,
			false, nTriangles, &indices[0], nVertices, 
			&vertices[0], nullptr, &normals[0], uvs.size() > 0?  &uvs[0] : nullptr, nullptr, nullptr);

		return std::move(tShape);
	}
}
