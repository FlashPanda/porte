//#include "porte.h"
#include <string>
#include <iostream>
#include <core/Scene.h>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "command parameters error, please set scene file!" << std::endl;
		return 0;
	}


	porte::Scene scene;

	scene.LoadSceneFromFile(argv[1]);
	scene.Render();
	return 0;
}