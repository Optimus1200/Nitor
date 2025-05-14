#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float outlineThickness; // e.g., 1.0 (in pixels)

void main()
{
    // Model space -> World space
    vec4 worldPosition = model * vec4(aPos, 1.0);
    vec3 worldNormal = mat3(transpose(inverse(model))) * aNormal; // Correct normal transform

    // World space -> View (camera) space
    vec4 viewPosition = view * worldPosition;
    vec3 viewNormal = normalize(mat3(view) * worldNormal); // transform normal to view space

    // Offset along view-space normal by a fixed screen-space amount
    float offsetAmount = outlineThickness * 0.001; // You can tweak the scaling
    viewPosition.xyz += viewNormal * offsetAmount;

    // View space -> Clip space
    gl_Position = projection * viewPosition;
}
