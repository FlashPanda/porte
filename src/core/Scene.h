#pragma once
#include <memory>
#include <unordered_map>
#include <pugixml.hpp>
#include <pugiconfig.hpp>
#include <core/porte.h>
#include <filesystem>

namespace porte
{
	class Scene
	{
	public:

		std::unordered_map<std::string, std::shared_ptr<Material>> mMaterials;
		std::vector<std::shared_ptr<Primitive>> mPrimitives;
		std::vector<std::shared_ptr<Light>> mLights;
		std::shared_ptr<Primitive> mAggregate;
		std::vector<std::shared_ptr<Shape>>	mShapes;

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
		Film* ParseXmlFilm(const pugi::xml_node& node, std::unique_ptr<Filter> inFilter);
		Filter* ParseXmlFilter(const pugi::xml_node& node);
		void ParseXmlSampler(const pugi::xml_node& node);
		void ParseXmlCamera(const pugi::xml_node& node);
		void ParseXmlShape(const pugi::xml_node& node);
		void ParseXmlMaterial(const pugi::xml_node& node);
		void ParseXmlLight(const pugi::xml_node& node);
		void CheckScene();

		Vector3f ParseVectorString(const std::string& str);

		void OutputNode(const pugi::xml_node& node);

		std::pair<std::string, std::string> ParseXmlString(const pugi::xml_node& node);
		std::pair<std::string, int32_t> ParseXmlInt(const pugi::xml_node& node);
		std::pair<std::string, Float> ParseXmlFloat(const pugi::xml_node& node);
		std::pair<std::string, Transform> ParseXmlTransform(const pugi::xml_node& node);

		std::vector<std::string> Split(const std::string& s, const std::string& delimiters = " ");

	private:
		std::vector<std::filesystem::path> mSearchPaths;

		Integrator* mIntegrator = nullptr;

		int32_t mMeshCount = 1;
		int32_t mMaterialCount = 1;
		std::shared_ptr<Sampler> mSampler;
		std::shared_ptr<Camera> mCamera;
	};

	extern Scene* g_pScene;
}