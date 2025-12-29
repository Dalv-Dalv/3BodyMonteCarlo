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
#include <random>


#include "../include/stb_image_write.h"
#include "UIWrapper.h"

std::mt19937 rng(std::random_device{}());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

void randomPointInUnitCircle(float& x, float& y) {
	float angle = 2.0f * M_PI * dist(rng);
	float radius = std::sqrt(dist(rng));

	x = radius * std::cos(angle);
	y = radius * std::sin(angle);
}


ThreeBodyGL::ThreeBodyGL(int screenWidth, int screenHeight, bool fullScreen)
	: screenWidth(screenWidth), screenHeight(screenHeight) {
	glfwInit();


	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

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




void ThreeBodyGL::Animate(int width, int height) {
	std::cout << "Start Animating" << std::endl;
	struct Agent {
		float posx, posy;
		float angle;
	};

	GLuint slimeTexture;
	glGenTextures(1, &slimeTexture);
	glBindTexture(GL_TEXTURE_2D, slimeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint agentBuffer;
	const int AGENT_COUNT = 100000;
	int sl_timeStep = UIWrapper::Get_TimeStep();
	std::vector<Agent> agents(AGENT_COUNT);
	for(int i = 0; i < AGENT_COUNT; i++) {
		float t = static_cast<float>(i) / static_cast<float>(AGENT_COUNT);
		randomPointInUnitCircle(agents[i].posx, agents[i].posy);
		agents[i].angle = std::atan2(-agents[i].posy, -agents[i].posx);
		agents[i].posx *= std::min(width, height) / 2 - 300;
		agents[i].posy *= std::min(width, height) / 2 - 300;
		agents[i].posx += width / 2;
		agents[i].posy += height / 2;
	}
	glGenBuffers(1, &agentBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, agentBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * agents.size(), agents.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, agentBuffer); // Binding 0 to match GLSL
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint computeProgram = CreateComputeProgram("Shaders/slimeMold.comp");
	glUseProgram(computeProgram);
	glUniform1i(glGetUniformLocation(computeProgram, "width"), width);
	glUniform1i(glGetUniformLocation(computeProgram, "height"), height);
	glUniform1i(glGetUniformLocation(computeProgram, "agentsCount"), AGENT_COUNT);
	int compTimeLoc = glGetUniformLocation(computeProgram, "time");
	int compDeltaTimeLoc = glGetUniformLocation(computeProgram, "deltaTime");
	int sl_trailWeightLoc = glGetUniformLocation(computeProgram, "trailWeight");
	int sl_moveSpeedLoc = glGetUniformLocation(computeProgram, "moveSpeed");
	int sl_turnSpeedLoc = glGetUniformLocation(computeProgram, "turnSpeed");
	int sl_sensorAngleSpacingLoc = glGetUniformLocation(computeProgram, "sensorAngleSpacing");
	int sl_sensorDistOffsetLoc = glGetUniformLocation(computeProgram, "sensorOffsetDist");

	GLuint evaporationComputeProgram = CreateComputeProgram("Shaders/pheromoneEvaporation.comp");
	glUseProgram(evaporationComputeProgram);
	glUniform1i(glGetUniformLocation(evaporationComputeProgram, "width"), width);
	glUniform1i(glGetUniformLocation(evaporationComputeProgram, "height"), height);
	int evapCompDeltaTimeLoc = glGetUniformLocation(evaporationComputeProgram, "deltaTime");
	int sl_diffusionRateLoc = glGetUniformLocation(evaporationComputeProgram, "diffuseRate");
	int sl_decayRateLoc = glGetUniformLocation(evaporationComputeProgram, "decayRate");

	auto fullscreenQuad = GetFullscreenQuad();
	GLuint fragmentShaderProgram = CreateShaderProgram("Shaders/defaultVertex.vert", "Shaders/slimeMold.frag");
	int fragTextureLoc = glGetUniformLocation(fragmentShaderProgram, "slimeTexture");
	int fragWidthLoc = glGetUniformLocation(fragmentShaderProgram, "width");
	int fragHeightLoc = glGetUniformLocation(fragmentShaderProgram, "height");
	int fragTimeLoc = glGetUniformLocation(fragmentShaderProgram, "time");

	glUseProgram(fragmentShaderProgram);
	glUniform1i(fragWidthLoc, screenWidth);
	glUniform1i(fragHeightLoc, screenHeight);

	float prevTime = 0;
	float deltaTime = 0;
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		float currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;

		sl_timeStep = UIWrapper::Get_TimeStep();
		for(int i = 0; i < sl_timeStep; i++) {
			glUseProgram(computeProgram);
			glUniform1f(compTimeLoc, currentTime);
			glUniform1f(compDeltaTimeLoc, deltaTime);

			glUniform1f(sl_trailWeightLoc, UIWrapper::Get_TrailWeight());
			glUniform1f(sl_moveSpeedLoc, UIWrapper::Get_MoveSpeed());
			glUniform1f(sl_turnSpeedLoc, UIWrapper::Get_TurnSpeed());
			glUniform1f(sl_sensorAngleSpacingLoc, UIWrapper::Get_SensorAngleSpacing());
			glUniform1f(sl_sensorDistOffsetLoc, UIWrapper::Get_SensorDistOffset());

			glBindImageTexture(0, slimeTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, agentBuffer);
			glDispatchCompute((AGENT_COUNT + 15) / 16, 1, 1);


			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);


			glUseProgram(evaporationComputeProgram);
			glUniform1f(evapCompDeltaTimeLoc, deltaTime);

			glUniform1f(sl_diffusionRateLoc, UIWrapper::Get_DiffusionRate());
			glUniform1f(sl_decayRateLoc, UIWrapper::Get_DecayRate());

			glBindImageTexture(0, slimeTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
			glDispatchCompute((width + 15) / 16, (height + 15) / 16, 1);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
		}

		glUseProgram(fragmentShaderProgram);
		glUniform1f(fragTimeLoc, currentTime);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, slimeTexture);
		glUniform1i(fragTextureLoc, 0);

		glBindVertexArray(fullscreenQuad);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		UIWrapper::Render(screenWidth, screenHeight);

		glfwSwapBuffers(window);
		glfwPollEvents();
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
