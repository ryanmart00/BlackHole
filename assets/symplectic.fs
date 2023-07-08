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
uniform Light SpotLight;

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
uniform vec3 CamDir;
uniform float RS;
uniform vec4 BlackHoleColor;
uniform vec4 BackgroundColor; 
uniform float MaxDist;
uniform float Threshold;
uniform float MaxStep;
uniform float EPSILON;
uniform float LOOP;

uniform mat3 rot;
uniform mat3 A;
uniform mat3 K;

#define M_PI 2 * 3.1415926535897932384626433832795
#define ONE_SIXTH 1.0/6.0
#define ONE_THIRD 1.0/3.0


float sphereSDF(vec3 pos, float r);
float prismSDF(vec3 p, vec3 d);
//radius, height
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
    vec3 dim = objects[2].dimensions;
    float h = dim.z;
    float t = dim.y;
    float w = dim.x;
    vec3 v = objects[2].orient * (pos - objects[2].position);
    // y is towards, x is right, z is up
    v = v.zxy;
    vec3 u = v - vec3(w-h,0,0);
    v = v + vec3(-w,0,-w*.75); 
    // B
    float z = prismSDF(v, vec3(h*ONE_SIXTH,t*0.5,h*.5));
    z = unionSDF(z, cylinderSDF(v +  h*vec3(ONE_SIXTH, 0, -.25), vec2(.25 * h,.5*t)));
    z = unionSDF(z, cylinderSDF(v +  h*vec3(ONE_SIXTH, 0, ONE_SIXTH), vec2(ONE_THIRD * h,.5*t)));
    // L
    v = v + vec3(h, 0 ,0);
    z = unionSDF(z, prismSDF(v, vec3(h*ONE_SIXTH,t*0.5,h*.5)));
    z = unionSDF(z, prismSDF(v + h*vec3(ONE_THIRD, 0, ONE_THIRD), 
        vec3(h*ONE_SIXTH,t*0.5,h*ONE_SIXTH)));
    // A
    v = v + vec3(h,0,0);
    z = unionSDF(z, prismSDF(A*v, vec3(h*ONE_SIXTH,t*0.5,h*.5)));
    z = unionSDF(z, prismSDF(transpose(A)*(v + vec3(ONE_THIRD*h, 0, 0)), 
        vec3(h*ONE_SIXTH,t*0.5,h*.5))); 
    // C
    v = v + vec3(h,0,0);
    z = unionSDF(z, cylinderSDF(v+vec3(ONE_THIRD*h,0,0), vec2(h*.5, .5*t)));
    z = differenceSDF(z, cylinderSDF(v+vec3(ONE_THIRD*h,0,0), vec2(h*ONE_THIRD, t)));
    z = differenceSDF(z, prismSDF(v+vec3(.75*h,0,0), vec3(.125*h, t, h)));
    // K
    v = v + vec3(h,0,0);
    z = unionSDF(z, prismSDF(v, vec3(h*ONE_SIXTH,t*0.5,h*.5)));
    z = unionSDF(z, prismSDF(A*(v+h*vec3(.25, 0, -.2)), vec3(h*ONE_SIXTH,t*0.5,h*ONE_THIRD)));
    z = unionSDF(z, prismSDF(K*(v+2*h*vec3(.2, 0, .12)), vec3(h*ONE_SIXTH,t*0.5,h*ONE_THIRD)));
    // H
    z = unionSDF(z, prismSDF(u, vec3(h*0.1,t*0.5,h*.5)));
    z = unionSDF(z, prismSDF(u+vec3(ONE_SIXTH*h,0,0), vec3(h*0.1,t*0.5,0.2 * h)));
    z = unionSDF(z, prismSDF(u+vec3(ONE_THIRD*h,0,0), vec3(h*0.1,t*0.5,h*.5)));

    // O
    u = u + vec3(h,0,0);
    z = unionSDF(z, sphereSDF(u+h*ONE_SIXTH*vec3(1,0,0), .5*h));
    // L
    u = u + vec3(h, 0 ,0);
    z = unionSDF(z, prismSDF(u, vec3(h*ONE_SIXTH,t*0.5,h*.5)));
    z = unionSDF(z, prismSDF(u + h*vec3(ONE_THIRD, 0, ONE_THIRD), 
        vec3(h*ONE_SIXTH,t*0.5,h*ONE_SIXTH)));
    // E
    u = u + vec3(h, 0 ,0);
    z = unionSDF(z, prismSDF(u, vec3(h*ONE_SIXTH,t*0.5,h*.5)));
    z = unionSDF(z, prismSDF(u + h*vec3(ONE_THIRD, 0, .4), 
        vec3(h*ONE_SIXTH,t*0.5,h*0.1)));
    z = unionSDF(z, prismSDF(u + h*vec3(ONE_THIRD, 0, -.4), 
        vec3(h*ONE_SIXTH,t*0.5,h*0.1)));
    z = unionSDF(z, prismSDF(u + h*vec3(ONE_THIRD, 0, 0), 
        vec3(h*0.1,t*0.5,h*0.1)));

    return z;
}

vec4 phong(Material obj, vec3 pos, vec3 viewDir, mat3 system);
vec3 grad(vec3 pos);

float f(float u);

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
    //initialize polar position and angle: note u = 1/r
    float phi = 0;
    float u = 1/length(Cam);
    float du = -localDir.x / localDir.z * u;
    float dphi = 0;
    float dist = 0;

    while(true)
    {
        if (1 > u * MaxDist)
        {
            FragColor = BackgroundColor;
            return;
        }
        if (LOOP * M_PI < abs(phi))
        {
            //Gone around twice
            FragColor = BackgroundColor;
            return;
        }
        //step ray march
        z = system * (1/u * vec3(cos(phi), 0, sin(phi)));

        dist = globalSDF(z);
        if (dist < Threshold)
        {
            // compute color
            // This is the vector at which the light came into the object:
            // if parametrized by r(phi) = (1/u cos phi, 0, 1/u sin phi)
            // then r' = 1/u (-sin phi, 0 , cos phi)  - 1/u^2 du/dphi (cos phi, 0, sin phi)
            vec3 viewDir = normalize(system * (1/u * vec3(-sin(phi), 0, cos(phi)) 
                - 1/u/u * du * vec3(cos(phi), 0, sin(phi))));
            FragColor = globalColor(z, viewDir, transpose(system));

            return;
        }
        // otherwise step the DE by dist
        // Note that ds = sqrt(r^2 + (dr/dphi)^2) dphi
        // and du/dphi = d/dphi (1/r) = -1/r^2 dr/dphi
        // so that ds = sqrt(1 + (r du/dphi)^2) r dphi
        // or dphi = ds * u/sqrt(1 + (du/dphi)^2)
        dphi = dist * u/sqrt(1 + du*du/u/u) * MaxStep;
        phi = phi + dphi;
        du = du + 0.5 * dphi * f(u);
        u = u + du * dphi;
        du = du + 0.5 * dphi * f(u);
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
    return -RS + length(pos);
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
        vec3 halfwayDir = normalize(lightDir - viewDir); //Cam - pos
        float spec = pow(max(dot(normal, halfwayDir), 0.0), obj.shininess);
        vec3 specular = Lights[i].specular * (spec * obj.specular);

        result += ambient + diffuse + specular;
    }
    {
        // Compute flashlight
        float theta = dot(normalize(Direction), CamDir);
        float intensity = clamp((theta - 0.96)/(0.02), 0.0, 1.0);

        //diffuse
        float diff = max(dot(normal, -viewDir), 0.0);
        vec3 diffuse = SpotLight.diffuse * (diff * obj.diffuse);

        //specular
        float spec = pow(max(dot(normal, -viewDir), 0.0), obj.shininess);
        vec3 specular = SpotLight.specular * (spec * obj.specular);

        result += intensity*(diffuse + specular);

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

// Returns the acceleration at position u
// We have the conservation (du/dphi)^2 + (1 - RS*u)*u^2 = 1/b^2 (where b = |L|/E)
// Taking derivatives: d^2u/dphi^2 = 3/2*RS*u^2 - u
float f(float u)
{
    return u*(3 * RS/2 * u  - 1);
}
