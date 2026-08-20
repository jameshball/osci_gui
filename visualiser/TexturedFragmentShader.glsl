std::string texturedFragmentShader = R"(

uniform sampler2D uTexture0;
varying vec2 vTexCoord;
uniform float uCropEnabled;
uniform float uPreserveAlpha;
uniform float uCheckerboardBackground;

void main() {
    gl_FragColor = texture2D(uTexture0, vTexCoord);
    if (uCheckerboardBackground > 0.5) {
        float checkerCell = mod(floor(gl_FragCoord.x / 12.0) + floor(gl_FragCoord.y / 12.0), 2.0);
        vec3 checkerColour = mix(vec3(0.22), vec3(0.34), checkerCell);
        gl_FragColor.rgb += checkerColour * (1.0 - gl_FragColor.a);
        gl_FragColor.a = 1.0;
    } else if (uPreserveAlpha < 0.5) {
        gl_FragColor.a = 1.0;
    }
}

)";
