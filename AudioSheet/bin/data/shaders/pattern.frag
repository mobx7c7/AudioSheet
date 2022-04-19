#version 330

#define PI 3.14159265358979323846
#define TWO_PI 2*PI
#define CLAMP1(x) clamp(0,1,x)
#define SVALUE(x) -1.+2.*x
#define UVALUE(x) .5+.5*x

struct Rect{vec2 pos, siz;};

struct Bounds{vec2 xrange, yrange;};

struct View
{
    vec2 mag;
    vec4 map; //xy=uv, z=sdf, w=mask
    Rect window;
    Bounds margin;
};

struct Channel
{
    sampler2D tex;
    vec2 siz;
};

struct Page
{
    int tracks;
    int bandguard;
    Bounds margin;
};

in vec2 texCoord;
uniform Page page;
uniform Rect screen;
uniform Rect window;
uniform Channel channel0;
uniform Channel channel1;
uniform Channel channel2;
uniform Channel channel3;
out vec4 fragOut;

float osc_triangle(float t, float f)
{
    float a, b, c;
    // 2x ramp 1/2
    a = 2.*fract(t*f); 
    // bias
    b = SVALUE(floor(a));
    // 2x ramp 1/4
    c = SVALUE(fract(a));

    return b*(1.-abs(c));
}

float osc_rectangle(float t, float f, float d)
{
    return smoothstep(-d, d, osc_triangle(t, f));
}
//Returns: sdf
float sd_square(vec2 uv)
{
    uv = abs(SVALUE(uv));
    return max(uv.x,uv.y);
}
//Returns: vec4(xy=uv, z=sdf, w=mask)
vec4 sd_rect(vec2 uv, Rect src, Rect dst)
{
    uv *= dst.siz;
    uv -= src.pos;
    uv /= src.siz;

    float sd = sd_square(uv);

    return vec4(uv, sd, step(sd,1));
}
//Returns: vec4(xy=uv, z=sdf, w=mask)
vec4 sd_bounds(vec2 uv, Rect r, Bounds b)
{
    vec2 kmin = vec2(b.xrange.x, b.yrange.x);
    vec2 kmax = vec2(b.xrange.y, b.yrange.y);

    vec2 bsize = r.siz - kmin + kmax;
    //vec2 bsize = r.siz - kmax * 2;
    
    uv *= r.siz;
    uv -= kmin;
    uv /= bsize;

    float sd = sd_square(uv);

    return vec4(uv, sd, step(sd,1));
}

Rect view_add_bounds(Rect r, Bounds b)
{   
    vec2 kmin = vec2(b.xrange.x, b.yrange.x);
    vec2 kmax = vec2(b.xrange.y, b.yrange.y);
    r.pos += kmin;
    r.siz -= kmin+kmax;
    return r;
}

void view_add_bounds2(Bounds b, out Rect r)
{   
    vec2 kmin = vec2(b.xrange.x, b.yrange.x);
    vec2 kmax = vec2(b.xrange.y, b.yrange.y);
    r.pos += kmin;
    r.siz -= kmin+kmax;
}


void view_init_map(vec2 uv, out View view)
{
    Rect r1 = view.window;

    view_add_bounds2(view.margin, r1);

    view.map = sd_rect(uv, r1, view.window);
}

vec4 view_get_map(View src, View dst)
{
    Rect r1 = src.window;
    
    view_add_bounds2(r1, src.margin);

    vec4 o = sd_rect(dst.map.xy, r, dst.window);
    
    o.w *= dst.map.w;
    
    return o;
}



void view_get_map2(View src, View dst)
{
    Rect r1 = src.window;
    
    view_add_bounds2(r1, src.margin);

    vec4 o = sd_rect(dst.map.xy, r, dst.window);
    
    o.w *= dst.map.w;
    
    return o;
}

void view_get_root(vec2 uv, out View view)
{
    view.mag = vec2(1); // 1:1
    view.window.siz = vec2(window.siz);
    view_init_map(uv, view);
}


//
/*
vec4 tracks_sdf(View view, int count, int bandguard)
{
    vec4 o;
    vec2 uv = view.map.xy;
    // coord map
    o.xy = fract(uv * vec2(1,count));
    o.y = abs(SVALUE(o.y));
    
    // index map
    o.z = floor(dot(uv,vec2(0,1)) * count) /count;
    // track mask
    o.w = view.map.w;
    // bandguard mask
    float q0 = 1/(view.window.siz.y/count/2);
    q0*=bandguard;
    o.w *= step(o.y,1-q0);
    return o;
}
*/
vec4 tracks_sdf(View view, int count, int bandguard)
{
    vec4 o;
    vec2 uv = view.map.xy;

    // coord map
    o.xy = uv;
    o.xy = fract(o.xy * vec2(1,count));

    float q0 = count/float(bandguard);
    
    {
        //
       // o.y /= 1-q0*2;
       //o.y+=q0;
        //o.y-=q0;
        //o.y/=q0;
       //o.y = abs(SVALUE(o.y));
      
    }
    
    // index map
    //o.z = floor(dot(uv,vec2(0,1)) * count) /count;

    // track mask
    o.w = view.map.w;
    o.w *= step(o.x,1.)*step(0.,o.x);
    o.w *= step(o.y,1.)*step(0.,o.y);

    return o;
}

float smoothstep2(float x, float y, float d)
{
    return smoothstep(x-d, x+d, y);
}

void view_add_margin(vec2 xrange, vec2 yrange, out View v)
{
    Bounds b2;
    b2 = v.margin;
    b2.xrange += xrange;
    b2.yrange += yrange;
    v.margin = b2;
}

void view_add_margin(Bounds b, out View v)
{
    Bounds b2;
    b2 = v.margin;
    b2.xrange += b.xrange;
    b2.yrange += b.yrange;
    v.margin = b2;
}

vec4 tracks_main(View view)
{
    //int tracks = 32; // units
    //int bandguard = 10; // pixels

    View v0 = view;
    View v1 = view;
    View v2 = view;

    view_add_margin(page.margin.xrange, page.margin.yrange, v1);
    v1.map = view_get_map(v1, v0);

    view_add_margin(vec2(page.bandguard), vec2(page.bandguard), v2);
    v2.map = view_get_map(v2, v1);

    vec4 map = tracks_sdf(v2, page.tracks, page.bandguard);

    float wave = texture(channel0.tex, map.xz*vec2(.15,1)).r;
    wave *= 0.85;

    float d = page.tracks/window.siz.x;
    wave = smoothstep2(wave,map.y,d*10.);

    //wave = step(wave,map.y)
    //return  vec4(vec3(wave)*map.w, v1.map.w);
    return map;
}
//
vec4 grid_main(vec2 uv, float freq)
{
    uv.y *= window.siz.y/window.siz.x;

    //freq = window.siz.x/freq;

    float d = freq/window.siz.x;
    vec4 o;
    vec2 clk;

    float d0 = d*4;
    clk.x = osc_rectangle(uv.x, freq, d0);
    clk.y = osc_rectangle(uv.y, freq, d0);

    //o.xy = clk;
    o.z = max(clk.x,clk.y)-min(clk.x,clk.y);
    o.w = 1;
    
    return o;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / window.siz.xy;

    //vec4 c0 = vec4(0.8,0,0,1); //grid_main(uv,10);

    //vec4 c1 = tracks_main(view_get_root(uv));
   
    //fragOut = mix(c0,c1,c1.a);

    View view;
    view_get_root(uv, view);

    fragOut = view.map;
}