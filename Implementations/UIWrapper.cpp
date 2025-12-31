#include "UIWrapper.h"

#include <cmath>
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
float UIWrapper::sl_decayRate = 1.0;
float UIWrapper::sl_moveSpeed = 5.0;
float UIWrapper::sl_turnSpeed = 5.0;
float UIWrapper::sl_sensorAngleSpacing = 10.0;
float UIWrapper::sl_sensorDistOffset = 45;

float UIWrapper::sl_alpha = 0.05f;
float UIWrapper::sl_epsilon = 0.05f;
int UIWrapper::ui_SIM_COUNT = 748;
int UIWrapper::calculatedN = 748;

Body UIWrapper::initialBody[3] = {{0.2f, 0.0f, 0.0f, 0.161f, 1.0f, 1.0f, 0, 0}, {-0.2f, 0.0f, 0.0f, -0.161f, 1.0f, 0, 1.0f, 0}, {0.0f, 0.346f, -0.161f, 0.0f, 1.0f, 0, 0, 1.0f}};

SimStats UIWrapper::stats;


void HelpMarker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void UIWrapper::CalculateRequiredN() {
	// N = ln(2/alpha) / (2 * epsilon^2)
	calculatedN = std::ceil(std::log(2.0f / sl_alpha) / (2.0f * sl_epsilon * sl_epsilon));
}

void UIWrapper::UpdateStats(const SimStats& newStats) {
	stats = newStats;
}
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

		initialBody[0] = { -1,0, 0.2374365149,0.2536896353, 1, 1.0f, 0.8f, 0.0f };
		initialBody[1] = { 1,0, 0.2374365149,0.2536896353, 1, 0.0f, 0.8f, 1.0f };
		initialBody[2] = { 0,0, -0.9497460596,-1.0147585412, 0.5, 1.0f, 0.0f, 0.8f };
		break;
	default:;
	}
}

void UIWrapper::Render(int screenWidth, int screenHeight) {
	if(window == nullptr)
		return;
	if(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
		if(!key_pressed) {
			showUI = !showUI;
			key_pressed = true;
		}
	} else {
		key_pressed = false;
	}

	if(!showUI)
		return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Slime mold simulation settings");
	// ImGui::DragFloat(("Error Exponent" ),colorPtr);

	ImGui::SliderInt("Time step", &sl_timeStep, 1, 20);
	ImGui::SliderFloat("Trail diffusion rate", &sl_diffusionRate, 0.0f, 20.0f);
	ImGui::SliderFloat("Trail decay rate", &sl_decayRate, 0.001f, 1.0f);
	for(int i = 0; i < 3; i++) {

		ImGui::Text(("Ball " + std::to_string(i)).c_str());
		float* colorPtr = reinterpret_cast<float*>(reinterpret_cast<char*>(&initialBody[i]) + offsetof(Body, r));
		float* posPtr = reinterpret_cast<float*>(reinterpret_cast<char*>(&initialBody[i]) + offsetof(Body, x));
		float* speedPtr = reinterpret_cast<float*>(reinterpret_cast<char*>(&initialBody[i]) + offsetof(Body, vx));
		ImGui::SliderFloat(("Mass##" + std::to_string(i)).c_str(), &initialBody[i].mass, 0.0f, 5.0f);
		ImGui::DragFloat2(("Speed##" + std::to_string(i)).c_str(), speedPtr, 0.0f, -1.0f, 1.0f);
		ImGui::DragFloat2(("Pos##" + std::to_string(i)).c_str(), posPtr, 0, -1, 1);
		ImGui::ColorEdit3(("Color##" + std::to_string(i)).c_str(), colorPtr);
	}
	if(ImGui::Button("Start Sim")) {
		ui_SIM_COUNT = calculatedN;
		restart = true;
	};
	const char* presets[] = {"Something", "Lagrange Equilateral triangle", "8 figure", "Random stable-ish"};
	static int currentPreset = 0;

	if(ImGui::Combo("Preset", &currentPreset, presets, IM_ARRAYSIZE(presets))) {
		applyPreset(currentPreset);
	}
	ImGui::End();

	MonteCarloStatistics();
	MonteCarloDashboard();

	ImGui::Render();
	glViewport(0, 0, screenWidth, screenHeight);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void UIWrapper::MonteCarloStatistics() {
	ImGui::Begin("Monte Carlo Statistics");

    // Calculam nivelul de incredere bazat pe alpha setat in Dashboard
	float trustLevel = (1.0f - sl_alpha) * 100.0f;

	ImGui::Text("Total Simulation Capacity: %d", ui_SIM_COUNT);
	ImGui::Separator();

	// Afișare status numeric din structura stats (completata de ThreeBodyGL)
	ImGui::TextColored(ImVec4(0, 1, 0, 1), "Active (Alive): %d", stats.alive);
	ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Collisions: %d", stats.collisions);
	ImGui::TextColored(ImVec4(1, 0, 0, 1), "Ejections: %d", stats.ejections);

	ImGui::Separator();

	// Rata de supravietuire și Eroarea Hoeffding
	ImGui::Text("Survival Probability: %.2f%%", stats.survivalProb * 100.0f);
	ImGui::Text("Hoeffding Margin (%.1f%% Trust): +/- %.4f", trustLevel, stats.hoeffdingError);

    // Vizualizare tip Progress Bar pentru distributie
	if(ui_SIM_COUNT > 0) {
		float fracAlive = (float)stats.alive / ui_SIM_COUNT;
		float fracColl = (float)stats.collisions / ui_SIM_COUNT;
		float fracEjec = (float)stats.ejections / ui_SIM_COUNT;

		ImGui::Text("Live Distribution:");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 0.8f, 0, 1));
		ImGui::ProgressBar(fracAlive, ImVec2(-1, 0), "Alive");
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 0.5f, 0, 1));
		ImGui::ProgressBar(fracColl, ImVec2(-1, 0), "Collisions");
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 0, 0, 1));
		ImGui::ProgressBar(fracEjec, ImVec2(-1, 0), "Ejections");
		ImGui::PopStyleColor();
	}

	ImGui::End();
}

void UIWrapper::MonteCarloDashboard() {
	ImGui::Begin("Monte Carlo Analysis Dashboard");

	ImGui::Text("=== MONTE CARLO SETUP (Theoretic) ===");

    // Slidere pentru controlul parametrilor statistici
	if (ImGui::SliderFloat("Confidence Alpha (a)", &sl_alpha, 0.001f, 0.9f, "%.3f")) {
		CalculateRequiredN();
	}
	ImGui::SameLine(); HelpMarker("Alpha este probabilitatea de a gresi. 0.05 inseamna ca suntem 95% siguri de rezultat.");

	if (ImGui::SliderFloat("Error Margin (e)", &sl_epsilon, 0.005f, 0.2f, "%.3f")) {
		CalculateRequiredN();
	}
	ImGui::SameLine(); HelpMarker("Epsilon este eroarea maxima acceptata. 0.01 inseamna o precizie de +/- 1%.");

	ImGui::BulletText("Recommended N for these settings: %d", calculatedN);

	if (ImGui::Button("Apply Settings & Restart Simulation")) {
		ui_SIM_COUNT = calculatedN; // Setam noul numar de simulari
		restart = true;
	}

	ImGui::Separator();

	// Sectiunea 1: Rigoare Statistica (Calculata pe datele reale)
	ImGui::Text("=== CURRENT STATISTICAL RIGOR ===");
	ImGui::Text("Empirical Survival (p-hat): %.4f", stats.survivalProb);
	ImGui::Text("Hoeffding Error Margin: +/- %.4f", stats.hoeffdingError);

    // Calculam intervalul de incredere [p - e, p + e]
    float lowBound = (stats.survivalProb - stats.hoeffdingError) * 100.0f;
    float highBound = (stats.survivalProb + stats.hoeffdingError) * 100.0f;
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "95%% Confidence Interval:");
    ImGui::Text("[%.2f%% < P_real < %.2f%%]", lowBound < 0 ? 0 : lowBound, highBound > 100 ? 100 : highBound);

	ImGui::Spacing();
	ImGui::Separator();

	// Sectiunea 2: Grafic Evolutie
	ImGui::Text("=== SURVIVAL PROBABILITY HISTORY ===");
	if (!stats.survivalHistory.empty()) {
		ImGui::PlotLines("##Survival", stats.survivalHistory.data(),
						 (int)stats.survivalHistory.size(), 0, nullptr, 0.0f, 1.0f, ImVec2(-1, 80));
	}

	ImGui::Spacing();
	ImGui::Separator();

	// Sectiunea 3: Audit Fizic (Validarea metodei numerice)
	ImGui::Text("=== PHYSICAL PRECISION AUDIT ===");
	ImGui::Text("Initial Mean Energy (E0): %.4f", stats.initialEnergy);
	ImGui::Text("Current Mean Energy (E): %.4f", stats.totalEnergyMean);

	float driftPercent = (stats.initialEnergy != 0) ? (stats.energyDrift / std::abs(stats.initialEnergy)) : 0;
	if (driftPercent > 0.1f)
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Status: UNSTABLE (Drift %.1f%%)", driftPercent * 100.0f);
	else
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: NUMERICALLY STABLE (Drift %.2f%%)", driftPercent * 100.0f);

	ImGui::End();
}
