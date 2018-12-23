
void main()
{
    vec3 normal = texture(u_normal_texture, var.v_texCoord).xyz;
    float i = dot(normal, normal);
    if (i < 0.0001) {
        discard;
    }
    else
    {
        vec3 light = normalize(vec3(0.1, 0.4, 0.2));
        vec3 diffuse = texture(u_diffuse_texture, var.v_texCoord).xyz;
        i = dot(normal, light);
        o_color_texture = vec4(diffuse, 1) * i;
    }
}
