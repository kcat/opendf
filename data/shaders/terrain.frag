#version 130

uniform vec4 illumination_color;

uniform sampler2DArray diffuseTex;
uniform sampler2D tilemapTex;

in vec3 pos_viewspace;
in vec3 n_viewspace;
in vec3 t_viewspace;
in vec3 b_viewspace;
in vec4 TexCoords;

out vec4 ColorData;
out vec4 NormalData;
out vec4 PositionData;
out vec4 IlluminationData;

void main()
{
    float idx = texelFetch(tilemapTex, ivec2(TexCoords.xy), 0).r * 255.0;
    vec4 color = vec4(texture(diffuseTex, vec3(TexCoords.xy, idx)).rgb, 0.0);
    vec4 nn = vec4(0.5, 0.5, 1.0, 1.0);

    mat3 nmat = mat3(normalize(t_viewspace),
                     normalize(b_viewspace),
                     normalize(n_viewspace));

    ColorData    = color;
    NormalData   = vec4(nmat*(nn.xyz - vec3(0.5)) + vec3(0.5), nn.w);
    PositionData = vec4(pos_viewspace, gl_FragCoord.z);
    IlluminationData = illumination_color;
}
