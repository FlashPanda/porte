//#include "porte.h"
#include <string>
#include <iostream>
#include <core/Scene.h>
#include <core/Parallel.h>

int main(int argc, char* argv[])
{
	google::InitGoogleLogging(argv[0]);
	FLAGS_stderrthreshold = 1; // Warning and above.

	if (argc < 2)
	{
		std::cout << "command parameters error, please set scene file!" << std::endl;
		return 0;
	}


	porte::Scene scene;		
	
	scene.LoadSceneFromFile(argv[1]);

	porte::ParallelInit();

	scene.Render();

	return 0;
}