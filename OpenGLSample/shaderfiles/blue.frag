#version 330 core
in vec4 ourColor;
in vec2 TexCoord;

out vec4 color;

// Texture samplers
uniform sampler2D ourTexture1;

void main()
{
    // Linearly interpolate between both textures (second texture is only slightly combined)
    color = vec4(0.0, 0.0, 1.0, 1.0); // blue
}