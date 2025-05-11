//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D SourceTexture;
uniform sampler2D DepthTexture;

void main()
{
	FragColor = texture(SourceTexture, TexCoord);
	//gl_FragDepth = texture(DepthTexture, TexCoord).x;
}
