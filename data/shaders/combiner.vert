#version 130

uniform mat4 osg_ProjectionMatrix;

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;

out vec4 TexCoord0;

void main()
{
    gl_Position = osg_ProjectionMatrix * osg_Vertex;
    TexCoord0 = osg_MultiTexCoord0;
}
