
// sky-fsh.glsl

out vec4 fragColor;

void main() {
	//fragColor = vec4(texture(skyCube, vert.v_eyeDirection).xyz, 1.0);
	fragColor = vec4(0.5 * clamp(1.0 - vert.v_eyeDirection.y, 0, 1), 0.5 * clamp(vert.v_eyeDirection.y, 0, 1), 0, 1);
}
