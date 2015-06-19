uniform sampler2D u_texture;
uniform int u_time;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 o_color;

void main(void) {
	vec2 uv = v_texcoord.xy;
	float offset = sin(uv.y * 50.0 + float(u_time / 1000.0)) * 0.01;
	uv.y += offset;
	uv.x += offset;
	vec4 color = texture2D(u_texture, uv);
	vec4 fcolor = v_color / 255.0;
	o_color = color * fcolor * 255.0;
}
