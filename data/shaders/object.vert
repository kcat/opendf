#version 130

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;

in vec4 osg_Vertex;
in vec3 osg_Normal;
in vec3 osg_MultiTexCoord1; // Binormal
in vec4 osg_Color;
in vec4 osg_MultiTexCoord0;

out vec3 pos_viewspace;
out vec3 n_viewspace;
out vec3 t_viewspace;
out vec3 b_viewspace;
out vec4 TexCoords;
out vec4 Color;

void main()
{
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
    TexCoords = osg_MultiTexCoord0;
    Color = osg_Color;

    pos_viewspace = (osg_ModelViewMatrix * osg_Vertex).xyz;

    n_viewspace   = normalize(mat3(osg_ModelViewMatrix) * osg_Normal);
    b_viewspace   = normalize(mat3(osg_ModelViewMatrix) * osg_MultiTexCoord1);
    t_viewspace   = cross(n_viewspace, b_viewspace);
}
