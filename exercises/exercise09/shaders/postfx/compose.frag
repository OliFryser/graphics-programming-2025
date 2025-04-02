//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D SourceTexture;
uniform float Exposure;

void main()
{
	FragColor = 1.0 - exp(-texture(SourceTexture, TexCoord) * Exposure);
}
