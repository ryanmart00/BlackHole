#version 330 core
precision mediump float;
out vec4 FragColor;

in vec3 Direction;

struct Light
{
    vec3 position;
    vec3 ambient; 
    vec3 diffuse;
    vec3 specular;
};

#define NUM_LIGHTS 2

uniform Light Lights[NUM_LIGHTS];

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Object
{
    Material mat;
    vec3 position;
    mat3 orient;
    vec3 dimensions;
};

#define NUM_OBJECTS 3

uniform Object objects[NUM_OBJECTS];

uniform vec3 Cam;
uniform float M;
uniform vec4 BlackHoleColor;
uniform vec4 BackgroundColor; 
uniform float MaxDist;
uniform float Threshold;
uniform float MaxStep;
uniform float EPSILON;
uniform float LOOP;

uniform mat3 rot;

#define M_PI 2 * 3.1415926535897932384626433832795
#define ONE_SIXTH 1.0/6.0
#define ONE_THIRD 1.0/3.0


float sphereSDF(vec3 pos, float r);
float prismSDF(vec3 p, vec3 d);
float cylinderSDF(vec3 p, vec2 d);
float torusSDF(vec3 p, vec2 d);
float coneSDF(vec3 p, vec2 d);
float blackholeSDF(vec3 pos);

float globalSDF(vec3 pos);
vec4 globalColor(vec3 pos, vec3 viewDir, mat3 system);

float unionSDF(float a, float b);
float intersectionSDF(float a, float b);
float differenceSDF(float a, float b);

float obj0SDF(vec3 pos)
{
    vec3 v = objects[0].orient * (pos - objects[0].position);
    float stat = sphereSDF(v, objects[0].dimensions.x);
    stat = differenceSDF(stat, torusSDF(v, vec2(objects[0].dimensions.x, 
        0.01*objects[0].dimensions.x)));
    vec3 w = transpose(rot) * objects[0].orient * (pos - objects[0].position + 
        objects[0].dimensions.x * rot * vec3(0,1,0));
    stat = differenceSDF(stat, coneSDF(w, objects[0].dimensions.x * vec2(0.5, 0.2)));
    return stat;
}

float obj1SDF(vec3 pos)
{
    vec3 v = objects[1].orient * (pos - objects[1].position);
    return torusSDF(v, objects[1].dimensions.xy);
}

float obj2SDF(vec3 pos)
{
    vec3 v = objects[2].orient * (pos - objects[2].position);
    vec3 b = v - vec3(-0.0,0,0.2);
    float cone = sphereSDF(b, 0.1);
    cone = unionSDF(cone, sphereSDF(v, 0.1));
    return cone;
}

vec4 phong(Material obj, vec3 pos, vec3 viewDir, mat3 system);
vec3 grad(vec3 pos);

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
    mat3 system = mat3(x,y,z);

    vec3 localDir = transpose(system) * Direction;
    vec3 data = 1/length(Cam) * vec3(0, 1, -localDir.x / localDir.z);
    float dist = 0;
    float range = 0;

    while(true)
    {
        if (1 > data.y * MaxDist)
        {
            FragColor = BackgroundColor;
            return;
        }
        if (LOOP * M_PI < abs(data.x))
        {
            //Gone around twice
            FragColor = BackgroundColor;
            return;
        }
        //step ray march
        z = system * (1/data.y * vec3(cos(data.x), 0, sin(data.x)));

        dist = globalSDF(z);
        if (dist < Threshold)
        {
            // compute color
            vec3 viewDir = normalize(system * 1/data.y * vec3(-sin(data.x), 0, cos(data.x)) 
                - 1/data.y/data.y * data.z * vec3(cos(data.x), 0, sin(data.x)));
            FragColor = globalColor(z, viewDir, transpose(system));

            return;
        }
        // otherwise step the DE by dist
        // Note that ds = sqrt(r^2 + (dr/dtheta)^2) dtheta
        // and du/dtheta = d/dtheta (1/r) = -1/r^2 dr/dtheta
        // so that ds = sqrt(1 + (du/dtheta)^2) r dtheta
        // or dtheta = u/sqrt(1 + (du)^2)
        range = data.y/sqrt(data.z*data.z + 1);
        data = stepDE(data, min(range, MaxStep));

    }
}

vec4 globalColor(vec3 pos, vec3 viewDir, mat3 system)
{
    if (blackholeSDF(pos) < Threshold)
    {
        return BlackHoleColor;
    }

    if(obj0SDF(pos) < Threshold)
    {
        return phong(objects[0].mat, pos, viewDir, system);
    }
    if(obj1SDF(pos) < Threshold)
    {
        return phong(objects[1].mat, pos, viewDir, system);
    }
    if(obj2SDF(pos) < Threshold)
    {
        return phong(objects[2].mat, pos, viewDir, system);
    }

    return BackgroundColor;
}

float globalSDF(vec3 pos)
{
    float r = blackholeSDF(pos);

    r = unionSDF(r, obj0SDF(pos));
    r = unionSDF(r, obj1SDF(pos));
    r = unionSDF(r, obj2SDF(pos));

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
    return max(a, -b);
}

float blackholeSDF(vec3 pos)
{
    return -2*M + length(pos);
}

float sphereSDF(vec3 pos, float r)
{
    return length(pos) - r;
}

float prismSDF(vec3 pos, vec3 dimensions)
{
    float x = max(pos.x - dimensions.x, -pos.x - dimensions.x); 
    float y = max(pos.y - dimensions.y, -pos.y - dimensions.y); 
    float z = max(pos.z - dimensions.z, -pos.z - dimensions.z); 
    return max(x,max(y,z));
}

float cylinderSDF(vec3 pos, vec2 dimensions)
{
    float y = max(pos.y - dimensions.y, -pos.y - dimensions.y); 
    return max(y, length(pos.xz) - dimensions.x);
}

float torusSDF(vec3 p, vec2 d)
{
    return length(vec2(length(p.xz) - d.x, p.y)) - d.y;
}

float coneSDF(vec3 p, vec2 d)
{
    vec2 v = vec2(length(p.xz), p.y);

    float x = d.x*(d.x*v.x+d.y*d.y-d.y*v.y)/dot(d,d);
    vec2 closestPoint = vec2(x, d.y - d.y/d.x*x);

    return max(-p.y, length(v) - length(closestPoint));
}


vec4 phong(Material obj, vec3 pos, vec3 viewDir, mat3 system)
{
    vec3 normal = grad(pos);

    vec3 result = vec3(0.0);

    for(int i = 0; i < NUM_LIGHTS; i++)
    {
        //ambient
        vec3 ambient = Lights[i].ambient * obj.ambient;

        //diffuse
        // TODO: Make this Non-euclidean
        vec3 lightDir = normalize(Lights[i].position - pos); 
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = Lights[i].diffuse * (diff * obj.diffuse);

        //specular
        vec3 halfwayDir = normalize(lightDir + Cam - pos);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), obj.shininess);
        vec3 specular = Lights[i].specular * (spec * obj.specular);

        result += ambient + diffuse + specular;
    }
    return vec4(result, 1.0);
}

vec3 grad(vec3 p)
{
    return normalize(vec3(
        globalSDF(vec3(p.x+EPSILON,p.y,p.z)) 
            - globalSDF(vec3(p.x-EPSILON,p.y,p.z)),
        globalSDF(vec3(p.x,p.y+EPSILON,p.z)) 
            - globalSDF(vec3(p.x,p.y-EPSILON,p.z)),
        globalSDF(vec3(p.x,p.y,p.z+EPSILON)) 
            - globalSDF(vec3(p.x,p.y,p.z-EPSILON))));
}

vec3 stepDE(vec3 data, float h)
{
    vec2 y = data.yz;
    vec2 k1 = f(y);        
    float halfH = 0.5 * h;
    vec2 k2 = f(halfH * k1 + y);
    vec2 k3 = f(halfH * k2 + y);
    vec2 k4 = f(h * k3 + y);
    y += h*ONE_SIXTH*(k1 + 2*k2 + 2*k3 + k4);
    return vec3(data.x + h, y);
}

// Returns <du, d^2u>
vec2 f(vec2 y)
{
    return vec2(y.y, 3 * M * y.x * y.x - y.x);
}
