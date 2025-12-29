#pragma once
#include <GLFW/glfw3.h>

#include "Body.h"


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
};
