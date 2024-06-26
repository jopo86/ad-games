#version 410 core

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec4 i_color;

out vec3 io_pos;
out vec4 io_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
	gl_Position = u_projection * u_view * u_model * vec4(i_pos, 1.0);
	io_pos = vec3(u_model * vec4(i_pos, 1.0));
	io_color = i_color;
}
