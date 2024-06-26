#version 410 core

in vec3 io_pos;

out vec4 o_color;

uniform vec3 u_camPos;
uniform vec4 u_color;

struct Fog
{
	bool enabled;
	vec3 color;
	float start, end;
};

uniform Fog u_fog;

void main()
{
	o_color = u_color;

	if (!u_fog.enabled) return;

	float camDist = distance(u_camPos, io_pos);

	if (camDist > u_fog.start)
	{
		float fogFactor = (camDist - u_fog.start) / (u_fog.end - u_fog.start);
		fogFactor = clamp(fogFactor, 0.0, 1.0);
		o_color = mix(o_color, vec4(u_fog.color, 1.0), fogFactor);
	}
}
