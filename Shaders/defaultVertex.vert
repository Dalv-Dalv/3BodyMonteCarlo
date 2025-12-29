#version 430

layout(location = 0) in vec2 position; // Incoming vertex position
layout(location = 1) in vec2 texCoord; // Incoming texture coordinates

out vec2 fragTexCoord; // Output texture coordinates to the fragment shader

void main() {
    fragTexCoord = texCoord;  // Pass the texture coordinates to the fragment shader
    gl_Position = vec4(position, 0.0, 1.0); // Set the position of the vertices
}
