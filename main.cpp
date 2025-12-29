#include "Implementations/ThreeBodyGL.h"

int main() {
	auto glslCompute = ThreeBodyGL(1920, 1080, true);
	glslCompute.Animate(1920, 1080);
	return 0;
}
