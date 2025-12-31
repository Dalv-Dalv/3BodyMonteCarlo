#version 430

uniform sampler2D trailTexture;
uniform sampler2D bodiesTexture;
uniform float time;
uniform int width, height;
uniform float hideTrail;

out vec4 FragColor;


vec4 toneMap(vec4 color, float exposure) {
    color *= exposure;
    return color / (color + vec4(1.0));
}


uniform vec3 body1Col = vec3(1.0, 0, 0.0547);
uniform vec3 body2Col = vec3(0.0117, 0.3509, 0.9882);
uniform vec3 body3Col = vec3(1.0, 0.8078, 0.6);

float smoothstep(float t){
    return t*t*(3 - 2 * t);
}
vec4 colormap(vec4 col) {
    return vec4(col.r * body1Col + col.g * body2Col + col.b * body3Col, col.a);
}

void main() {
    vec2 unit = vec2(1) / vec2(width, height);
    vec2 uv = gl_FragCoord.xy * unit;

    vec4 bodyCol = texture(bodiesTexture, uv);
    vec4 trailCol = texture(trailTexture, uv);

    bodyCol = pow(bodyCol, vec4(mix(20.0, 1.0, hideTrail)));

    vec4 trailGlow = colormap(pow(trailCol, vec4(4.0)));

    bodyCol = colormap(bodyCol);
    trailCol = colormap(trailCol);

    bodyCol *= mix(50.0, 2.0, hideTrail);
    trailCol = trailCol * 1.5 + trailGlow * 40.0;

    vec4 col = mix(trailCol, bodyCol, bodyCol.a);
    col = toneMap(col, 1.2);
    col = pow(col, vec4(1.0 / 1.5));

    col = mix(col, bodyCol * max(0.2, bodyCol.a * bodyCol.a), hideTrail);

    FragColor = col;
}
