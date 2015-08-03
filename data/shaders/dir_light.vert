#version 130

uniform mat4 osg_ProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;

uniform vec3 light_direction;

in vec4 osg_Vertex;

out vec3 localLightDir;

void main()
{
    localLightDir = mat3(osg_ModelViewMatrix) * light_direction;

    // Vertex position in main camera Screen space.
    gl_Position = osg_ProjectionMatrix * osg_Vertex;
}
