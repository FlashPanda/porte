#include <core/UtilFunctions.h>
#include <core/Scene.h>

namespace porte
{
	Scene* g_pScene = static_cast<Scene*>(new Scene());
	void porteParseFile(const std::string& filename)
	{
		g_pScene->LoadSceneFromFile(filename);
	}

	void porteRender()
	{
		g_pScene->Render();
	}
}