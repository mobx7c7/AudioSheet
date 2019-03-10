#version 150

in vec4 position;
in vec2 texcoord;

uniform mat4 modelViewProjectionMatrix;

out vec2 texCoord;

void main()
{
    texCoord = texcoord;
    
    gl_Position = modelViewProjectionMatrix * position;
}