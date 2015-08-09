#version 140

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;

uniform usampler2D tilemapTex;

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;

out vec3 pos_viewspace;
out vec3 n_viewspace;
out vec3 t_viewspace;
out vec3 b_viewspace;
out vec4 TexCoords;
flat out uint TexIndex;

void main()
{
    int x = gl_InstanceID & 15;
    int y = gl_InstanceID / 16;
    vec4 pos = osg_Vertex + vec4(x*256, 0, y*-256, 0);

    gl_Position = osg_ModelViewProjectionMatrix * pos;
    TexCoords = osg_MultiTexCoord0;
    TexIndex = texelFetch(tilemapTex, ivec2(x, y), 0).r;

    pos_viewspace = (osg_ModelViewMatrix * pos).xyz;

    vec3 normal   = vec3(0.0, -1.0, 0.0);
    vec3 binormal = vec3(1.0,  0.0, 0.0);
    n_viewspace = normalize(mat3(osg_ModelViewMatrix) * normal);
    t_viewspace = normalize(mat3(osg_ModelViewMatrix) * cross(normal, binormal));
    b_viewspace = normalize(mat3(osg_ModelViewMatrix) * binormal);
}
