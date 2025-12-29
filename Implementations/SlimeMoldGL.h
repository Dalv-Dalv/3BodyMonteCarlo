#pragma once
#include <glad.h>
#include <GLFW/glfw3.h>
#include <utility>

class SlimeMoldGL {
	GLFWwindow* window;
	int screenWidth, screenHeight;

	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

	static GLuint GetFullscreenQuad();
public:
	SlimeMoldGL(int screenWidth = 1024, int screenHeight = 800, bool fullScreen = false);
	~SlimeMoldGL();

	void Animate(int width, int height);

	static GLuint LoadShader(GLenum type, const char* path);
	static GLuint CreateComputeProgram(const char* shaderPath);
	static GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
	static void SaveTexture(GLuint textureID, int width, int height, const char* filename);
};