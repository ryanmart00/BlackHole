#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

void main()
{             
    vec4 color = texture(image, TexCoords);
    FragColor = vec4(color.r, color.g, color.b, 1.0); 
}
