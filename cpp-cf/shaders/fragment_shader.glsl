precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float time;
uniform float alpha;

// create a noise generator with a random seed
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    vec2 uv = vec2(1.0 - v_texcoord.x, 1.0 - v_texcoord.y);
    uv.x = 1.0 - uv.x;

    // Normalize time to avoid large values
    float norm_time = mod(time, 1000.0) * 0.001;
    vec4 color1 = texture2D(tex1, uv);

    // Use normalized time in the random function
    float rndm = random(uv + norm_time);
    vec4 color2 = vec4(rndm, rndm, rndm, 1.0);

    gl_FragColor = mix(color1, color2, alpha);
}
