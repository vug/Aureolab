#type vertex
#version 460 core

uniform mat4 u_MVP;
uniform vec3 u_camPos;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

out vec3 v_Color;
out float v_DistToCamera;

void main(void) {
	vec4 pos = u_MVP * vec4(a_Position, 1.0);
	v_DistToCamera = pos.z;
	gl_PointSize = 1.0 / (1.0 + v_DistToCamera) * 64.0f;
	gl_Position = pos;
	v_Color = a_Color;
}



#type geometry
#version 460 core

layout (points) in;
layout (points, max_vertices = 6) out;

in vec3 v_Color[];
in float v_DistToCamera[];

out vec3 g_Color;

void main(void)
{
	int k;
	float outOfFocus = pow(abs(v_DistToCamera[0] - 4.0) * 0.25, 2.0) * 0.5;
	for (k = 0; k < 6; k++) {
		gl_Position = gl_in[0].gl_Position + vec4(cos(2. * 3.14159265 * k / 6) * outOfFocus, sin(2. * 3.14159265 * k / 6) * outOfFocus, 0.0, 0.0);
		gl_PointSize = gl_in[0].gl_PointSize;
		g_Color = v_Color[0];
		EmitVertex();
	}
    EndPrimitive();
}



#type fragment
#version 460 core

in vec3 g_Color;

void main(void) {
	const vec4 color1 = vec4(g_Color, 0.1);
	const vec4 color2 = vec4(g_Color, 0.0);

	vec2 temp = gl_PointCoord - vec2(0.5);
	float r2 = dot(temp, temp);

	if (r2 > 0.25) discard;
	gl_FragColor = mix(color1, color2, smoothstep(0.1, 0.25, r2));
}