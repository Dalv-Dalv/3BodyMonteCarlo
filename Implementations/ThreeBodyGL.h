#pragma once
#include <glad.h>

#include <GLFW/glfw3.h>
#include <utility>


class ThreeBodyGL {
	GLFWwindow* window;
	int screenWidth, screenHeight;
	GLuint simBuffer;
	GLuint trailTexture, bodiesTexture;
	const int SIM_COUNT = 1000;

	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

	static GLuint GetFullscreenQuad();
public:

	ThreeBodyGL(int screenWidth = 1024, int screenHeight = 800, bool fullScreen = false);
	~ThreeBodyGL();

	void LoadData();
	void Animate();

	static GLuint LoadShader(GLenum type, const char* path);
	static GLuint CreateComputeProgram(const char* shaderPath);
	static GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
	static void SaveTexture(GLuint textureID, int width, int height, const char* filename);
};