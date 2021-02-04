#shader vertex
#version 330 core

layout (location = 0) in vec4 position;

void main()
{
    gl_Position = vec4(position.xy, 0., 1.);
}

#shader fragment
#version 330 core

out vec4 fragColor;
uniform vec2 resolution;
uniform sampler3D diffTexture;
uniform float runTime;
float signcos(in float v)
{
    return cos(v) * .5 + .5;
}
float height(in vec2 p)
{
    vec2 uv = p;
    float res = 1.;
    for (int i = 0; i < 3; i++)
    {
        res += cos(uv.y * 12.345 - runTime * 4. + cos(res * 12.234) * .2 +
    cos(uv.x * 32.2345 + cos(uv.y * 17.234)) ) + cos(uv.x * 12.345);
        uv = uv.yx;
        uv.x += res * .1;
    }
    return res;
}
vec2 normal(in vec2 p)
{
    const vec2 NE = vec2(.1, .0);
    return normalize(vec2( height(p + NE) - height(p - NE),
   height(p + NE.yx) - height(p - NE.yx) ));
}

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy - .5;
    vec3 lightDir = normalize(vec3(sin(runTime), 1., cos(runTime)));
    vec3 norm3d = normalize(vec3(normal(uv), 1.).xzy);
    vec3 diff = vec3(.4,.4, .4);
    diff *= .25 + max(0., dot(norm3d, lightDir));
    vec3 spec = vec3(.774597, .774597, .774597);
    fragColor = vec4(mix(diff,spec,.5), 1.);
}