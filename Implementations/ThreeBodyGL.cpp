#include "ThreeBodyGL.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../Utils/Random.h"
#include "../include/stb_image_write.h"
#include "Palette.h"
#include "UIWrapper.h"


struct Simulation {
	Body bodies[3];  // 3 * 32 bytes = 96 bytes
	int status;      // 4 bytes (1 = stabil, 0 = coliziune/expulzat)
	float padding[3];// 12 bytes padding
};




void AnalyzeStatistics(const std::vector <Simulation> &data) {
	SimStats s = UIWrapper::stats;
	s.alive = 0; s.collisions = 0; s.ejections = 0;
	float G = 1;
	double currentTotalEnergy = 0.0;
	for(auto sim : data) {
		if(sim.status == 1) {
			s.alive ++;
			// Calcul Energie Cinetica (K = 1/2 * m * v^2)
			// Calcul Energie Potentiala (V = -G * m1 * m2 / r)
			double K = 0, V = 0;
			for(int i=0; i<3; i++) {
				float vx = sim.bodies[i].vx;
				float vy = sim.bodies[i].vy;
				K += 0.5 * sim.bodies[i].mass * (vx*vx + vy*vy);

				for(int j=i+1; j<3; j++) {
					float dx = sim.bodies[i].x - sim.bodies[j].x;
					float dy = sim.bodies[i].y - sim.bodies[j].y;
					float dist = std::sqrt(dx*dx + dy*dy) + 0.01f;
					V -= (G * sim.bodies[i].mass * sim.bodies[j].mass) / dist;
				}
			}
			currentTotalEnergy += (K + V);
		}
		else if(sim.status == 2) s.collisions ++;
		else if(sim.status == 3) s.ejections ++;
	}
	s.survivalProb = s.alive * 1. / data.size();
	float alpha = UIWrapper::sl_alpha;
    s.hoeffdingError = std::sqrt(std::log(2.0f / alpha) / (2.0f * data.size()));

	s.survivalHistory.push_back(s.survivalProb);
	if(s.survivalHistory.size() > 100) { // patram ultimele 100 sec
		s.survivalHistory.erase(s.survivalHistory.begin());
	}
	if (s.alive > 0) {
		s.totalEnergyMean = s.totalEnergyMean = (float)(currentTotalEnergy / (double)s.alive);;
		if (UIWrapper::restart || s.initialEnergy == 0)
			s.initialEnergy = s.totalEnergyMean;
		s.energyDrift = std::abs(s.totalEnergyMean - s.initialEnergy);
	}

	UIWrapper::UpdateStats(s);
}





ThreeBodyGL::ThreeBodyGL(int screenWidth, int screenHeight, bool fullScreen)
	: screenWidth(screenWidth), screenHeight(screenHeight) {
	glfwInit();

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL Test", fullScreen ? primaryMonitor : NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return;
	}

	std::cout << "OpenGL Initialized successfully" << std::endl;

	UIWrapper::Initialize(window);

	std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << std::endl;
}
ThreeBodyGL::~ThreeBodyGL() {
	glfwDestroyWindow(window);
	glfwTerminate();
}
void ThreeBodyGL::LoadData() {
	simCount = UIWrapper::ui_SIM_COUNT;
	UIWrapper::stats = {};
	std::vector<Simulation> simulations(simCount);

	// First simulation in perfectly stable condition
	simulations[0].status = 1;
	simulations[0].bodies[0] = UIWrapper::GetBody()[0];
	simulations[0].bodies[1] = UIWrapper::GetBody()[1];
	simulations[0].bodies[2] = UIWrapper::GetBody()[2];

	for(int i = 1; i < simCount; i++) {
		simulations[i].status = 1;
		simulations[i].bodies[0] = UIWrapper::GetBody()[0];
		simulations[i].bodies[1] = UIWrapper::GetBody()[1];
		simulations[i].bodies[2] = UIWrapper::GetBody()[2];

		// Monte Carlo
		float noiseScale = 0.001f;

		for(int b = 0; b < 3; b++) {
			float x, y;
			Random::RandomPointInUnitCircle(x, y);

			simulations[i].bodies[b].x += x * noiseScale;
			simulations[i].bodies[b].y += y * noiseScale;
		}
	}


	glBindBuffer(GL_SHADER_STORAGE_BUFFER, simBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Simulation) * simulations.size(), simulations.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, simBuffer); // Binding 0 to match GLSL
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Reset texture
	glGenTextures(1, &trailTexture);
	glBindTexture(GL_TEXTURE_2D, trailTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void ThreeBodyGL::Animate() {
	static_assert(sizeof(Simulation) % 16 == 0);

	LoadData();

	Palette crntPalette = UIWrapper::GetPalette();
	Palette targetPalette = crntPalette;


	glGenTextures(1, &trailTexture);
	glBindTexture(GL_TEXTURE_2D, trailTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &bodiesTexture);
	glBindTexture(GL_TEXTURE_2D, bodiesTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glGenBuffers(1, &simBuffer);


	GLuint computeProgram = CreateComputeProgram("Shaders/threeBody.comp");
	glUseProgram(computeProgram);
	glUniform1i(glGetUniformLocation(computeProgram, "width"), screenWidth);
	glUniform1i(glGetUniformLocation(computeProgram, "height"), screenHeight);
	glUniform1i(glGetUniformLocation(computeProgram, "simCount"), simCount);
	glUniform1f(glGetUniformLocation(computeProgram, "G"), 1.0f); // Constanta G
	glUniform1f(glGetUniformLocation(computeProgram, "escapeThreshold"), 5.0f); // Distanța max
	glUniform1f(glGetUniformLocation(computeProgram, "collisionThreshold"), 0.01f); // Distanța max
	glUniform1f(glGetUniformLocation(computeProgram, "deltaTime"), 0.0005f);
	int compTimeLoc = glGetUniformLocation(computeProgram, "time");

	GLuint evaporationComputeProgram = CreateComputeProgram("Shaders/trailEvaporation.comp");
	glUseProgram(evaporationComputeProgram);
	glUniform1i(glGetUniformLocation(evaporationComputeProgram, "width"), screenWidth);
	glUniform1i(glGetUniformLocation(evaporationComputeProgram, "height"), screenHeight);
	int evapCompDeltaTimeLoc = glGetUniformLocation(evaporationComputeProgram, "deltaTime");
	int diffuseRateLoc = glGetUniformLocation(evaporationComputeProgram, "diffuseRate");
	int decayRateLoc = glGetUniformLocation(evaporationComputeProgram, "decayRate");

	auto fullscreenQuad = GetFullscreenQuad();
	GLuint fragmentShaderProgram = CreateShaderProgram("Shaders/defaultVertex.vert", "Shaders/threeBody.frag");
	int fragTextureLoc = glGetUniformLocation(fragmentShaderProgram, "trailTexture");
	int fragBodiesTextureLoc = glGetUniformLocation(fragmentShaderProgram, "bodiesTexture");
	int fragWidthLoc = glGetUniformLocation(fragmentShaderProgram, "width");
	int fragHeightLoc = glGetUniformLocation(fragmentShaderProgram, "height");
	int fragTimeLoc = glGetUniformLocation(fragmentShaderProgram, "time");
	int fragHideTrailLoc = glGetUniformLocation(fragmentShaderProgram, "hideTrail");
	int fragB1ColLoc = glGetUniformLocation(fragmentShaderProgram, "body1Col");
	int fragB2ColLoc = glGetUniformLocation(fragmentShaderProgram, "body2Col");
	int fragB3ColLoc = glGetUniformLocation(fragmentShaderProgram, "body3Col");

	glUseProgram(fragmentShaderProgram);
	glUniform1i(fragWidthLoc, screenWidth);
	glUniform1i(fragHeightLoc, screenHeight);

	float prevTime = 0;
	float deltaTime = 0;
	float hideTrailLerp = 0;
	float lastStatTime = 0;
	UIWrapper::restart=true;
	float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	std::cout << "Start Animating" << std::endl;
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glClearTexImage(bodiesTexture, 0, GL_RGBA, GL_FLOAT, clearColor);

		float currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;

		// Interpolate between palettes
		float colorLerpSpeed = 2.0f * deltaTime;
		targetPalette = UIWrapper::GetPalette();
		for(int i = 0; i < 3; i++) {
			crntPalette.c1[i] += (targetPalette.c1[i] - crntPalette.c1[i]) * colorLerpSpeed;
			crntPalette.c2[i] += (targetPalette.c2[i] - crntPalette.c2[i]) * colorLerpSpeed;
			crntPalette.c3[i] += (targetPalette.c3[i] - crntPalette.c3[i]) * colorLerpSpeed;
		}

		for(int i = 0; i < UIWrapper::Get_TimeStep(); i++) {
			glUseProgram(computeProgram);
			glUniform1f(compTimeLoc, currentTime);
			glUniform1i(glGetUniformLocation(computeProgram, "stepMethod"), UIWrapper::sl_stepMethod);



			glBindImageTexture(0, trailTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(1, bodiesTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, simBuffer);
			glDispatchCompute((simCount + 255) / 256, 1, 1);

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);

			glUseProgram(evaporationComputeProgram);
			glUniform1f(diffuseRateLoc, UIWrapper::Get_DiffusionRate());
			glUniform1f(decayRateLoc, UIWrapper::Get_DecayRate());
			glUniform1f(evapCompDeltaTimeLoc, deltaTime);
			glBindImageTexture(0, trailTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
			glDispatchCompute((screenWidth + 15) / 16, (screenHeight + 15) / 16, 1);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
		}

		glUseProgram(fragmentShaderProgram);
		glUniform1f(fragTimeLoc, currentTime);
		hideTrailLerp = hideTrailLerp * (1.0f - deltaTime * 2.0f) + (UIWrapper::hideTrail ? 1.0f : 0.0f) * deltaTime * 2.0f;
		glUniform1f(fragHideTrailLoc, hideTrailLerp);
		glUniform3fv(fragB1ColLoc, 1, crntPalette.c1);
		glUniform3fv(fragB2ColLoc, 1, crntPalette.c2);
		glUniform3fv(fragB3ColLoc, 1, crntPalette.c3);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, trailTexture);
		glUniform1i(fragTextureLoc, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bodiesTexture);
		glUniform1i(fragBodiesTextureLoc, 1);

		glBindVertexArray(fullscreenQuad);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		if(currentTime - lastStatTime >= 1.0f) {
			lastStatTime = currentTime;

			std::vector<Simulation> cpuSimData(simCount);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, simBuffer);

			GLint actualBufferSize = 0;
			glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &actualBufferSize);

			if(actualBufferSize >= (GLint)(sizeof(Simulation) * simCount)) {
				glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Simulation) * simCount, cpuSimData.data());
				AnalyzeStatistics(cpuSimData);
			}
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}


		UIWrapper::Render(screenWidth, screenHeight);

		glfwSwapBuffers(window);
		glfwPollEvents();
		if(UIWrapper::restart) {
			LoadData();
			UIWrapper::restart = false;
		}
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

GLuint ThreeBodyGL::LoadShader(GLenum type, const char* path) {
	std::ifstream file(path);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string shaderCode = buffer.str();
	const char* source = shaderCode.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "Shader Compilation Failed:\n" << infoLog << std::endl;
		return 0;
	}

	return shader;
}

GLuint ThreeBodyGL::CreateComputeProgram(const char* shaderPath) {
	GLuint computeShader = LoadShader(GL_COMPUTE_SHADER, shaderPath);
	GLuint program = glCreateProgram();
	glAttachShader(program, computeShader);
	glLinkProgram(program);

	// Check for linking errors
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "Compute Program Linking Failed:\n" << infoLog << std::endl;

		throw 404;
	}

	glDeleteShader(computeShader);
	return program;
}

GLuint ThreeBodyGL::CreateShaderProgram(const char* vertexPath, const char* fragmentPath) {
	GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertexPath);
	GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragmentPath);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
		throw 404;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}


void ThreeBodyGL::SaveTexture(GLuint textureID, int width, int height, const char* filename) {
	// Bind texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Allocate buffer to store pixels
	std::vector<int> pixels(width * height);

	// Read texture data (assuming GL_R32I integer format)
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_INT, pixels.data());

	// Convert integer texture to 8-bit grayscale for saving
	std::vector<unsigned char> image(width * height);
	for(size_t i = 0; i < pixels.size(); i++) {
		image[i] = static_cast<unsigned char>(pixels[i] % 256); // Normalize
	}

	// Save as PNG (1 channel, grayscale)
	if(stbi_write_png(filename, width, height, 1, image.data(), width)) {
		std::cout << "Texture saved to " << filename << std::endl;
	} else {
		std::cerr << "Failed to save texture!" << std::endl;
	}
}


void ThreeBodyGL::FramebufferSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

GLuint ThreeBodyGL::GetFullscreenQuad() {
	GLfloat quadVertices[] = {
		// positions        // texCoords
		-1.0f,  1.0f,      0.0f, 1.0f,
		-1.0f, -1.0f,      0.0f, 0.0f,
		 1.0f, -1.0f,      1.0f, 0.0f,
		-1.0f,  1.0f,      0.0f, 1.0f,
		 1.0f, -1.0f,      1.0f, 0.0f,
		 1.0f,  1.0f,      1.0f, 1.0f
	};

	// Create VAO and VBO for the quad
	GLuint quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	// Set up the quad's vertex data
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// Set up vertex attributes for position and texture coordinates
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Unbind VAO and VBO (optional)
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return quadVAO;
}
