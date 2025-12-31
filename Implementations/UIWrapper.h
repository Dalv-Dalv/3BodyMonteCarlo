#pragma once
#include <GLFW/glfw3.h>
#include <vector>

#include "Body.h"
#include "Palette.h"


class UIWrapper {
private:
	static GLFWwindow* window;
	UIWrapper();
	static Body initialBody[3];
	static bool showUI;
	static bool key_pressed;

	static int sl_timeStep;
	static float sl_trailWeight;
	static float sl_diffusionRate;
	static float sl_decayRate;

	static bool enableFpsCap;

	static std::vector<Palette> colorPresets;

public:
	static bool restart;
	static bool hideTrail;

	static int selectedPaletteIndex;
	static float customCol1[3];
	static float customCol2[3];
	static float customCol3[3];

	static void Initialize(GLFWwindow* window);
	static void applyPreset(int preset);
	static void Render(int screenWidth, int screenHeight);

	static Palette GetPalette();

	static Body* GetBody() { return initialBody; }
	static int Get_TimeStep() { return sl_timeStep; }
	static float Get_TrailWeight() { return sl_trailWeight; }
	static float Get_DiffusionRate() { return sl_diffusionRate; }
	static float Get_DecayRate() { return sl_decayRate; }
};
