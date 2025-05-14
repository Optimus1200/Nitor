#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec3 FragPos;

// Structs

struct Material
{
    sampler2D albedo;
    sampler2D normal;
    sampler2D roughness;
    sampler2D metallic;
    sampler2D occlusion;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
};

struct PointLight
{
    vec3 position;
    vec3 color;
};

// SSBO's

layout (std430, binding = 0) buffer LightSpaceMatrices
{
    mat4 lightSpaceMatrices[];
};

layout (std430, binding = 1) buffer CasCadePlaneDistances
{
    float cascadePlaneDistances[];
};

// Uniforms

uniform Material            material;
uniform DirectionalLight    directionalLight;
uniform vec3                cameraPosition;
uniform float               cameraFarPlane;
uniform mat4                view;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdflUT;

uniform sampler2DArray shadowMap;

// Other

const float PI = 3.14159265359;

// Bayer Dithering Matrix (4x4)
const float bayerMatrix[16] = float[](
    0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
    12.0/16.0, 4.0/16.0, 14.0/16.0,  6.0/16.0,
    3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
    15.0/16.0, 7.0/16.0, 13.0/16.0,  5.0/16.0
);

// Offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[](
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.normal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 calculateDirectionalLight(DirectionalLight dirLight, vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness)
{
    vec3 L = normalize(-dirLight.direction);
    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Diffuse component
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = dirLight.color;

    // Combine diffuse and specular
    return (kD * albedo / PI + specular) * radiance * NdotL;
}
// ----------------------------------------------------------------------------
vec3 calculatePointLight(PointLight pointLight, vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness)
{
    // calculate per-light radiance
        vec3 L = normalize(pointLight.position - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(pointLight.position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = pointLight.color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        return (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}
// ----------------------------------------------------------------------------
float calculateShadow(DirectionalLight dirLight, vec3 fragPosWorldSpace)
{
    // Select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    int cascadeCount = cascadePlaneDistances.length();
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    // Beyond the last cascade? No shadow.
    if (layer == -1)
    {
        return 0.0;
    }

    // Apply normal offset to reduce peter panning
    vec3 normal = normalize(Normal);
    float normalOffsetScale = 0.02; // Adjust based on scene scale
    vec3 offsetPos = fragPosWorldSpace + normal * normalOffsetScale;

    // Transform to light space
    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(offsetPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Clamp to avoid edge artifacts
    projCoords.xy = clamp(projCoords.xy, 0.0, 1.0);
    if (projCoords.z > 1.0)
    {
        return 0.0;
    }

    // Calculate bias (slope-scaled + cascade-aware)
    float bias = max(0.005 * (1.0 - dot(normal, -dirLight.direction)), 0.001);
    bias *= cascadePlaneDistances[layer] / cameraFarPlane; // Scale bias with cascade

    // PCF Soft Shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0).xy);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}
// ----------------------------------------------------------------------------
vec3 applyDithering(vec3 color, ivec2 pixelCoords)
{
    int index = (pixelCoords.y % 4) * 4 + (pixelCoords.x % 4);
    float ditherValue = bayerMatrix[index];
    return color + vec3(ditherValue) / 256.0;
}
// ----------------------------------------------------------------------------

void main()
{
    vec3 albedo     = pow(texture(material.albedo, TexCoords).rgb, vec3(2.2));
    float metallic  = texture(material.metallic, TexCoords).r;
    float roughness = texture(material.roughness, TexCoords).r;
    float ao        = texture(material.occlusion, TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(cameraPosition - WorldPos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    float shadow = calculateShadow(directionalLight, FragPos);
    
    //Lo += calculateDirectionalLight(directionalLight, N, V, F0, albedo, metallic, roughness);
    Lo += (calculateDirectionalLight(directionalLight, N, V, F0, albedo, metallic, roughness) * (1.0 - shadow));
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = albedo * ao * 0.15;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));
    // dithering
    ivec2 pixelCoords = ivec2(gl_FragCoord.xy);
    color = applyDithering(color, pixelCoords);

    FragColor = vec4(color, 1.0);

    //FragColor = vec4(normalize(N) * 0.5 + 0.5, 1.0);
}