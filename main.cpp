#include "Implementations/ThreeBodyGL.h"
#include <Windows.h> // Required for __declspec

// Force NVIDIA high performance GPU
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

// Force AMD high performance GPU
extern "C" {
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


int main() {
	auto glslCompute = ThreeBodyGL(1920, 1080, true);
	glslCompute.Animate();
	return 0;
}
