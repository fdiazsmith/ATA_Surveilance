precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float alpha;
void main() {
    vec4 color1 = texture2D(tex1, v_texcoord);
    vec4 color2 = texture2D(tex2, v_texcoord);
    gl_FragColor = mix(color1, color2, alpha);
}
