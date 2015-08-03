#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect TexImage;

in vec4 TexCoord0;

out vec4 ColorOutput;

void main()
{
    ColorOutput = texture2DRect(TexImage, TexCoord0.xy);
}
