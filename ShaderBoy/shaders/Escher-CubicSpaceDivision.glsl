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

const int MAX_MARCHING_STEPS = 150;
const float MIN_DIST = 0.0;
const float MAX_DIST = 1000.0;
const float EPSILON = 0.01;
const float PI = 3.14159265;

/**
 * Rotation matrix around the X axis.
 */
mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}

/**
 * Rotation matrix around the Y axis.
 */
mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0),
        vec3(-s, 0, c)
    );
}

/**
 * Rotation matrix around the Z axis.
 */
mat3 rotateZ(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, -s, 0),
        vec3(s, c, 0),
        vec3(0, 0, 1)
    );
}


/**
 * Return a transform matrix that will transform a ray from view space
 * to world coordinates, given the eye point, the camera target, and an up vector.
 *
 * This assumes that the center of the camera is aligned with the negative z axis in
 * view space when calculating the ray marching direction. See rayDirection.
 */
mat3 viewMatrix(vec3 eye, vec3 center, vec3 up) {
    // Based on gluLookAt man page
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    return mat3(s, u, -f);
}


/**
 * Return the normalized direction to march in from the eye point for a single pixel.
 * 
 * fieldOfView: vertical field of view in degrees
 * size: resolution of the output image
 * fragCoord: the x,y coordinate of the pixel in the output image
 */
vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord) {
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fieldOfView) / 2.0);
    return normalize(vec3(xy, -z));
}





float noise3D(vec3 p)
{
	return fract(sin(dot(p ,vec3(12.9898,78.233,128.852))) * 43758.5453)*2.0-1.0;
}

float simplex3D(vec3 p)
{
	
	float f3 = 1.0/3.0;
	float s = (p.x+p.y+p.z)*f3;
	int i = int(floor(p.x+s));
	int j = int(floor(p.y+s));
	int k = int(floor(p.z+s));
	
	float g3 = 1.0/6.0;
	float t = float((i+j+k))*g3;
	float x0 = float(i)-t;
	float y0 = float(j)-t;
	float z0 = float(k)-t;
	x0 = p.x-x0;
	y0 = p.y-y0;
	z0 = p.z-z0;
	
	int i1,j1,k1;
	int i2,j2,k2;
	
	if(x0>=y0)
	{
		if(y0>=z0){ i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
		else if(x0>=z0){ i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
		else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; }  // Z X Z order
	}
	else 
	{ 
		if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
		else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
		else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
	}
	
	float x1 = x0 - float(i1) + g3; 
	float y1 = y0 - float(j1) + g3;
	float z1 = z0 - float(k1) + g3;
	float x2 = x0 - float(i2) + 2.0*g3; 
	float y2 = y0 - float(j2) + 2.0*g3;
	float z2 = z0 - float(k2) + 2.0*g3;
	float x3 = x0 - 1.0 + 3.0*g3; 
	float y3 = y0 - 1.0 + 3.0*g3;
	float z3 = z0 - 1.0 + 3.0*g3;	
				 
	vec3 ijk0 = vec3(i,j,k);
	vec3 ijk1 = vec3(i+i1,j+j1,k+k1);	
	vec3 ijk2 = vec3(i+i2,j+j2,k+k2);
	vec3 ijk3 = vec3(i+1,j+1,k+1);	
            
	vec3 gr0 = normalize(vec3(noise3D(ijk0),noise3D(ijk0*2.01),noise3D(ijk0*2.02)));
	vec3 gr1 = normalize(vec3(noise3D(ijk1),noise3D(ijk1*2.01),noise3D(ijk1*2.02)));
	vec3 gr2 = normalize(vec3(noise3D(ijk2),noise3D(ijk2*2.01),noise3D(ijk2*2.02)));
	vec3 gr3 = normalize(vec3(noise3D(ijk3),noise3D(ijk3*2.01),noise3D(ijk3*2.02)));
	
	float n0 = 0.0;
	float n1 = 0.0;
	float n2 = 0.0;
	float n3 = 0.0;

	float t0 = 0.5 - x0*x0 - y0*y0 - z0*z0;
	if(t0>=0.0)
	{
		t0*=t0;
		n0 = t0 * t0 * dot(gr0, vec3(x0, y0, z0));
	}
	float t1 = 0.5 - x1*x1 - y1*y1 - z1*z1;
	if(t1>=0.0)
	{
		t1*=t1;
		n1 = t1 * t1 * dot(gr1, vec3(x1, y1, z1));
	}
	float t2 = 0.5 - x2*x2 - y2*y2 - z2*z2;
	if(t2>=0.0)
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(gr2, vec3(x2, y2, z2));
	}
	float t3 = 0.5 - x3*x3 - y3*y3 - z3*z3;
	if(t3>=0.0)
	{
		t3 *= t3;
		n3 = t3 * t3 * dot(gr3, vec3(x3, y3, z3));
	}
	return 96.0*(n0+n1+n2+n3);
	
}



/*
 * Yet another reproduction of Escher's Cubic Space Division (1954) in GLSL. 
 * https://www.wikiart.org/en/m-c-escher/cubic-space-division
 *
 * Ray marching and lighting code taken from: https://www.shadertoy.com/view/4tcGDr
 * Simplex3D code taken from: https://www.shadertoy.com/view/XtBGDG
 * Edge Detection idea taken from: https://www.shadertoy.com/view/4s2XRd
 * Fog code taken from: http://www.iquilezles.org/www/articles/fog/fog.htm
 * Repetition by modulus code taken from: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
 * Anti-alias code taken from: https://www.shadertoy.com/view/Mss3R8 not giving a good result :-(
 */
#define AA 1

float boxSDF(vec3 p, vec3 size) {
    vec3 d = abs(p) - (size / 2.0);
    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0); 
    float outsideDistance = length(max(d, 0.0));
    
    return insideDistance + outsideDistance;
}

float columnSDF(vec3 p, vec3 size) {
    vec3 d = abs(vec3(p.x, 0.0, p.z)) - size * 0.5;
    
    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
    float outsideDistance = length(max(d, 0.0));
    
    return insideDistance + outsideDistance;    
}

float columnsSDF(vec3 p, float dbc) {
    float columnThickness = 1.0;
    
    vec3 c = vec3(dbc, 0.0, dbc);
    vec3 q = mod(p, c) - 0.5 * c;
    float columns = columnSDF(q, vec3(1.0) * columnThickness);
    return columns;    
}

float sceneSDF(vec3 p) {
    float distanceBetweenColumns = 20.0;
    
    vec3 p1 = p;    
    float columns1 = columnsSDF(p1, distanceBetweenColumns);
    
    vec3 p2 = rotateZ(PI * 0.5) * p;
    float columns2 = columnsSDF(p2, distanceBetweenColumns);
    
    vec3 p3 = rotateX(PI * 0.5) * p;
    float columns3 = columnsSDF(p3, distanceBetweenColumns);
    
    float columns = min(min(columns1, columns2), columns3);
    
    vec3 s = mod(p, vec3(distanceBetweenColumns)) - 0.5 * vec3(distanceBetweenColumns);
    float boxes = boxSDF(s, vec3(3.0));
    
    return min(columns, boxes);
}



vec2 rayMarch(vec3 eye, vec3 marchingDirection) {
    float depth = MIN_DIST;
    
    float EDGE_SIZE = 0.08;
    float SMOOTH = 0.05; 
    float minDist = 10000.0;
    bool stp = false;
    
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        float dist = sceneSDF(eye + depth * marchingDirection);
        
		if (!stp) {
            if (minDist < EDGE_SIZE && minDist < dist) {
                stp = true;
            }
            minDist = min(minDist, dist);
        }        
        
        
        if (dist < EPSILON) {
            float edge = 1.0;
            if (stp) {
                edge = smoothstep(EDGE_SIZE - SMOOTH, EDGE_SIZE + 0.01, minDist);
            }                
			return vec2(depth, edge);
        }
        
        depth += dist;
        if (depth >= MAX_DIST) {
            return vec2(MAX_DIST, 1.0);
        }
    }
    return vec2(MAX_DIST, 1.0);
}


vec3 estimateNormal(vec3 p) {
    return normalize(vec3(
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

vec3 diffuseContribForLight(vec3 k_d, vec3 p, vec3 eye, vec3 lightDir, vec3 lightIntensity) {
    vec3 N = estimateNormal(p);
    vec3 L = normalize(lightDir);
    vec3 V = normalize(eye - p);
    vec3 R = normalize(reflect(-L, N));
    
    float dotLN = dot(L, N);
    float dotRV = dot(R, V);
    
    if (dotLN < 0.0) {  // Light not visible from this point on the surface
        return vec3(0.0, 0.0, 0.0);
    } 
    
    return lightIntensity * (k_d * dotLN);    
}

vec3 illumination(vec3 k_a, vec3 k_d, vec3 p, vec3 eye) {
    const vec3 ambientLight = vec3(-0.15	);
    vec3 color = ambientLight * k_a;
    
    vec3 light1Pos = vec3(0.3, 0.5, -1.0);
    vec3 light1Intensity = vec3(1.0) * 5.0;
    color += diffuseContribForLight(k_d, p, eye, light1Pos, light1Intensity);

    return color;
}

vec3 applyFog( in vec3  rgb, in float distance )
{
   	float decayRate = 0.03;
    float fogAmount = 1.0 - exp(-distance * pow(decayRate, 1.4));
    vec3 fogColor = vec3(1.0);
    return mix( rgb, fogColor, fogAmount );
}

vec3 computeColor(vec3 eye, vec3 worldDir, bool isVR) {
    vec2 rm = rayMarch(eye, worldDir);    
    float dist = rm.x;
    float edge = rm.y;
    
    if (dist > MAX_DIST - EPSILON) {  // Didn't hit anything
		return vec3(0.95);
    }

    vec3 p = eye + dist * worldDir;
    
    vec3 K_a = vec3(1.0);
    vec3 K_d = vec3(0.14);
    float shininess = 5.0;
    vec3 color = illumination(K_a, K_d, p, eye);

    float noise = simplex3D(p * 12.0) * 0.5 + 0.5;
    noise = pow(noise, 3.0);
    color += noise * 0.4;

    color = applyFog(color, dist);
    
    if (!isVR) {
  		color += vec3(1.0 - edge);
    }
    
    color = pow(color, vec3(1.5)); // contrast
        
    return color;    
}

vec3 render(vec2 fragCoord) {
    float fov = 40.0;
    fov += 90.0 * iMouse.y / iResolution.y;
	vec3 viewDir = rayDirection(fov, iResolution.xy, fragCoord);    
    vec3 target = vec3(7.0, 8.0, 3.0);
    target += vec3(0.0, 0.0, -iTime * 4.0);
    vec3 eye = vec3(0.7, 0.83, 1.8);
    eye *= vec3(
        cos(iMouse.x / iResolution.x * PI),
        cos(iMouse.x / iResolution.x * PI * 2.0),
        1.0
    );
    eye = eye * 25.0 + target;
    
    mat3 viewToWorld = viewMatrix(eye, target, vec3(0.0, 1.0, 0.0));
    vec3 worldDir = viewToWorld * viewDir;   
    
	return computeColor(eye, worldDir, false);
}


void main()
{
    vec3 sColor = vec3(0.0);
	for( int j=0; j<AA; j++ )
	for( int i=0; i<AA; i++ )
	{
		vec2 of = -0.5 + vec2( float(i), float(j) )/float(AA);
	    sColor += render( fragCoord+of );
	}
	sColor /= float(AA*AA); 
    
    gl_FragColor = vec4(sColor, 1.0);
}

void mainVR(out vec4 fragColor, in vec2 fragCoord, in vec3 fragRayOri, in vec3 fragRayDir) {
    vec3 ro = fragRayOri + vec3(0.0, 0.0, iTime);
    vec3 color = computeColor(-10.0*ro, fragRayDir, true);
    gl_FragColor = vec4(color, 1.0);
}