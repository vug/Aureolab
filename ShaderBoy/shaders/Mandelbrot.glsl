#type vertex
#version 460 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_UV;

uniform vec3 iResolution;

out vec2 uv;
out vec2 fragCoord;

void main() {
    gl_Position = vec4(a_Position, 0.0, 1.0);
    uv = a_UV;
	fragCoord = a_UV * iResolution.xy;
}


#type fragment
#version 460 core

uniform float iTime;
uniform float iTimeDelta;
uniform vec3 iResolution;
uniform vec4 iMouse;

in vec2 uv;
in vec2 fragCoord;

/////



// z = a*I+b => mag2 = |a|^2 + |b|^2
float mag2(mat2 m) {
	return abs(m[0][0])*abs(m[0][0]) + abs(m[1][0])*abs(m[1][0]);
}

const int maxIter = 50;

void main()
{
	vec2 c = fragCoord.xy / iResolution.y;
	vec2 ratio = iResolution.xy / iResolution.y;
	vec2 m = iMouse.xy / iResolution.xy;
	//c = (c*2.0 - ratio)*m.y + vec2(m.x,0.0); // for mouse control, vertical shift and zoom
	c = (c*2.0 - ratio) + vec2(-0.5,0.0); // coordinates = complex plane
	mat2 mc = mat2(c.x, c.y, -c.y, c.x); // matrix representation of a complex number
	
	mat2 mz = mat2(0.0, 0.0, 0.0, 0.0); // inital number
	int nIter = 0; // number of iterations before breaking the loop
	int iterBreak = 0; // was the loop broken?
	for(int i=0; i < maxIter; i++) {
		mz = mz*mz + mc; // iterate the sequence according to the mandelbrot set rule. z_{n+1} = z_n^2 + c
		if(mag2(mz) > 4.0 && iterBreak == 0) { // if the magnitude of z_n > 2 then end iteration
			nIter = i; // at which step is the iteration broken?
			iterBreak = 1;
		}
	}

	if(iterBreak==0) nIter = maxIter;
	//float col = float(nIter)/float(maxIter); // constant coloring
	//float col = mod(float(nIter)+50.0*iTime, 50.0)/float(maxIter); // pulses out
	float col = (sin( 3.1415*(float(nIter)+iTime*5.0)/float(maxIter) )+1.0)*0.5;

	gl_FragColor = vec4(col, col, col, 1.0);
}