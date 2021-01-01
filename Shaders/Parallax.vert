#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureUV;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec3 fragPos;
out vec2 texCoords;
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 viewMatrix;

uniform vec3 lightPosition;
uniform float eyeX;
uniform float eyeY;
uniform float eyeZ;

uniform sampler2D depthMap;
uniform sampler2D normalMap;

vec3 viewPos;

void main()
{				
    mat4 model = inverse(viewMatrix) * modelView;
    viewPos = vec3(eyeX, eyeY, eyeZ);

    fragPos = vec3(model * vec4(position, 1.0));   
    texCoords = textureUV;   
    
    vec3 T = normalize(mat3(model) * tangent);
    vec3 B = normalize(mat3(model) * bitangent);
    vec3 N = normalize(mat3(model) * normal);
    mat3 TBN = transpose(mat3(T, B, N));

    tangentLightPos = TBN * lightPosition;
    tangentViewPos  = TBN * viewPos;
    tangentFragPos  = TBN * fragPos;
    
    gl_Position = projection * modelView * vec4(position + texture(depthMap, textureUV).r * normal, 1.0);
}