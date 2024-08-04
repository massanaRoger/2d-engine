#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;

// Circle
uniform vec2 u_center;
uniform float u_radius;

// AABB
uniform vec2 u_min;
uniform vec2 u_max;

uniform int u_objType; // 0 - AABB, 1 - Circle, 2 - Polygon

void main()
{
    vec2 uv = (gl_FragCoord.xy / u_resolution) * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    if (u_objType == 0) {
        float inside = step(u_min.x, uv.x) * step(u_min.y, uv.y) * step(uv.x, u_max.x) * step(uv.y, u_max.y);
        vec3 color = mix(vec3(0.0), vec3(1.0), inside);
        FragColor.rgb = color;
        FragColor.a = inside;
    } else if (u_objType == 1) {
        // Normalize fragment coordinates
        vec2 normCenter = u_center;
        normCenter.x *= u_resolution.x / u_resolution.y;

        // Calculate the distance from the current fragment to the circle center
        float distance = 1.0 - length(uv - normCenter);
        float fade = 0.005;
        distance = 1 - smoothstep(1 - u_radius, 1 - u_radius - fade, distance);
        FragColor.rgb = vec3(1.0);
        FragColor.a = distance;
    } else if (u_objType == 2) {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
