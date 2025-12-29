#include "UIWrapper.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

GLFWwindow* UIWrapper::window = nullptr;
bool UIWrapper::showUI = false;
bool UIWrapper::key_pressed = false;
int UIWrapper::sl_timeStep = 1;
float UIWrapper::sl_trailWeight = 0.5;
float UIWrapper::sl_diffusionRate = 10.0;
float UIWrapper::sl_decayRate = 0.06;
float UIWrapper::sl_moveSpeed = 5.0;
float UIWrapper::sl_turnSpeed = 5.0;
float UIWrapper::sl_sensorAngleSpacing = 10.0;
float UIWrapper::sl_sensorDistOffset = 45;

void UIWrapper::Initialize(GLFWwindow* window) {
	UIWrapper::window = window;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void UIWrapper::Render(int screenWidth, int screenHeight) {
	if(window == nullptr) return;
	if(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
		if(!key_pressed) {
			showUI = !showUI;
			key_pressed = true;
		}
	}else {
		key_pressed = false;
	}

	if(!showUI) return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Slime mold simulation settings");

	ImGui::SliderInt("Time step", &sl_timeStep, 1, 20);
	ImGui::SliderFloat("Trail weight", &sl_trailWeight, 0.0f, 10.0f);
	ImGui::SliderFloat("Diffusion rate", &sl_diffusionRate, 0.0f, 20.0f);
	ImGui::SliderFloat("Decay rate", &sl_decayRate, 0.001f, 3.0f);
	ImGui::SliderFloat("Move speed", &sl_moveSpeed, 0.0f, 100.0f);
	ImGui::SliderFloat("Turn speed", &sl_turnSpeed, -20.0f, 20.0f);
	ImGui::SliderFloat("Sensor spacing", &sl_sensorAngleSpacing, 0.0f, 179.9f);
	ImGui::SliderFloat("Sensor distance", &sl_sensorDistOffset, 0.0f, 150.0f);

	ImGui::End();

	ImGui::Render();
	glViewport(0, 0, screenWidth, screenHeight);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
