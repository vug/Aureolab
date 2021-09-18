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

#define PI 3.14159265358979323846

// Presets
const int pattern = 0; // 0: stripes, 1: checkerboard, 2: texture
//

float hash(float x)
{
    return fract(sin(x) * 43758.5453) * 2.0 - 1.0;
}
vec2 hashPosition(float x)
{
	return vec2(hash(x), hash(x * 1.1))*2.0-1.0;
}

bool xor(bool a, bool b) {
	return (a && !b) || (!a && b);
}
float checkerBoardPattern(vec2 p) { // p in [0,1]x[0,1]
	bool x = p.x<0.5;
	bool y = p.y<0.5;
	return ( xor(x,y) ) ? 1.:0.;
}

float stripes(vec2 p) {
	float aa = 0.02;
	float xVal = + smoothstep(0.0-aa, 0.0+aa, p.x)
		         + (1.0 - smoothstep(0.5-aa, 0.5+aa, p.x))
		         + smoothstep(1.0-aa, 1.0+aa, p.x);
	return xVal-1.0;
}

void main()
{
	vec2 r = (fragCoord.xy - 0.5*iResolution.xy) / iResolution.y;
	
	// move the center around
	r += vec2(0.35,0.0)*sin(0.92*sin(2.1*iTime)+0.2);
	r += vec2(0.0,0.5)*cos(0.45*iTime+0.3);	

	// polar coordinates
	float mag = length(r);
	float angle = atan(r.y,r.x)/PI;

	float side = 1.0;
	float val = 0.0;
	if(pattern == 0) {
		vec2 tunnel = vec2(0.3/mag, angle);
		tunnel += vec2(2.5*iTime, 0.0);//forward speed and angular speed		
		val = stripes(mod(tunnel, side));
	} else if(pattern == 1) {
		vec2 tunnel = vec2(0.8/mag, 5.*angle+2.*mag);
		tunnel += vec2(2.5*iTime,0.2*iTime);		
		val = checkerBoardPattern(mod(tunnel, side));
	} 
	//else if (pattern == 2) {
	//	vec2 tunnel = vec2(0.3/mag, 2.*angle);
	//	tunnel += vec2(2.5*iTime,0.2*iTime);		
	//	val = texture(iChannel0, tunnel*1.0).x;
	//}
	// yellow and black colors
	vec3 color = mix(vec3(1.0,1.0,0.0), vec3(0.0,0.0,0.0), val);
	
	// the light ring that goes into the tunnel
	float signalDepth = pow(mod(1.5 - 0.9*iTime, 4.0),2.0);
	float s1 = signalDepth*0.9;
	float s2 = signalDepth*1.1;
	float dd = 0.05;
	color += 0.5*smoothstep(s1-dd,s1+dd, mag)*(1.0-smoothstep(s2-dd,s2+dd,mag))*vec3(1.0,1.0,0.5);
	
	color -= (1.0-smoothstep(0.0, 0.2, mag)); // shadow at the end of the tunnel
	color *= smoothstep( 1.8, 0.15, mag ); // vignette
	gl_FragColor = vec4(color, 1.0);
}