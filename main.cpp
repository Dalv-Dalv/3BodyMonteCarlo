#include "Implementations/ThreeBodyGL.h"
#include <Windows.h> // Required for __declspec




int main() {
	auto glslCompute = ThreeBodyGL(1920, 1080, true);
	glslCompute.Animate(1920, 1080);
	return 0;
}
