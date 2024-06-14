precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float alpha;
void main() {
    vec2 uv = vec2( 1.0 - v_texcoord.x, 1.0 - v_texcoord.y);
    uv.x = 1.0 - uv.x;
    vec4 color1 = texture2D(tex1, uv);
    vec4 color2 = texture2D(tex2, uv);
    gl_FragColor = mix(color1, color2, alpha);
}
