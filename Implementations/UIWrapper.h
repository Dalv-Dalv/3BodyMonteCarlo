#pragma once
#include <GLFW/glfw3.h>
#include <vector>

#include "Body.h"
#include "Palette.h"

struct SimStats {
	int alive = 0;
	int collisions = 0;
	int ejections = 0;
	float survivalProb = 0.0f;
	float hoeffdingError = 0.0f;

	std::vector<float> survivalHistory;
	float totalEnergyMean = 0.0f;
	float initialEnergy = 0.0f;
	float energyDrift = 0.0f;
	SimStats() {
		alive = collisions = 0; ejections = 0;
		survivalProb = hoeffdingError = 0.0f;
		survivalHistory.clear();
		totalEnergyMean = initialEnergy = 0.0f;
		energyDrift = 0.0f;
	}
};

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
	static float sl_moveSpeed;
	static float sl_turnSpeed;
	static float sl_sensorAngleSpacing;
	static float sl_sensorDistOffset;

	static bool enableFpsCap;

	static std::vector<Palette> colorPresets;

public:
	static bool restart;
	static bool hideTrail;

	static float sl_alpha;
	static float sl_epsilon;
	static int ui_SIM_COUNT;
	static int calculatedN;
	static void CalculateRequiredN();
	static void UpdateStats(const SimStats& newStats);
	static SimStats stats;

	static int selectedPaletteIndex;
	static float customCol1[3];
	static float customCol2[3];
	static float customCol3[3];

	static int sl_stepMethod;

	static void Initialize(GLFWwindow* window);
	static void applyPreset(int preset);
	static void Render(int screenWidth, int screenHeight);

	static Palette GetPalette();

	static Body* GetBody() { return initialBody; }
	static int Get_TimeStep() { return sl_timeStep; }
	static float Get_TrailWeight() { return sl_trailWeight; }
	static float Get_DiffusionRate() { return sl_diffusionRate; }
	static float Get_DecayRate() { return sl_decayRate; }
	static void MonteCarloStatistics();
	static void MonteCarloDashboard();
};
