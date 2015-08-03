#version 130
#extension GL_ARB_texture_rectangle : enable

in vec3 localLightDir;

uniform vec4 ambient_color;
uniform vec4 diffuse_color;
uniform vec4 specular_color;

uniform sampler2DRect ColorTex;
uniform sampler2DRect NormalTex;
uniform sampler2DRect PosTex;

out vec4 DiffuseData;
out vec4 SpecularData;

void main()
{
    vec4 c_viewspace = texture2DRect(ColorTex,  gl_FragCoord.xy);
    vec3 n_viewspace = texture2DRect(NormalTex, gl_FragCoord.xy).xyz*2.0 - vec3(1.0);
    vec3 p_viewspace = texture2DRect(PosTex,    gl_FragCoord.xy).xyz;
    vec3 s_viewspace = vec3(1.0);

    // Direction from point to light (not vice versa!)
    vec3 lightDir_viewspace = normalize(-localLightDir);

    // Lambertian diffuse color.
    vec3 diff = max(ambient_color.rgb,
                    diffuse_color.rgb * s_viewspace * dot(lightDir_viewspace, n_viewspace)
    );

    // Direction from point to camera.
    vec3 viewDir_viewspace = normalize(-p_viewspace);

    // Blinn-Phong specular highlights.
    vec3 h_viewspace = normalize(lightDir_viewspace + viewDir_viewspace);
    float amount = max(0.0, dot(h_viewspace, n_viewspace));
    vec3 spec = specular_color.rgb * s_viewspace * pow(amount, 32.0) * c_viewspace.a;

    DiffuseData  = vec4(diff, 1.0);
    SpecularData = vec4(spec, 1.0);
}
