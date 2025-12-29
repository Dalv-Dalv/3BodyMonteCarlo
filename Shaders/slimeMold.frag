#version 430

uniform sampler2D slimeTexture;
uniform float time;
uniform int width, height;

out vec4 FragColor;

vec3 hueShift( vec3 color, float hueAdjust ){

    const vec3  kRGBToYPrime = vec3 (0.299, 0.587, 0.114);
    const vec3  kRGBToI      = vec3 (0.596, -0.275, -0.321);
    const vec3  kRGBToQ      = vec3 (0.212, -0.523, 0.311);

    const vec3  kYIQToR     = vec3 (1.0, 0.956, 0.621);
    const vec3  kYIQToG     = vec3 (1.0, -0.272, -0.647);
    const vec3  kYIQToB     = vec3 (1.0, -1.107, 1.704);

    float   YPrime  = dot (color, kRGBToYPrime);
    float   I       = dot (color, kRGBToI);
    float   Q       = dot (color, kRGBToQ);
    float   hue     = atan (Q, I);
    float   chroma  = sqrt (I * I + Q * Q);

    hue += hueAdjust;

    Q = chroma * sin (hue);
    I = chroma * cos (hue);

    vec3    yIQ   = vec3 (YPrime, I, Q);

    return vec3( dot (yIQ, kYIQToR), dot (yIQ, kYIQToG), dot (yIQ, kYIQToB) );

}

vec3 pallete(float t){
    vec3 a = vec3(0.5);
    vec3 b = vec3(0.5);
    vec3 c = vec3(1.0);
    vec3 d = vec3(0.3, 0.2, 0.2);

    return a + b * cos(6.28318 * (c * t + d));
}

vec3 lerp(vec3 a, vec3 b, float t){
    return a * (1 - t) + b * t;
}

float smoothstep(float t){
    return t*t*(3 - 2 * t);
}

vec4 colorize(vec3 inCol){
    float val = inCol.r;
    float highlight = 1 - val;
    highlight = min(1, max(0, 0.2/highlight - 1.0));
    val = smoothstep(val);
    highlight = max(0, val - 0.6) / 0.8;


    vec3 col = lerp(vec3(23, 42, 58) / 500.0, vec3(80, 137, 145) / 255.0, val);
    col = lerp(col, vec3(255) / 255.0, highlight);

    col = hueShift(col, time);

    return vec4(col, 1);
}

void main() {
    vec2 unit = vec2(1) / vec2(width, height);
    vec2 uv = gl_FragCoord.xy * unit;

    FragColor = colorize(texture(slimeTexture, uv).rgb);
//    FragColor = colorize(uv.xxx);
}
