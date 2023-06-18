#version 330 core
out vec4 FragColor;

in vec2 uv;
in vec3 worldPosition;

uniform sampler2D mainTex;
uniform sampler2D normalTex;

uniform sampler2D dirt, sand, rock;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

vec2 lerp(vec2 a, vec2 b, float t)
{
    return a + (b - a) * t;
}

void main()
{
    //Normal Map
    vec3 normal = texture(normalTex, uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal.gb = normal.bg;
    normal.r = -normal.r;
    normal.b = -normal.b;
    
    //Specular Data
    vec3 viewDir = normalize(worldPosition.rgb - cameraPosition);
    //vec3 reflDir = normalize(reflect(lightDirection, normal));
    
    //lighting
    float lightValue = max(-dot(normal, lightDirection), 0.0);
    //float specular = pow(max(-dot(reflDir, viewDir), 0.0), 8);
    
    //build color!
    float y = worldPosition.y;
    
    float ds = clamp((y - 100) / 10 , -1, 1) * 0.5 + 0.5;
    float sr = clamp((y - 250) / 10, -1, 1) * 0.5 + 0.5;

    float dist = length(worldPosition.xyz - cameraPosition);
    float uvLerp = clamp((dist - 250) / 150, -1, 1) * 0.5 + 0.5;
    
    vec3 dirtColorClose = texture(dirt, uv * 100).rgb;
    vec3 sandColorClose = texture(sand, uv * 100).rgb;
    vec3 rockColorClose = texture(rock, uv * 100).rgb;
    
    vec3 dirtColorFar = texture(dirt, uv * 10).rgb;
    vec3 sandColorFar = texture(sand, uv * 10).rgb;
    vec3 rockColorFar = texture(rock, uv * 10).rgb;

    vec3 dirtColor = lerp(dirtColorClose, dirtColorFar, uvLerp);
    vec3 sandColor = lerp(sandColorClose, sandColorFar, uvLerp);
    vec3 rockColor = lerp(rockColorClose, rockColorFar, uvLerp);

    vec3 diffuse = lerp(lerp(dirtColor, sandColor, ds), rockColor, sr);
    
    float fog = pow(clamp((dist - 250) / 1000, 0, 1), 2);
    
    vec3 topColor = vec3(255.0 / 255.0, 118.0 / 255.0, 189.0 / 255.0);
    vec3 botColor = vec3(188.0 / 255.0, 214.0 / 255.0, 231.0 / 255.0);
    
    vec3 fogColor = lerp(botColor, topColor, max(viewDir.y, 0.0));
    
    //Separate RGB and RGBA Calculations
    vec4 output = vec4(diffuse * min(lightValue + 0.8, 1.0), 1.0); // //+specular * output.rgb;
    
    FragColor = output;

}