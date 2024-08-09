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
uniform vec3 u_color;

void main()
{
    vec2 uv = (gl_FragCoord.xy / u_resolution) * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    if (u_objType == 0) {
        // Calculate the distance from the current fragment to the circle center
        float distance = 1.0 - length(uv - u_center);
        float fade = 0.005;
        distance = 1 - smoothstep(1 - u_radius, 1 - u_radius - fade, distance);
        FragColor.rgb = u_color;
        FragColor.a = distance;
    } else if (u_objType == 1) {
        FragColor = vec4(u_color, 1.0);
    }
}
