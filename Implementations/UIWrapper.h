#pragma once
#include <GLFW/glfw3.h>

#include "Body.h"
#include <vector>

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




public:
	static bool restart;

	static float sl_alpha;
	static float sl_epsilon;
	static int ui_SIM_COUNT;
	static int calculatedN;
	static void CalculateRequiredN();
	static void UpdateStats(const SimStats& newStats);
	static SimStats stats;

	static void Initialize(GLFWwindow* window);
	static void applyPreset(int preset);
	static void Render(int screenWidth, int screenHeight);

	static Body* GetBody() { return initialBody; }
	static int Get_TimeStep() { return sl_timeStep; }
	static float Get_TrailWeight() { return sl_trailWeight; }
	static float Get_DiffusionRate() { return sl_diffusionRate; }
	static float Get_DecayRate() { return sl_decayRate; }
	static float Get_MoveSpeed() { return sl_moveSpeed; }
	static float Get_TurnSpeed() { return sl_turnSpeed; }
	static float Get_SensorAngleSpacing() { return sl_sensorAngleSpacing; }
	static float Get_SensorDistOffset() { return sl_sensorDistOffset; }
	static void MonteCarloStatistics();
	static void MonteCarloDashboard();
};
