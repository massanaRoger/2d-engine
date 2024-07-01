#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec2 u_center;
uniform float u_radius;

void main()
{
    // Normalize fragment coordinates
    vec2 uv = (gl_FragCoord.xy / u_resolution) * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    // Calculate the distance from the current fragment to the circle center
    float distance = 1.0 - length(uv - u_center);
    float fade = 0.005;

    distance = smoothstep(0.0, fade, distance);
    FragColor.rgb = vec3(1.0);
    FragColor.a = distance;

}