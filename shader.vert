#version 330

in vec3 vertexPosition;
in vec4 vertexColour;
in vec2 vertexUV;

out vec3 fragmentPosition;
out vec4 fragmentColour;
out vec2 fragmentUV;

uniform mat4 projectionMatrix = mat4(1.0);

void main() {
	gl_Position.xyz = (projectionMatrix * vec4(vertexPosition.xyz, 1.0)).xyz;
	gl_Position.w = 1.0;
	
	fragmentPosition = vertexPosition;
	fragmentColour = vertexColour;
	fragmentUV = vec2(vertexUV.x, 1.0 - vertexUV.y);
}
