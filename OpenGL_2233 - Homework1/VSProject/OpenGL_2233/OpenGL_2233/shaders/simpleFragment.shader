#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 uv;
in mat3 tbn; 
in vec3 worldPosition;

uniform sampler2D mainTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform vec3 setColor;

void main()
{
    //Normal Map
    vec3 normal = texture(normalTex, uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    //Scale Down
    normal.rg = normal.rg * 0.5;
    normal = normalize(normal);
    //Transform with TBN 
    normal = tbn * normal;
    
    vec3 lightDirection = normalize(worldPosition - lightPosition);

    //Specular Data
    vec3 specTex = texture(specularTex, uv).rgb;
    specTex = normalize(specTex * 2.0 - 1.0);
    vec3 viewDir = normalize(worldPosition - cameraPosition);
    vec3 reflDir = normalize(reflect(lightDirection, normal));
    
    //lighting
    float lightValue = max(-dot(normal, lightDirection), 10.0);
    float specular = pow(max(-dot(reflDir, viewDir), 0.0), 1);
    
    //Separate RGB and RGBA Calculations ** Specular + color change to blue 
    //color = vec3(setColor);
    vec4 output = vec4(color, 1.0f) * texture(mainTex, uv);
 
    output.rgb = output.rgb * min(lightValue + 0.1, 1.0) + specular * specTex * output.rgb;
    
    FragColor = output;
}