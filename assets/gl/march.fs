#version 330 core
precision highp float;
out vec4 FragColor;

in vec3 Direction;

struct Sphere
{
    vec4 color;
    vec3 position;
    float radius;
};

#define NUM_SPHERES 3

uniform Sphere Spheres[NUM_SPHERES];

uniform vec3 Cam;
uniform float M;
uniform vec4 BlackHoleColor;
uniform vec4 BackgroundColor;
uniform float MaxDist;
uniform float Threshold;
uniform float MaxStep;

#define MAX_ITER 600
#define M_PI 3.1415926535897932384626433832795

float sphereSDF(Sphere s, vec3 pos, mat3 system);
float blackholeSDF(vec3 pos);

float globalSDF(vec3 pos, mat3 system);
vec4 globalColor(vec3 pos, mat3 system);

float unionSDF(float a, float b);
float intersectionSDF(float a, float b);
float differenceSDF(float a, float b);

// returns <theta, u, du>
vec3 stepDE(vec3 data, float range); 
vec2 f(vec2 y);

void main()
{ 
    // First compute the coordinate system using the Laplacess 
    vec3 x = Cam;
    vec3 z = Direction;
    z = z - dot(x,z)/dot(x,x) * x;
    vec3 y = cross(z, x); //My own special twist
    x = normalize(x);
    y = normalize(y);
    z = normalize(z);

    // this is the transpose of the orthonormal basis constructed above 
    // and is thus the inverse transformation
    mat3 system = mat3(x.x, y.x, z.x, x.y, y.y, z.y, x.z, y.z, z.z);

    vec3 localDir = system * Direction;
    vec3 data = vec3(0, 1/length(Cam), -localDir.x /length(Cam) / localDir.z);
    float dist = 0;
    float range = 0;

    for(int i = 0; i < MAX_ITER; i++)
    {
        if (1 > data.y * MaxDist)
        {
            FragColor = BackgroundColor;
            return;
        }
        if (2* M_PI < abs(data.x))
        {
            FragColor = BackgroundColor;
            return;
        }
        //step ray march
        z = 1/data.y * vec3(cos(data.x), 0, sin(data.x));
        dist = globalSDF(z, system);
        if (dist < Threshold)
        {
            // compute color
            FragColor = globalColor(z, system);

            return;
        }
        // otherwise step the DE by dist
        // Note that ds = sqrt(r^2 + (dr/dtheta)^2) dtheta
        // and du/dtheta = d/dtheta (1/r) = -1/r^2 dr/dtheta
        // so that ds = sqrt(1 + (du/dtheta)^2) r dtheta
        // or dtheta = u/sqrt(1 + (du)^2)
        range = data.y/sqrt(1 + data.z*data.z);
        data = stepDE(data, min(range, MaxStep));
        
    }
    // we took too long
    FragColor = BackgroundColor;
}

vec4 globalColor(vec3 pos, mat3 system)
{
    int hits = 0;
    vec4 color = vec4(0);
    if (blackholeSDF(pos) <= 0)
    {
        color += BlackHoleColor;
        hits++;
    }
    for(int i = 0; i < NUM_SPHERES; i++)
    {
        if (sphereSDF(Spheres[i], pos, system) <= 0)
        {
            color += Spheres[i].color;
            hits++;
        }
    }
    return (1/hits) * color;
}

float globalSDF(vec3 pos, mat3 system)
{
    float r = blackholeSDF(pos);
    for(int i = 0; i < NUM_SPHERES; i++)
    {
        r= unionSDF(r, sphereSDF(Spheres[i], pos, system));
    }
    return r;
}

float unionSDF(float a, float b)
{
    return min(a,b);
}

float intersectionSDF(float a, float b)
{
    return max(a,b);
}

float differenceSDF(float a, float b)
{
    return min(a, -b);
}

float blackholeSDF(vec3 pos)
{
    return length(pos) - 2*M;
}

float sphereSDF(Sphere s, vec3 pos, mat3 system)
{
    return length(pos - system * s.position) - s.radius;
}

vec3 stepDE(vec3 data, float h)
{
    vec2 y = vec2(data.y, data.z);
    vec2 k1 = f(y);        
    vec2 k2 = f(y + (h/2) * k1);
    vec2 k3 = f(y + (h/2) * k2);
    vec2 k4 = f(y + h * k3);
    y = y + (h/6)*(k1 + 2*k2 + 2*k3 + k4);
    return vec3(data.x + h, y.x, y.y);
}

// Returns <du, d^2u>
vec2 f(vec2 y)
{
    return vec2(y.y, 3*M* y.x * y.x - y.x);
}
