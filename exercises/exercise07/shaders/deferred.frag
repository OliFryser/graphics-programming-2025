//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D DepthTexture;
uniform sampler2D AlbedoTexture;
uniform sampler2D NormalTexture;
uniform sampler2D OthersTexture;
uniform mat4 InvViewMatrix;
uniform mat4 InvProjMatrix;

void main()
{
	//FragColor = texture(AlbedoTexture, TexCoord);
	//FragColor = vec4(GetImplicitNormal(texture(NormalTexture, TexCoord).xy), 1.0);
	FragColor = vec4(ReconstructViewPosition(DepthTexture, TexCoord, InvProjMatrix), 1.0);
}
