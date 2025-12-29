#include "Implementations/SlimeMoldGL.h"

int main() {
	auto* glslCompute = new SlimeMoldGL(1920, 1080, true);
	glslCompute->Animate(1920, 1080);
	delete glslCompute;
	return 0;
}
