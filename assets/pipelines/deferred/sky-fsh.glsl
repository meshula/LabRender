
// sky-fsh.glsl

void main() {
	//o_diffuse_texture = vec4(texture(skyCube, var.v_eyeDirection).xyz, 1.0);
	o_diffuse_texture = vec4(0.5 * clamp(1.0 - var.v_eyeDirection.y, 0, 1), 0.5 * clamp(var.v_eyeDirection.y, 0, 1), 0, 1);
    o_diffuse_texture.x = 0;
}
