#type vertex
#version 460 core

uniform mat4 u_MVP;
uniform vec3 u_camPos;

layout (location = 0) in vec3 a_Position;

void main(void) {
	vec4 pos = u_MVP * vec4(a_Position, 1.0);
	gl_PointSize = 1.0 / (1.0 + length(a_Position - u_camPos)) * 128.0f;
	//gl_PointSize = 64.0f;
	gl_Position = pos;
}

#type fragment
#version 460 core

void main(void) {
	const vec4 color1 = vec4(0.6, 0.0, 0.0, 1.0);
	const vec4 color2 = vec4(0.9, 0.7, 1.0, 1.0);

	vec2 temp = gl_PointCoord - vec2(0.5);
	float r2 = dot(temp, temp);

	if (r2 > 0.25) discard;
	gl_FragColor = mix(color1, color2, smoothstep(0.1, 0.25, r2));
}