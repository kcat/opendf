#version 130

uniform sampler2D TexImage;

in vec4 Color;
in vec4 TexCoord0;

out vec4 ColorOutput;

void main()
{
    ColorOutput = texture2D(TexImage, TexCoord0.xy) * Color;
}
