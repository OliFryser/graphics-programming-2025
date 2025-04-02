//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D SourceTexture;
uniform float Exposure;
uniform float Contrast;
uniform float HueShift;
uniform float Saturation;
uniform vec3 ColorFilter;

vec3 ApplyExposure(vec3 color)
{
	return (1.0 - exp(-color * Exposure)).rgb;
}

vec3 ApplyContrast(vec3 color)
{
	return clamp((color - vec3(0.5)) * Contrast + vec3(0.5), vec3(0), vec3(1));
}

vec3 ApplyHueShift(vec3 color)
{
	vec3 hsv = RGBToHSV(color);
	hsv.x = fract(hsv.x + HueShift);
	return HSVToRGB(hsv);
}

vec3 ApplySaturation(vec3 color)
{
	float luminance = GetLuminance(color);
	return clamp((color - vec3(luminance)) * Saturation + vec3(luminance), vec3(0), vec3(1));
}

vec3 ApplyColorFilter(vec3 color)
{
	return color * ColorFilter;
}

void main()
{
	vec3 color = ApplyExposure(texture(SourceTexture, TexCoord).rgb);
	color = ApplyContrast(color);
	color = ApplyHueShift(color);
	color = ApplySaturation(color);
	color = ApplyColorFilter(color);
	FragColor = vec4(color, 1.0);
}
