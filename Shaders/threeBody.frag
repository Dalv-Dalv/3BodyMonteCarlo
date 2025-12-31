#version 430

uniform sampler2D visualizationTexture;
uniform float time;
uniform int width, height;

out vec4 FragColor;


void main() {
    vec2 unit = vec2(1) / vec2(width, height);
    vec2 uv = gl_FragCoord.xy * unit;

    vec4 texColor = texture(visualizationTexture, uv);

    vec3 body1Col = vec3(1.0, 0, 0.3647);
    vec3 body2Col = vec3(0.0117, 0.4509, 0.9882);
    vec3 body3Col = vec3(1.0, 0.8078, 0.6);

    vec3 col = texColor.r * body1Col + texColor.g * body2Col + texColor.b * body3Col;

    FragColor = vec4(col, 1.0);
}
