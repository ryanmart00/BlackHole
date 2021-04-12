#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aDir;

out vec3 Direction;

uniform mat3 View;

void main()
{
    Direction = View * aDir;
    gl_Position = vec4(aPos, 1.0);
}
