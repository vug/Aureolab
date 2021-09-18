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

in vec2 uv;
in vec2 fragCoord;

const int nBalls = 4;

float hash(float x)
{
	// Return a "random" number based on the "seed"
    return fract(sin(x) * 43758.5453) * 2.0 - 1.0;
}

vec2 hashPosition(float x)
{
	// Return a "random" position based on the "seed"
	return vec2(hash(x), hash(x * 1.1))*2.0-1.0;
}

float metaball(vec2 r, float hashSeed) {
	vec2 balls[nBalls]; // Ball coordinates
	r *= 1.0;
	float s[nBalls]; // Ball coordinates
	for(int i=0; i<nBalls; i++) {
		float ii = float(i)+hashSeed;
		balls[i] = hashPosition(ii)*0.1 // random position
			+0.8*vec2(hash(ii+0.1)*cos(iTime*hash(ii+0.2)),
				      hash(ii+0.3)*sin(iTime*hash(ii+0.4))); // random rotation around the position
		s[i] = 0.02*(hash(ii+0.5)+1.0)*0.5;
	}
	float mSurf = 0.0; // metaball surface value
	for(int i=0; i<nBalls; i++) {
		mSurf += s[i]/pow(length(r-balls[i]),2.0);
	}
	float cutoff = 0.8;
	mSurf = smoothstep(cutoff-0.02, cutoff+0.02, mSurf);
	//mSurf = clamp(mSurf, 0.7, 5.7)-0.7;
	return mSurf;
}

void main() {
	vec2 r = (fragCoord.xy - 0.5*iResolution.xy) / iResolution.y; // pixel coordinate

	gl_FragColor = (1.0-pow(abs(r.x),8.0))*vec4( 1.0, 1.0, 1.0, 1.0); // set a white background with darkened sides

	r += vec2(0.0, iTime*0.2);
	r = vec2(r.x, mod(r.y+1.0,2.0)-1.0);
	vec3 color = + metaball(r*1.5, 12.3)*vec3(1.0,0.0,0.0)
  				 + metaball(r*1.0, 124.3)*vec3(0.0,1.0,0.0)
	  			 + metaball(r*0.67, 56.2)*vec3(0.0,0.0,1.0);
	gl_FragColor -= 0.4*vec4(color, 1.0);
}
