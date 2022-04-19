
#version 330

//////////////////////////////////////////////////////////////////////
// Switches
//////////////////////////////////////////////////////////////////////

//#define TRACK_FMT_RAW     // raw data
//#define TRACK_FMT_SVA     // single variable area
#define TRACK_FMT_BVA       // bilateral variable area
#define TRACK_USE_GNR       // ground noise reduction

//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////

const float pi      = 3.14159265358979323846;
const float twoPi   = 2*pi;
const vec2  bit     = vec2(0,1);

//////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////

//x = value
#define TEXCOORD(x) (-1.+2.*(x)) 
//x = value
#define NDCCOORD(x) (.5+.5*(x))
//x = value
#define INVERT(x) (1.-(x))
//x = value
#define RECIPROC(x) (1./(x))
//x = value
#define INVMUL(x) RECIPROC(x)
//a = map, b = size
#define QUANTIZE(a,b) (floor((a)*(b))/(b))
//a = distance, b = map, c = value
#define SMOOTH(a,b,c) smoothstep((b)-(a),(b)+(a),c)
//x = value
#define CLAMP1(x) clamp((x),bit.x,bit.y)
//x = vector
#define CLAMP2(x) clamp((x),bit.xx,bit.yy)
//x = vector
#define CLAMP3(x) clamp((x),bit.xxx,bit.yyy)
//x = vector
#define CLAMP4(x) clamp((x),bit.xxxx,bit.yyyy)

//////////////////////////////////////////////////////////////////////

struct App
{
    float time;
    int frame;
    vec2 mouse;
    vec2 windowSize;
};

struct Channel
{
    sampler2D tex;
    vec2 siz;
};

struct Tile
{
    int index;
    int count;
    int channels;
    vec2 textureSize;
    vec2 displaySize;
    float progress;
};

uniform App app;
uniform Channel channel[4];
uniform Tile tile;

in vec2 texCoord;
out vec4 fragOut;

// Gets tile map space in 2D
// Returns: xy=coord_per_tile, z=index_per_tile, w=bounds
vec4 getMapTile2D(vec2 uv, vec2 res)
{
 	vec2 xy = uv * res;

    vec4 o;
    //Tiles space
    o.xy = fract(xy);
    // Index ascending over y
    //o.z = dot(floor(xy),vec2(res.y,1));
    // Index ascending over x
    o.z = dot(floor(xy),vec2(1,res.x));
    // Base space mask (incl. all tiles)
    o.w = float(dot(floor(uv),vec2(1)) == 0.0);

    return o;
}
// Gets tile map space in 1D
// Returns: xy=coord_per_tile, z=index_per_tile, w=bounds
vec4 getMapTile1D(vec2 uv, float width)
{
    return getMapTile2D(uv, vec2(width,1));
}
// Gets tile map space to build a line from 2D textures
// Returns: xy=coord_per_tile, z=index_per_tile, w=bounds
vec4 getMapTileLine(vec2 uv, float width, float cols, float offs)
{
    uv.x = ((uv.x*cols)+offs)/width;
    vec4 o = getMapTile1D(uv, width);
    o.y = o.z/width;
    return o;
}

/*
//Problema com uso de sample2D como argumento. Porque não compila?
vec4 getSample(vec2 uv) 
{
    int oversample = 4;
    vec4 o = vec4(0);
    float step = RECIPROC(float(oversample))
    for(int i=0; i<oversample; i++)
        o += NDCCOORD(texture(channel1.tex, vec2(step*float(i), uv.y)));
    return o / float(oversample);
}
*/

//vec4 CHANNEL_SAMPLE(int id, vec2 uv){return texture(channel[id].tex, vec2(0.5, uv.y));}
#define CHANNEL_SAMPLE(chn, map) (texture(channel[chn].tex, vec2(0.5, map.y)))
#define TRACK_SMOOTH(d,m,t) t.xy = SMOOTH(vec2(d), m.xx, t.xy)
#define TRACK_BLEND_MIX(a,b) a = mix(a, b, b.w)
#define TRACK_BLEND_ADD(a,b) a = vec4(a.xyz + b.xyz * b.w, a.w) // problema
#define TRACK_SELECT(chn, map) (o *= float(floor(map.z) != float(chn)))

// Gets vertical oriented track map
// Returns: xy=uv, z=mask
vec3 getMapTrackV(vec2 uv, int count, int index)
{
    uv.x *= float(count);

    vec3 o;
    o.z = float(floor(uv.x) != float(index));
    o.x = fract(uv.x);
    o.y = uv.y;

    #if defined TRACK_FMT_BVA
    o.x = abs(TEXCOORD(o.x)); // Bilateral map
    #endif

    return o;
}
// Gets horizontal oriented track map
// Returns: xy=uv, z=mask
vec3 getMapTrackH(vec2 uv, int count, int index)
{
    vec3 o;

    uv.y *= float(count);

    o.z = float(floor(uv.y) != float(index));
    o.x = uv.x;
    o.y = fract(uv.y);

    #if defined TRACK_FMT_BVA
    o.y = abs(TEXCOORD(o.y)); // Bilateral map
    #endif

    return o;
}
// Gets tile texture size relative to current mode
// Returns: x=width, y=height
vec2 getTileTextureSize()
{
    vec2 s0 = tile.textureSize;
    s0 /= float(tile.channels);
    #if defined TRACK_FMT_BVA
    s0 /= 2;
    #endif
    return s0;
}
// Gets tile display size relative to current mode
// Returns: x=width, y=height
vec2 getTileDisplaySize()
{
    vec2 s0 = tile.displaySize;
    s0 /= float(tile.channels);
    #if defined TRACK_FMT_BVA
    s0 /= 2;
    #endif
    return s0;
}
// Gets envelope value to be used for animation
// Returns: normalized value (0-1)
float getAnimEnvelope(int i)
{
    switch(i)
    {
        case 1:
            return INVERT(abs(INVERT(tile.progress*2))); //ping-pong
        default:
            return tile.progress; // linear
    }
}
// Gets w0 data from simulated shutters
// Returns: x=sound, y=envelope, z=enveloped sound
vec4 getWaveData(vec2 uv, int track, float d)
{
    vec4 o = bit.xxxx;

    // Sound data
    o.x = NDCCOORD(texture(channel[0].tex, uv)[track]); 
    // Envelope data
    o.y = NDCCOORD(texture(channel[1].tex, uv)[track]);
    // Output data
    o.z = o.x;

    #if defined TRACK_USE_GNR
    float biasLineWidth = d*2;
    float biasedShutter = INVERT(o.y);
    o.z += biasLineWidth - biasedShutter;
    #endif

    return o;
}

// Draws portion being sampled from the sound data channel
// Returns: color frag
vec4 drawReadLine(vec2 uv, vec2 res, float cols, float offs)
{
    vec2 p; vec4 m0, k, c1, c2;
    
    m0 = getMapTile2D(uv, res);

    p.x = offs*res.x; // beg offs
    p.y = cols*res.x; // end offs
    p.y+=p.x;
    
    k.x = dot(m0.xz, bit.yy);  // sum
    k.y = step(p.x, k.x);       // 1st line map
    k.z = step(k.x, p.y);       // 2nd line map
    k.x = k.y*k.z;              // diff
    k.yz-=k.xx;                 // subtract diff

    //RAW Sound data
    c1 = texture(channel[0].tex,uv);
    c1.rg = NDCCOORD(c1.rg);
    c1.rgb = bit.xyx * c1.r + bit.yxy * c1.g; //green/magenta
    c1.rgb /= 2; // dimmer

    //RAW fields
    c2 = bit.xxxy;
    c2.rgb += bit.xxy * k.y;
    c2.rgb += bit.xyx * k.z;
    c2.rgb += bit.yxx * k.x; // middle
    c2.rgb /= 2;

    return c1+c2;
}

vec4 drawTrackV(vec2 uv, int track)
{
    uv.x = INVERT(uv.x);

    vec4 t0,t1; // textures
    vec4 c0,c1; // colors
    vec2 s0; // sizes
    vec3 m0; // maps
    float d0; // distances

    m0 = getMapTrackV(uv, tile.channels, track);
    s0 = getTileTextureSize();
    d0 = RECIPROC(s0.x)*2; // mutiplies by 'n' pixels

    t0 = NDCCOORD(CHANNEL_SAMPLE(0, m0.xy));
    t1 = NDCCOORD(CHANNEL_SAMPLE(1, m0.xy));

    //FIXME: Single envelope to all channels
    #if defined TRACK_USE_GNR
    // Bias line width
    t0 += d0;
    // Shutter bias for ground noise reduction
    t0 -= INVERT(t1);
    #endif

    // For visualization purpose
    t1 = TEXCOORD(t1)-d0;
    
    TRACK_SMOOTH(d0,m0,t0);
    TRACK_SMOOTH(d0,m0,t1);

    c0 = vec4(bit.yyx, t0[track]);
    c1 = vec4(bit.yxx, t1[track]);

    vec4 o = bit.xxxx;

    //vec4 rawData = texture(channel[0].tex, m0.xy);
    //o.rgb += vec3(bit.xyy) * rawData.x;
    //o.rgb += vec3(bit.yxy) * rawData.y;
    
    TRACK_BLEND_MIX(o,c1);
    TRACK_BLEND_MIX(o,c0);
    //TRACK_BLEND_ADD(o,c2);
    //TRACK_BLEND_ADD(o,c0);

    return o * m0.z;
}

vec4 drawTrackH(vec2 uv, int track)
{
    float   shutterFeather  = 1;
    vec2    axisMode        = bit.xy;
    bool    useCursor       = false;
    bool    useStepping     = false;
    bool    useIntOffset    = true;
    int     clampMode       = 2;
    int     rangeMode       = 0;
    
    float size = dot(tile.textureSize, axisMode);

    // hdiv = size*4;     // 4x Oversample
    // hdiv = size        // 1H line sampling
    // hdiv = 2           // Half-height sampling
    float hdiv = size;
    float cols = size/hdiv;

    float envp, offs;

    if(useCursor)
        envp = dot(app.mouse/app.windowSize, axisMode);
    else
        envp = getAnimEnvelope(0);

    envp = dot(tile.textureSize, axisMode)*envp;

    switch(clampMode)
    {
        case 1: // Read inside bounds
            envp = clamp(envp, 0, size-cols);
            break;
        case 2: // Cursor inside bounds
            envp = clamp(envp, 0, size);
            break;
        case 3: // Cursor inside bounds, Read offset at cursor
            envp = clamp(envp, 0, size)-(cols/2);
            break;
    }

    envp /= size;

    if(useStepping) 
        envp = QUANTIZE(envp,hdiv);

    //TODO: Add some ranges
    switch(rangeMode)
    {
        //case 1: // Read inside bounds
        //    offs = mix(0, size-cols, envp);
        //    break;
        //case 3: // Cursor inside bounds, Read offset at cursor
        //    offs = mix(-cols, size+cols, envp);
        //    break;
        default:
            offs = mix(0, size, envp);
    }

    if(useIntOffset)
        offs = floor(offs);

    vec3 m0 = getMapTrackH(uv, tile.channels, track);
    vec4 m1 = getMapTileLine(m0.xy, size, cols, offs);
    
    ///////////////////////////////////////////////

    vec2 d0 = RECIPROC(getTileDisplaySize());

    vec4 w0 = getWaveData(m1.xy, track, d0.y);

    d0 *= shutterFeather;
    // Sound shutter mask
    float t0 = mix(0, SMOOTH(d0.y, m0.y, w0.z), m1.w);
    // Envelope shutter mask
    float t1 = mix(0, SMOOTH(d0.y, m0.y, TEXCOORD(w0.y)), m1.w);

    ///////////////////////////////////////////////

    vec4 o;
    //o = vec4(m0.xy, bit);
    //o = vec4(m1.xy, bit);
    o = drawReadLine(uv, tile.textureSize, cols, offs);
    o = mix(o, bit.yxyy, t1);   // envelope
    o = mix(o, bit.yyyy, t0);   // sound
    o = mix(o, bit.xxxx, m0.z); // track mask
    o = clamp(vec4(0),vec4(1),o);
    
    return o;
}

void main()
{
    vec2 uv = texCoord;

    fragOut = bit.xxxx;

    uv.x = INVERT(uv.x);
    for(int c=0; c<tile.channels; c++)
        fragOut += drawTrackH(uv,c);

    //for(int c=0; i<tile.channels; c++)
    //    fragOut += drawTrackV(uv,c);
}