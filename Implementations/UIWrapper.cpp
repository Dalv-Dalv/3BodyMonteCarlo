#include "UIWrapper.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>

#include "Body.h"
GLFWwindow* UIWrapper::window = nullptr;
bool UIWrapper::showUI = false;
bool UIWrapper::restart = true;
bool UIWrapper::key_pressed = false;
int UIWrapper::sl_timeStep = 1;
float UIWrapper::sl_trailWeight = 0.5;
float UIWrapper::sl_diffusionRate = 1.0;
float UIWrapper::sl_decayRate = 0.01;
float UIWrapper::sl_moveSpeed = 5.0;
float UIWrapper::sl_turnSpeed = 5.0;
float UIWrapper::sl_sensorAngleSpacing = 10.0;
float UIWrapper::sl_sensorDistOffset = 45;
Body UIWrapper::initialBody[3]={
		{ 0.2f, 0.0f, 0.0f, 0.161f, 1.0f, 1.0f, 0, 0},
		{-0.2f, 0.0f, 0.0f, -0.161f, 1.0f, 0, 1.0f, 0},
		{ 0.0f, 0.346f, -0.161f, 0.0f, 1.0f, 0, 0, 1.0f}
	};
void UIWrapper::Initialize(GLFWwindow* window) {
	UIWrapper::window = window;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}
void UIWrapper::applyPreset(int preset) {
	switch (preset) {

	case 0:
		initialBody[0]={ 0.2f, 0.0f, 0.0f, 0.161f, 1.0f, 1.0f, 0, 0};
		initialBody[1]={ -0.2f, 0.0f, 0.0f, -0.161f, 1.0f, 0, 1.0f, 0};
		initialBody[2]={ 0.0f, 0.346f, -0.161f, 0.0f, 1.0f, 0, 0, 1.0f};
		break;
	case 1:
		{
		float mult=1.4;
		initialBody[0] = {  0.0f,      1.1547f,  -0.5f*mult,     0.0f*mult,     1.0f, 1.0f, 0, 0};
		initialBody[1] = { -1.0f,      -0.5773f,  0.25f*mult,    -0.433f*mult,  1.0f, 0, 1.0f, 0};
		initialBody[2] = {  1.0f,      -0.5773f,  0.25f*mult,     0.433f*mult,  1.0f, 0, 0, 1.0f};
		break;
	}
	case 2:
		initialBody[0]={ -0.97000436,  0.24308753, 0.466203685,  0.43236573, 1.0f, 1.0f, 0, 0};
		initialBody[1]={  0.97000436, -0.24308753,   0.466203685,  0.43236573, 1.0f, 0, 1.0f, 0};
		initialBody[2]={ 0.0,0.0, -0.93240737,  -0.86473146, 1,0, 0, 1.0f};
		break;
	case 3:

		initialBody[0] = { -1,0, 0.2374365149,0.2536896353, 1, 1.0f, 0.0f, 0.0f };
		initialBody[1] = { 1,0, 0.2374365149,0.2536896353, 1, 0.0f, 1.0f, 0.0f };
		initialBody[2] = { 0,0, -0.9497460596,-1.0147585412, 0.5, 0.0f, 0.0f, 1.0f };
		break;
	default:;
	}
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
	ImGui::SliderFloat("Trail diffusion rate", &sl_diffusionRate, 0.0f, 20.0f);
	ImGui::SliderFloat("Trail decay rate", &sl_decayRate, 0.001f, 1.0f);
	for(int i=0;i<3;i++) {

		ImGui::Text(("Body "+std::to_string(i)).c_str());
		float* posPtr = reinterpret_cast<float*>(
			reinterpret_cast<char*>(&initialBody[i]) + offsetof(Body, x)
		);
		float* speedPtr = reinterpret_cast<float*>(
			reinterpret_cast<char*>(&initialBody[i]) + offsetof(Body, vx)
		);
		ImGui::SliderFloat(("Mass##"+std::to_string(i)).c_str(),&initialBody[i].mass , 0.0f, 5.0f);
		ImGui::DragFloat2(("Speed##"+ std::to_string(i)).c_str(), speedPtr, 0.0f, -1.0f, 1.0f);
		ImGui::DragFloat2(("Pos##" + std::to_string(i)).c_str(),posPtr,0,-1,1);
	}
	if(ImGui::Button("Start Sim")) {
		restart=true;
	};
	const char* presets[] = {
		"Something",
		"Lagrange Equilateral triangle",
		"Infinity",
		"Random stable-ish"
	};
	static int currentPreset = 0;

	if (ImGui::Combo("Preset", &currentPreset, presets, IM_ARRAYSIZE(presets))) {
		applyPreset(currentPreset);
	}
	ImGui::End();

	ImGui::Render();
	glViewport(0, 0, screenWidth, screenHeight);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
