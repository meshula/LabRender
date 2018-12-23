
out vec4 fragColor;

void main(void) {
	vec4 color = vec4(0.0,0.0,0.0,1.0);
	vec2 uv = vec2(var.v_texCoord.x * iResolution.x, var.v_texCoord.y * iResolution.y);
	mainImage(color, uv);
	color.w = 1.0;
	fragColor = color;
}
