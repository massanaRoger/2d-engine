#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 transform;
uniform mat4 u_projection;

void main()
{
   gl_Position = u_projection * transform * vec4(aPos, 1.0);
}