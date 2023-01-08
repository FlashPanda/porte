#pragma once
#include <memory>
#include <unordered_map>
#include <pugixml.hpp>
#include <pugiconfig.hpp>
#include "MathUtil.h"
#include "Vector.hpp"
#include "Matrix.hpp"
#include <filesystem>
#include "Filter.h"
#include "Material.h"
#include "Film.h"

namespace panda
{
	class SceneObject;
	class SceneNode;
	class SceneObjectMesh;
	class SceneObjectCamera;
	class SceneObjectMaterial;
	class SceneObjectLight;
	class SceneNodeMesh;
	class SceneNodeCamera;
	class SceneNodeLight;
	class Image;
	class Integrator;
	class Sampler;
	class SurfaceInteraction;
	class Ray;
	class Primitive;
	class Light;

	void CreatePPMImage(std::shared_ptr<Image> pImage);

	void CreatePPMImage(const Vector2Di& res, float* rgb);

	class Scene
	{
		// 不需要add函数因为这本身就是一个public的结构
	public:
		std::shared_ptr<SceneNode> SceneGraph;

		std::unordered_map<std::string, std::shared_ptr<SceneObjectMesh>> mMeshes;
		std::unordered_map<std::string, std::shared_ptr<SceneObjectCamera>> mCameras;
		std::unordered_map<std::string, std::shared_ptr<SceneObjectMaterial>> mMaterials;
		std::unordered_map<std::string, std::shared_ptr<SceneObjectLight>> mLights;

		std::unordered_map<std::string, std::shared_ptr<SceneNodeMesh>> mMeshNodes;
		std::unordered_map<std::string, std::shared_ptr<SceneNodeCamera>> mCameraNodes;
		std::unordered_map<std::string, std::shared_ptr<SceneNodeLight>> mLightNodes;

		std::shared_ptr<SceneNodeCamera> mMainCamera;

		std::unordered_map<std::string, std::shared_ptr<Material>> mBSDFMaterials;
		std::vector<std::shared_ptr<Primitive>> mPrimitives;
		std::vector<std::shared_ptr<Light>> mPbrtLights;
		std::shared_ptr<Primitive> mAggregate;

	public:
		// 从文件中加载场景
		void LoadSceneFromFile(std::string filename);

		void Render();

		// 是否相交，返回交点信息
		bool Intersect(const Ray& ray, SurfaceInteraction* insec) const ;

		// 是否相交
		bool IntersectP(const Ray& ray) const;

		/* xml场景解析 */
	public: 
		void ParseXmlScene(const std::string filename);

		void ParseXmlPath(const pugi::xml_node& node, const std::string& infile);
		void ParseXmlIntegrator(const pugi::xml_node& node);
		Film* ParseXmlFilm(const pugi::xml_node& node, Filter* inFilter);
		Filter* ParseXmlFilter(const pugi::xml_node& node);
		void ParseXmlSampler(const pugi::xml_node& node);
		void ParseXmlCamera(const pugi::xml_node& node);
		void ParseXmlShape(const pugi::xml_node& node);
		void ParseXmlMaterial(const pugi::xml_node& node);
		void ParseXmlLight(const pugi::xml_node& node);
		void CheckScene();

		Vector3Df ParseVectorString(const std::string& str);

		void OutputNode(const pugi::xml_node& node);

		std::pair<std::string, std::string> ParseXmlString(const pugi::xml_node& node);
		std::pair<std::string, int32> ParseXmlInt(const pugi::xml_node& node);
		std::pair<std::string, float> ParseXmlFloat(const pugi::xml_node& node);
		std::pair<std::string, Matrix4f> ParseXmlTransform(const pugi::xml_node& node);

		std::vector<std::string> Split(const std::string& s, const std::string& delimiters = " ");

#pragma region 测试

	public:
		void CreateTestScene1();
		void CreateTestScene2();
		void CreateTestScene3();
		void CreateTestScene4();

		
#pragma endregion

	private:
		std::vector<std::filesystem::path> mSearchPaths;

		Integrator* mIntegrator = nullptr;
		Sampler* mSampler = nullptr;

		int32 mMeshCount = 1;
		int32 mMaterialCount = 1;
	};

	extern Scene* g_pScene;
}