#version 430

uniform sampler2D visualizationTexture;
uniform float time;
uniform int width, height;

out vec4 FragColor;


void main() {
    vec2 unit = vec2(1) / vec2(width, height);
    vec2 uv = gl_FragCoord.xy * unit;

    vec4 texColor = texture(visualizationTexture, uv);

    FragColor = texColor;
}
