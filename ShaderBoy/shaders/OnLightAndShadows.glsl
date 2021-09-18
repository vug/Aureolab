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
//out vec4 fragColor;

void main()
{
	const int nLights = 2;
	const int nObjects = 2;
	vec2 r = (fragCoord.xy - 0.5*iResolution.xy)  / iResolution.y; // coordinate of the pixel
	vec2 m = (iMouse.xy - 0.5*iResolution.xy)  / iResolution.y; // coordinate of mouse
	vec2 s[nLights]; // coordinates of light sources
	vec3 sColor[nLights]; // colors of lights
	vec2 c[nObjects]; // coordinates of disks
	float rad[nObjects]; // radii of disks
	
	s[0] = vec2(-0.5, 0.0)+vec2(0.2*cos(iTime),0.2*sin(iTime));
	sColor[0] = vec3(0.0, 1.0, 1.0); // rotating turquaz light
	s[1] = vec2(0.3, 0.2);
	sColor[1] = vec3(1.0, 1.0, 0.0)*(sin(iTime*1.2)+1.0)*0.6; // flashing yellow light
	
	c[0] = vec2(0.0, 0.0) + vec2(0.0, 0.25)*sin(iTime); // oscillating big disk
	rad[0] = 0.1;
	c[1] = m.xy; // disk moved by mouse
	rad[1] = 0.05;

	vec3 light = vec3(0.0);	
	float getLight;
	vec2 dsc, dsr, ndsc, ndsr;
	for(int i=0; i<nLights; i++) {
		getLight = 1.0;
		for(int j=0; j<nObjects; j++) {
			dsc = c[j] - s[i]; // distance vector from the light source [i] to object [j]
			dsr = r - s[i]; // distance vector from the light source [i] to the pixel
			ndsc = normalize(dsc); // normalized dsc
			ndsr = normalize(dsr); // normalized dsr
			float cosPixelObject = dot( ndsc, ndsr ); // cosine of the angle between the dsc and dsr
			// Add the light color if
			// 2) "from light to pixel" direction is not in the range of "light to disc center OR 
			// 1) distance between light and pixel is less than the distance light and object center
			if( cosPixelObject<cos(asin( rad[j]/length(dsc))) || length(dsr) < length(dsc)) {
				getLight *= 1.0;
			}
			else {
				getLight *= 0.0;
			}
		}
		if(getLight>0.0) {
			light += 0.2*sColor[i]/pow(length(dsr)+0.3, 1.2);
		}		
	}
	
	float objCol = 0.0;
	for(int j=0; j<nObjects; j++) { 
		objCol += 1.0-smoothstep(rad[j]-0.005,rad[j]+0.005, length(r.xy-c[j].xy)); // draw objects
		objCol += 1.0-step(0.01, length(r.xy-s[j].xy)); // and lights a white circles
	}	
	vec3 col = vec3(objCol) + vec3(light);
	//fragColor = vec4(col,1.0);
	gl_FragColor = vec4(col,1.0);
}