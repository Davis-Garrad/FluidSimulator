#version 330

in vec3 fragmentPosition;
in vec4 fragmentColour;
in vec2 fragmentUV;

out vec4 colour0;

uniform sampler2D textureSampler;

void main() {
	float density = texture(textureSampler, fragmentUV.xy).r;
	
	if(density <= 0.3) {
		colour0.r = density;
	}
	if(density > 0.3) {
		colour0.r = 0.6;
	}
	if(density > 0.6) {
		colour0.r = 0.9;
	}
	if(density > 0.9) {
		colour0.r = 1.0;
	}
	
	colour0 = vec4(colour0.r, colour0.r, colour0.r, 1.0);
	colour0.rgb *= vec3(0.5137, 0.8431, 0.9333);
}
