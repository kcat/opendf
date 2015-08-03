#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect ColorTex;
uniform sampler2DRect DiffuseTex;
uniform sampler2DRect SpecularTex;

in vec4 TexCoord0;

out vec4 ColorOutput;

void main()
{
    vec3 color = texture2DRect(ColorTex, TexCoord0.xy).rgb;
    vec3 diffuse = texture2DRect(DiffuseTex, TexCoord0.xy).rgb;
    vec3 specular = texture2DRect(SpecularTex, TexCoord0.xy).rgb;

    ColorOutput = vec4(color*diffuse + specular, 1.0);
}
