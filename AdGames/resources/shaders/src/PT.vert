#version 410 core

layout (location = 0) in vec3 i_pos;
layout (location = 2) in vec2 i_texCoord;

out vec3 io_pos;
out vec2 io_texCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(i_pos, 1.0);
	io_texCoord = i_texCoord;
	io_pos = vec3(u_model * vec4(i_pos, 1.0));
}
