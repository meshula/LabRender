
// sky-fsh.glsl

void main() {
	//o_diffuseTexture = vec4(texture(skyCube, vert.v_eyeDirection).xyz, 1.0);
	o_diffuseTexture = vec4(0.5 * clamp(1.0 - vert.v_eyeDirection.y, 0, 1), 0.5 * clamp(vert.v_eyeDirection.y, 0, 1), 0, 1);
    o_diffuseTexture.x = 0;
}
