#include <fstream>
#include <filesystem>	// c++ 17
#include <iostream>
#include <core/Scene.h>
#include <integrators/Whitted.h>
#include <core/Transform.h>
#include <filters/Box.h>
#include <core/Film.h>
#include <samplers/Random.h>
#include <cameras/Perspective.h>
#include <materials/Matte.h>
#include <core/Spectrum.h>
#include <lights/Point.h>
#include <core/ObjLoader.h>
#include <shapes/Triangle.h>
#include <core/Primitive.h>
#include <accelerators/BVH.h>

namespace porte
{
	void Scene::LoadSceneFromFile(std::string filename)
	{
		std::size_t pos = filename.rfind('.');
		if (pos == std::string::npos)
		{
			std::cout << "[ERROR]: Loading scene from file. There is no suffix in the filename string." << std::endl;
			return;
		}

		std::string suffix = filename.substr(pos);
		if (suffix == ".xml")
		{
			ParseXmlScene(filename);

		}
		else
		{
			std::cout << "Invalid filename. Only \"xml\" file is supported so far." << std::endl;
		}
	}

	void Scene::Render()
	{
		mIntegrator->Render(*this);
	}

	bool Scene::Intersect(const Ray& ray, SurfaceInteraction* isect) const
	{
		return mAggregate? mAggregate->Intersect(ray, isect) : false;
	}

	// 是否相交
	bool Scene::IntersectP(const Ray& ray) const
	{
		return mAggregate? mAggregate->IntersectP(ray) : false;
	}

	void Scene::ParseXmlScene(const std::string filename)
	{
		/// c++ 17
		// 文件检验
		const std::filesystem::path filepath(filename);
		if (!std::filesystem::exists(filepath))
		{
			std::cout << "[ERROR]: the file '" << filename << "' does not exist!" << std::endl;
			return;
		}

		std::size_t pos = filename.rfind('.');
		if (pos == std::string::npos)
		{
			std::cout << "[ERROR]: Loading scene from file. There is no suffix in the '" << filename
				<< "' string." << std::endl;
			return;
		}

		std::string suffix = filename.substr(pos);
		if (suffix != ".xml")
		{
			std::cout << "[ERROR]: Input file name error. Only 'xml' format is supported." << std::endl;
			return;
		}

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(filename.c_str(),
			pugi::parse_default | pugi::parse_comments);

		if (!result)
		{
			std::cout << "Load xml file error." << std::endl;
			return;
		}

		pugi::xml_node root = doc.document_element();
		if (std::string(root.name()) != "scene")
		{
			std::cout << "root node must be \"scene\"" << std::endl;
			return;
		}

		OutputNode(root);

		for (auto& ch : root.children())
		{
			OutputNode(ch);

			if (std::string(ch.name()) == "path")
				ParseXmlPath(ch, filename);
			else if (std::string(ch.name()) == "integrator")
				ParseXmlIntegrator(ch);
			else if (std::string(ch.name()) == "sampler")
				ParseXmlSampler(ch);
			else if (std::string(ch.name()) == "camera")
				ParseXmlCamera(ch);
			else if (std::string(ch.name()) == "shape")
				ParseXmlShape(ch);
			else if (std::string(ch.name()) == "material")
				ParseXmlMaterial(ch);
			else if (std::string(ch.name()) == "light")
				ParseXmlLight(ch);
			else
				std::cout << "Invalid node \"" << ch.name() << "\"" << std::endl;
		}

		// 创建加速结构
		mAggregate = CreateBVHAccelerator(mPrimitives);

		CheckScene();
	}

	void Scene::ParseXmlPath(const pugi::xml_node& node, const std::string& filename)
	{
		// 当前exe运行的目录
		std::filesystem::path currentPath = std::filesystem::current_path();

		// 输入的filename的所在路径
		size_t pos = filename.find_last_of('\\');
		if (pos == std::string::npos)
		{
			pos = filename.find_last_of('/');
		}
		std::filesystem::path targetPath(filename.substr(0, pos));

		pugi::xml_attribute pathAttr = node.attribute("value");
		std::string pathStr = pathAttr.as_string();
		mSearchPaths.push_back(currentPath.append(pathStr));
		mSearchPaths.push_back(targetPath.append(pathStr));

		// 输出搜索路径
		for (int32_t i = 0; i < mSearchPaths.size(); ++i)
		{
			std::cout << "Search path : " << mSearchPaths[i].string() << std::endl;
		}
	}

	void Scene::ParseXmlIntegrator(const pugi::xml_node& node)
	{
		pugi::xml_attribute attr = node.attribute("type");
		if (!attr.empty())
		{
			if (std::string(attr.value()) == "directlighting")
			{
				//int32 nEmitterSamples = 1;
				//int32 nBsdfSamples = 0;
				//for (const pugi::xml_node& ch : node.children())
				//{
				//	if (std::string(ch.name()) == "integer")
				//	{
				//		std::pair<std::string, int32> spair = ParseXmlInt(ch);
				//		if (spair.first == "emitter_samples")
				//		{
				//			nEmitterSamples = spair.second;
				//		}
				//		else if (spair.first == "bsdf_samples")
				//		{
				//			nBsdfSamples = spair.second;
				//		}
				//	}
				//}

				//mIntegrator = CreateDirectLightingIntegrator(LightStrategy::UniformSampleOne, 5,
				//	std::shared_ptr<Sampler>(mSampler), mMainCamera);
			}
			else if (std::string(attr.value()) == "whitted")
			{
				mIntegrator = CreateWhittedIntegrator(mSampler, mCamera);
			}
			else
			{
				std::cout << "integrator type is not valid! " << std::endl;
			}
		}
		else
		{
			// 默认用Direct积分器
			//CreateDirectLightingIntegrator(LightStrategy::UniformSampleOne, 5,
			//	std::shared_ptr<Sampler>(mSampler), mMainCamera);
			std::cout << "[ERROR] no integrator!" << std::endl;
		}
	}

	Film* Scene::ParseXmlFilm(const pugi::xml_node& node, std::unique_ptr<Filter> inFilter)
	{
		Vector2i res({1280, 720});
		pugi::xml_attribute widthAttr = node.attribute("width");
		if (!widthAttr.empty())
		{
			res[0] = widthAttr.as_int();
		}
		else
		{
			std::cout << "Film width property is empty. Set default width (1280). " << std::endl;
			res[0] = 1280;
		}

		pugi::xml_attribute heightAttr = node.attribute("height");
		if (!heightAttr.empty())
		{
			res[1] = heightAttr.as_int();
		}
		else
		{
			std::cout << "Film height property is empty. Set default height (720). " << std::endl;
			res[1] = 720;
		}

		return CreateFilm(res, std::move(inFilter));
	}

	Filter* Scene::ParseXmlFilter(const pugi::xml_node& node)
	{
		pugi::xml_attribute typeAttr = node.attribute("type");
		if (typeAttr.empty())
		{
			std::cout << "Filter type property is invalid. Set default filter : box. " << std::endl;
			return CreateBoxFilter();
		}

		if (std::string(typeAttr.as_string()) == "box")
		{
			// Box filter
			return CreateBoxFilter();
		}
		else
		{
			std::cout << "Filter type is invalid. Set default to \"box\" filter. " << std::endl;
			return CreateBoxFilter();
		}
	}

	void Scene::ParseXmlSampler(const pugi::xml_node& node)
	{
		pugi::xml_attribute typeAttr = node.attribute("type");
		if (typeAttr.empty())
		{
			std::cout << "Sampler type property is empty. set default to random. " << std::endl;
			mSampler = std::make_shared<RandomSampler>(4);
			return;
		}

		if (std::string(typeAttr.as_string()) == "random")
		{
			pugi::xml_attribute sc = node.attribute("sample_count");
			if (sc.empty())
			{
				std::cout << "Sampler sample_count property is empty. set default to 4" << std::endl;
				mSampler = std::make_shared<RandomSampler>(4);
				return;
			}
			else
			{
				mSampler = std::make_shared<RandomSampler>(4);
				return;
			}
		}
	}

	void Scene::ParseXmlCamera(const pugi::xml_node& node)
	{
		pugi::xml_attribute typeAttr = node.attribute("type");
		if (typeAttr.empty() || std::string(typeAttr.value()) != "perspective")
		{
			std::cout << "camera has no \"perspective\" camera! " << std::endl;
			return;
		}

		std::string cameraName("MainCamera");
		Float fov = 90.f;
		Transform trans;

		pugi::xml_node propertyNode = node.child("property");
		if (propertyNode.empty())
		{
			std::cout << "Camera property node empty. Set to default name \"MainCamera\". " << std::endl;
			std::cout << "	Set to default near clip 0.1f. Set to default far clip 1000.f." << std::endl;
		}
		else 
		{
			pugi::xml_attribute attr = propertyNode.attribute("name");
			if (attr.empty())
				std::cout << "Camera poperty \"name\" is not found. Set to default name \"MainCamera\". " << std::endl;
			else 
				cameraName = attr.as_string();

			attr = propertyNode.attribute("fov");
			if (attr.empty())
				std::cout << "Camera property \"near_clip\" is not found. Set to default value 0.1f. " << std::endl;
			else 
				fov = attr.as_float();
		}

		pugi::xml_node transformNode = node.child("transform");
		if (transformNode.empty())
			std::cout << "Camera child \"transform\" is not found. Set to idendity transformation. " << std::endl;
		else
		{
			std::pair<std::string, Transform> tPair = ParseXmlTransform(transformNode);
			trans = tPair.second;
		}
		

		// Filter
		Filter* tFilter = nullptr;
		pugi::xml_node filterNode = node.child("filter");
		if (filterNode.empty())
		{
			std::cout << "Camera node has no filter child node. Set default to \"box\" filter." << std::endl;
			tFilter = CreateBoxFilter();
		}
		else
		{
			tFilter = ParseXmlFilter(filterNode);
		}

		// Film
		Film* tFilm = nullptr;
		pugi::xml_node filmNode = node.child("film");
		if (filmNode.empty())
		{
			std::cout << "Camera node has no \"film\" child node. Set Default to (1280,720). " << std::endl;
			tFilm = CreateFilm(Vector2i({ 1280, 720 }), std::unique_ptr<Filter>(tFilter));
		}
		else
		{
			tFilm = ParseXmlFilm(filmNode, std::unique_ptr<Filter>(tFilter));
		}
		

		// 创建相机对象与节点
		mCamera = std::shared_ptr<Camera>(CreatePerspectiveCamera(trans, tFilm, fov));
	}

	void Scene::ParseXmlShape(const pugi::xml_node& node)
	{
		static int32_t shapeIndex = 1;

		// 解析shape名
		std::string shapeName;
		pugi::xml_node nodename = node.child("name");
		if (nodename.empty())
		{
			std::cout << "shape node has no name! Use default index {" << shapeIndex << "}. " << std::endl;
			char nameBuf[256] = { 0 };
			sprintf_s(nameBuf, sizeof(nameBuf), "DefaultShape%d", shapeIndex);
			shapeName = nameBuf;
			++shapeIndex;
		}
		else
		{
			pugi::xml_attribute attr = nodename.attribute("value");
			shapeName = attr.as_string();
			if (shapeName.empty())
			{
				std::cout << "shape node has no name! Use default index {" << shapeIndex << "}. " << std::endl;
				char nameBuf[256] = { 0 };
				sprintf_s(nameBuf, sizeof(nameBuf), "DefaultShape%d", shapeIndex);
				shapeName = nameBuf;
				++shapeIndex;
			}
		}

		// 解析变换
		pugi::xml_node transformNode = node.child("transform");
		Transform trans({
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			});
		if (!transformNode.empty())
		{
			//std::pair<std::string, Matrix4f> tPair = ParseXmlTransform(transformNode);
			//pNodeMesh->AppendTransform(tPair.second);
		}

		// 解析引用的材质名
		pugi::xml_node refMaterialNode = node.child("ref_material");
		std::string matName;
		if (refMaterialNode.empty())
		{
			std::cout << "shape node has no ref_material child. Set default to \"default\" material. " << std::endl;
			matName = "default";
		}
		else
		{
			pugi::xml_attribute attrMat = refMaterialNode.attribute("name");
			if (attrMat.empty())
			{
				std::cout << "shape child ref_material child has no \"name\" property. Set default to \"default\" material. " << std::endl;
				matName = "default";
			}
			else
			{
				matName = attrMat.as_string();
			}
		}

		// 解析type
		pugi::xml_attribute typeAttr = node.attribute("type");
		if (typeAttr.empty())
		{
			std::cout << "Shape node has no \"type\" attribute. " << std::endl;
			return;
		}
		std::string type = typeAttr.as_string();
		if (type == "obj" || type == "ply")
		{
			std::vector<std::string> fullPaths;
			pugi::xml_node filename = node.child("filename");
			if (filename.empty())
			{
				std::cout << "Shape node has no \"filename\" child node! " << std::endl;
				return;
			}
			else
			{
				pugi::xml_attribute attr = filename.attribute("path");
				if (attr.empty())
				{
					std::cout << "Shape filename node has no \"path\" attribute. " << std::endl;
					return;
				}

				std::string path = attr.as_string();
				if (path.find_first_of(':') == std::string::npos)
				{	// 没有冒号判定为相对路径
					for (auto it : mSearchPaths)
					{
						it.append(path);
						fullPaths.push_back(it.string());
					}
				}
				else
				{	// 有冒号就是绝对路径
					fullPaths.push_back(path);
				}
			}

			// 搜索路径解析模型
			for (int32_t i = 0; i < fullPaths.size(); ++i)
			{
				if (std::filesystem::exists(fullPaths[i]))
				{
					if (type == "obj")
					{
						std::vector<std::shared_ptr<Shape>> tShapes = ObjLoader(fullPaths[i], &trans, &Inverse(trans));

						mShapes.insert(mShapes.end(), tShapes.begin(), tShapes.end());

						// 创建Primitive
						std::vector<std::shared_ptr<Primitive>> prims;
						prims.reserve(tShapes.size());
						for (auto it : tShapes)
						{
							prims.push_back(std::make_shared<GeometricPrimitive>(it, mMaterials[matName]));
						}
						mPrimitives.insert(mPrimitives.end(), prims.begin(), prims.end());
					}
					else if (type == "ply")
					{

					}
				}
			}
		}
		else if (type == "trianglemesh")
		{
			// points
			pugi::xml_node pointsNode = node.child("points");
			if (pointsNode.empty())
			{
				std::cout << "triangle mesh has no points child node! " << std::endl;
				return;
			}

			pugi::xml_attribute pointsAttr = pointsNode.attribute("value");
			if (pointsAttr.empty())
			{
				std::cout << "triangle mesh has no points attr value! " << std::endl;
				return;
			}

			std::vector<Point3f> points;
			{
				std::string str(pointsAttr.as_string());
				std::vector<std::string> tokens = Split(str, " ");
				if (tokens.size() < 9)
				{
					std::cout << "triangle mesh has less than 3 points! " << std::endl;
					return;
				}

				if (tokens.size() % 3 != 0)
				{
					std::cout << "triangle mesh points value can't be evenly divisible by 3! " << std::endl;
					return;
				}

				int nPoints = tokens.size() / 3;
				for (int i = 0; i < nPoints; ++i)
				{
					Point3f pos;
					pos[0] = std::atof(tokens[i * 3 + 0].c_str());
					pos[1] = std::atof(tokens[i * 3 + 1].c_str());
					pos[2] = std::atof(tokens[i * 3 + 2].c_str());
					points.push_back(std::move(pos));
				}
			}

			// normals 不允许没有法线
			pugi::xml_node normalsNode = node.child("normals");
			if (normalsNode.empty())
			{
				std::cout << "triangle mesh has no normals child node! " << std::endl;
				return;
			}

			pugi::xml_attribute normalsAttr = normalsNode.attribute("value");
			if (normalsAttr.empty())
			{
				std::cout << "triangle mesh has no normals attr value! " << std::endl;
				return;
			}

			std::vector<Normal3f> normals;
			{
				std::string str(normalsAttr.as_string());
				std::vector<std::string> tokens = Split(str, " ");
				
				if (tokens.size() / 3 != points.size())
				{
					std::cout << "normals count is not the same as points count!" << std::endl;
					return;
				}

				int nNormals = tokens.size() / 3;
				for (int i = 0; i < nNormals; ++i)
				{
					Normal3f normal;
					normal[0] = std::atof(tokens[i * 3 + 0].c_str());
					normal[1] = std::atof(tokens[i * 3 + 1].c_str());
					normal[2] = std::atof(tokens[i * 3 + 2].c_str());
					normals.push_back(std::move(normal));
				}
			}

			// uvs （允许没有uv，但是如果要有，比如和顶点数量一致）
			pugi::xml_node uvsNode = node.child("uvs");
			std::vector<Point2f> uvs;
			if (uvsNode.empty())
			{
				std::cout << "triangle mesh has no uvs child node! Skip uvs." << std::endl;
			}
			else
			{
				pugi::xml_attribute uvsAttr = uvsNode.attribute("value");
				if (uvsAttr.empty())
				{
					std::cout << "triangle mesh has no uvs values! Skip uvs." << std::endl;
				}
				else
				{
					std::string str(uvsAttr.as_string());
					std::vector<std::string> tokens = Split(str, " ");
					if (tokens.size() / 2 != points.size())
					{
						std::cout << "uvs count is not the same as points count!" << std::endl;
						return;
					}

					int count = tokens.size() / 2;
					for (int i = 0; i < count; ++i)
					{
						Point2f uv;
						uv[0] = std::atof(tokens[i * 2 + 0].c_str());
						uv[1] = std::atof(tokens[i * 2 + 1].c_str());
						uvs.push_back(uv);
					}
				}
			}

			// indices
			pugi::xml_node indicesNode = node.child("indices");
			if (indicesNode.empty())
			{
				std::cout << "triangle mesh has no indices child node! " << std::endl;
				return;
			}

			pugi::xml_attribute indicesAttr = indicesNode.attribute("value");
			if (indicesAttr.empty())
			{
				std::cout << "triangle mesh has no indices value! " << std::endl;
				return;
			}

			std::vector<int> indices;
			{
				std::string str(indicesAttr.as_string());
				std::vector<std::string> tokens = Split(str, " ");
				if (tokens.size() < 3)
				{
					std::cout << "triangle mesh indices has less than 3 indices! " << std::endl;
					return;
				}

				if (tokens.size() % 3 != 0)
				{
					std::cout << "triangle mesh indices count can't be evenly divisible by 3! " << std::endl;
					return;
				}

				for (int i = 0; i < tokens.size(); ++i)
				{
					indices.push_back(std::atoi(tokens[i].c_str()));
				}
			}

			Transform trans;

			int nTriangles = indices.size() / 3;
			std::vector<Point3f> vertices;
			std::vector<Normal3f> normalss;
			std::vector<Point2f> uvss;

			for (int i = 0; i < indices.size(); ++i)
			{
				vertices.push_back(points[indices[i]]);
				normalss.push_back(normals[indices[i]]);
				uvss.push_back(uvs[indices[i]]);
			}

			std::vector<std::shared_ptr<Shape>> tShape = CreateTriangleMesh(&trans, &Inverse(trans),
				false, nTriangles, &indices[0], nTriangles,
				&vertices[0], nullptr, &normals[0], uvss.size() > 0?  &uvss[0] : nullptr, nullptr, nullptr);

			mShapes.insert(mShapes.end(), tShape.begin(), tShape.end());

			// 创建Primitive
			std::vector<std::shared_ptr<Primitive>> prims;
			prims.reserve(tShape.size());
			for (auto it : tShape)
			{
				prims.push_back(std::make_shared<GeometricPrimitive>(it, mMaterials[matName]));
			}
			mPrimitives.insert(mPrimitives.end(), prims.begin(), prims.end());
		}
		else
		{
			std::cout << "shape type invalid!" << std::endl;
		}
	}

	void Scene::ParseXmlMaterial(const pugi::xml_node& node)
	{
		std::string matName;
		pugi::xml_attribute nameAttr = node.attribute("name");
		if (nameAttr.empty())
		{
			std::cout << "Material node has no name. Error!" << std::endl;
			return;
		}
		else
		{
			matName = nameAttr.as_string();
		}

		pugi::xml_attribute typeAttr = node.attribute("type");
		if (typeAttr.empty())
		{
			std::cout << "Material node has no type. Error! " << std::endl;
			return;
		}
		else
		{
			if (std::string(typeAttr.as_string()) == "matte")
			{
				pugi::xml_attribute kdAttr = node.attribute("Kd");
				Vector3f kd(1.f, 1.f, 1.f);
				if (!kdAttr.empty())
				{
					kd = ParseVectorString(kdAttr.as_string("1.f 1.f 1.f"));
				}

				Spectrum sp = RGBSpectrum::FromRGB(&kd[0]);
				std::shared_ptr<Material> pMaterial(CreateMatteMaterial(sp));
				mMaterials.emplace(matName, pMaterial);
			}
			else
			{
				std::cout << "unrecognized material type." << std::endl;
			}
		}
	}

	void Scene::ParseXmlLight(const pugi::xml_node& node)
	{
		static int32_t lightIndex = 1;
		pugi::xml_attribute typeAttr = node.attribute("type");
		if (typeAttr.empty())
		{
			std::cout << "light node has no type attribute! " << std::endl;
			return;
		}
		else
		{
			if (std::string(typeAttr.as_string()) == "point")
			{
				// 名字
				std::string name;
				pugi::xml_attribute nameAttr = node.attribute("name");
				if (nameAttr.empty())
				{
					std::cout << "light node has no name attribute. Set default to \"pointlight" << lightIndex << "\". " << std::endl;
					char buff[100];
					snprintf(buff, sizeof(buff), "pointlight%d", lightIndex);
					name = buff;
				}
				else
				{
					name = nameAttr.as_string();
				}

				// 位置
				Vector3f pos;
				pugi::xml_attribute posAttr = node.attribute("position");
				if (posAttr.empty())
				{
					std::cout << "light node has no position attribute. Set default to origin. " << std::endl;
					pos.x = pos.y = pos.z = 0.f;
				}
				else
				{
					pos = ParseVectorString(posAttr.as_string());
				}

				// 颜色
				Vector3f color(1.f, 1.f, 1.f);
				pugi::xml_attribute colorAttr = node.attribute("color");
				if (!colorAttr.empty())
				{
					color = ParseVectorString(colorAttr.as_string());
				}
				Float values[3] = {color.x, color.y, color.z};

				// 强度
				float intensity = 1.f;
				pugi::xml_attribute intensityAttr = node.attribute("intensity");
				if (!intensityAttr.empty())
				{
					intensity = intensityAttr.as_float(1.f);
				}

				Spectrum s = RGBSpectrum::FromRGB(values);
				s *= intensity;

				mLights.push_back(CreatePointLight(Translate(pos), s));
			}
		}
	}

	Vector3f Scene::ParseVectorString(const std::string& str)
	{
		std::vector<std::string> vecs = Split(str);

		if (vecs.size() != 3)
		{
			return Vector3f(1.f, 1.f, 1.f);
		}
		else
		{
			return Vector3f(std::stof(vecs[0]), std::stof(vecs[1]), std::stof(vecs[2]));
		}
	}

	std::pair<std::string, std::string> Scene::ParseXmlString(const pugi::xml_node& node)
	{
		pugi::xml_attribute name = node.first_attribute();
		pugi::xml_attribute value = node.last_attribute();
		if (name.empty() || value.empty() ||
			std::string(name.name()) != "name" || std::string(value.name()) != "value")
		{
			std::cout << "string node is invalid!" << std::endl;
			return std::pair<std::string, std::string>("", "");
		}

		return std::pair<std::string, std::string>(name.value(), value.value());
	}

	std::pair<std::string, int> Scene::ParseXmlInt(const pugi::xml_node& node)
	{
		pugi::xml_attribute name = node.first_attribute();
		pugi::xml_attribute value = node.last_attribute();
		if (name.empty() || value.empty() ||
			std::string(name.name()) != "name" || std::string(value.name()) != "value")
		{
			std::cout << "int node is invalid!" << std::endl;
			return std::pair<std::string, int>("", 0);
		}

		return std::pair<std::string, int>(name.value(), value.as_int());
	}

	std::pair<std::string, Float> Scene::ParseXmlFloat(const pugi::xml_node& node)
	{
		pugi::xml_attribute name = node.first_attribute();
		pugi::xml_attribute value = node.last_attribute();
		if (name.empty() || value.empty() ||
			std::string(name.name()) != "name" || std::string(value.name()) != "value")
		{
			std::cout << "float node is invalid!" << std::endl;
			return std::pair<std::string, float>("", 0);
		}

		return std::pair<std::string, float>(name.value(), value.as_float());
	}

	std::pair<std::string, Transform> Scene::ParseXmlTransform(const pugi::xml_node& node)
	{
		pugi::xml_attribute name = node.first_attribute();
		if (std::string(name.name()) == "name")
		{
			std::vector<Transform> matrixs;
			for (const pugi::xml_node& ch : node.children())
			{
				if (std::string(ch.name()) == "lookat")
				{
					pugi::xml_attribute attr = ch.attribute("target");

					if (attr.empty())
					{
						std::cout << "lookat node miss \"target\" attribute!" << std::endl;
						return std::pair<std::string, Transform>(name.name(), Transform());
					}
					std::string str(attr.as_string());
					std::vector<std::string> tokens = Split(str, ",");
					if (tokens.size() < 3)
					{
						std::cout << "lookat node \"target\" attribute value error!" << std::endl;
						return std::pair<std::string, Transform>(name.name(), Transform());
					}
					
					Point3f lookat({ std::stof(tokens[0]) , std::stof(tokens[1]) , std::stof(tokens[2]) });

					attr = ch.attribute("origin");
					if (attr.empty())
					{
						std::cout << "lookat node miss \"origin\" attribute!" << std::endl;
						return std::pair<std::string, Transform>(name.name(), Transform());
					}
					str = attr.as_string();
					tokens = Split(str, ",");
					if (tokens.size() < 3)
					{
						std::cout << "lookat node \"origin\" attribute value error!" << std::endl;
						return std::pair<std::string, Transform>(name.name(), Transform());
					}
					Point3f origin({ std::stof(tokens[0]) , std::stof(tokens[1]) , std::stof(tokens[2]) });

					attr = ch.attribute("up");
					if (attr.empty())
					{
						std::cout << "lookat node miss \"up\" attribute!" << std::endl;
						return std::pair<std::string, Transform>(name.name(), Transform());
					}
					str = attr.as_string();
					tokens = Split(str, ",");
					if (tokens.size() < 3)
					{
						std::cout << "lookat node \"up\" attribute value error!" << std::endl;
						return std::pair<std::string, Transform>(name.name(), Transform());
					}
					Vector3f up({ std::stof(tokens[0]) , std::stof(tokens[1]) , std::stof(tokens[2]) });

					Transform mat = LookAt(origin, lookat, up);
					
					return std::pair<std::string, Transform>(name.name(), mat);
				}
				else if (std::string(ch.name()) == "rotate")
				{
					pugi::xml_attribute attr = ch.attribute("x");
					float x = attr.as_float();
					attr = ch.attribute("y");
					float y = attr.as_float();
					attr = ch.attribute("z");
					float z = attr.as_float();
					Vector3f vec({x, y, z});

					attr = ch.attribute("angle");
					float angle = attr.as_float();
					
					matrixs.push_back(Rotate(angle, vec));
				}
				else if (std::string(ch.name()) == "translate")
				{
					pugi::xml_attribute attr = ch.attribute("x");
					float x = attr.as_float();
					attr = ch.attribute("y");
					float y = attr.as_float();
					attr = ch.attribute("z");
					float z = attr.as_float();
					Vector3f vec({ x, y, z });

					matrixs.push_back(Translate(vec));
				}
				else if (std::string(ch.name()) == "scale")
				{
					pugi::xml_attribute attr = ch.attribute("x");
					float x = attr.as_float();
					attr = ch.attribute("y");
					float y = attr.as_float();
					attr = ch.attribute("z");
					float z = attr.as_float();
					
					matrixs.push_back(Scale(x, y, z));
				}
				else if (std::string(ch.name()) == "matrix")
				{
					pugi::xml_attribute attr = ch.attribute("value");
					if (!attr.empty())
					{
						std::string str = attr.as_string();
						std::vector<std::string> values = Split(str);
						std::vector<Float> vals;
						for (int i = 0; i < 16; ++i)
						{
							vals.push_back(std::stof(values[i]));
						}
						matrixs.push_back(Matrix4x4(vals[0], vals[1], vals[2], vals[3],
							vals[4], vals[5], vals[6], vals[7],
							vals[8], vals[9], vals[10], vals[11],
							vals[12], vals[13], vals[14], vals[15]));
					}
					else
					{
						std::cout << "matrix has no \"value\"" << std::endl;
					}
				}
				else 
				{
					std::cout << "Transform child node invalid!" << std::endl;
				}
			}

			Transform temp;
			for (int i = 0; i < matrixs.size(); ++i)
			{
				temp = temp * matrixs[i];
			}
			return std::pair<std::string, Transform>(name.value(), temp);
		}
		else
		{
			std::cout << "Transform name attribute is invalid!" << std::endl;
			return std::pair<std::string, Transform>("default", Transform());
		}
	}

	std::vector<std::string> Scene::Split(const std::string& s, const std::string& delimiters)
	{
		std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
		std::string::size_type pos = s.find_first_of(delimiters, lastPos);

		std::vector<std::string> tokens;
		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			tokens.push_back(s.substr(lastPos, pos - lastPos));
			lastPos = s.find_first_not_of(delimiters, pos);
			pos = s.find_first_of (delimiters, lastPos);
		}
		return tokens;
	}

	void Scene::CheckScene()
	{
		// 默认积分器
		//if (mIntegrator == nullptr)
		//{
		//	mIntegrator = CreateDirectLightingIntegrator(LightStrategy::UniformSampleOne, 5, 
		//		std::shared_ptr<Sampler>(mSampler), mMainCamera);
		//}

	}

	void Scene::OutputNode(const pugi::xml_node& node)
	{
		std::cout << "node:  " << node.name() << std::endl;
		for (pugi::xml_attribute_iterator iter = node.attributes_begin(); iter != node.attributes_end(); ++iter)
		{
			std::cout << "attr name = " << iter->name() << ", value = " << iter->value() << std::endl << std::endl;
		}
	}
}
