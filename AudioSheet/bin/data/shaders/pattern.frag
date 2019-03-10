// #version 150 // para OGL 3.2
#version 330 // para OGL 3.3

#define PI 3.14159265358979323846
#define TWO_PI 2*PI=
#define CLAMP1(x) clamp(0,1,x)
#define SVALUE(x) -1.+2.*x
#define UVALUE(x) .5+.5*x

struct Rect
{
    vec2 pos, siz;
};

struct Channel
{
    sampler2DRect tex;
    vec2 siz;
};

in vec2 texCoord;

uniform Rect 
    screen,
    window;
uniform Channel 
    channel0,
    channel1,
    channel2,
    channel3;

out vec4 fragOut;

vec4 grid()
{
    vec2 uv = gl_FragCoord.xy / window.siz.xx;
    uv *= 10.0;
    uv = fract(uv);
    uv = abs(-1.+2.*uv);
    
    vec4 color;

    float d1 = 64.0/window.siz.x;
    float d2 = 8.0/window.siz.x;
    float s0 = max(uv.x,uv.y);
    float k0 = smoothstep(d1,d1+d2,1.-s0);
    vec3 c0 = mix(vec3(0),vec3(1), k0);
    
    color.a = 1.-k0;

    return color;
}

float oscTriangle(float time, float freq)
{
    float a, b, c;
    // 2x ramp 1/2
    a = 2.*fract(time*freq); 
    // bias
    b = SVALUE(floor(a));
    // 2x ramp 1/4
    c = SVALUE(fract(a));

    return b*(1.-abs(c));
}

float oscRectangle(float time, float freq, float d)
{
    d*=freq;
    return smoothstep(d, -d, oscTriangle(time, freq));
}

void main()
{
    vec2 uv = gl_FragCoord.xy / window.siz;
    vec4 color;

    float d0 = 10.0/window.siz.x;

    float clk = oscRectangle(uv.y, 32.0, d0);

    color.a = clk;

    fragOut = color;
}