//#version 330

/*--------------------------------------------------------------------*/
#if defined VERTEX_SHADER
/*--------------------------------------------------------------------*/

uniform mat4 MP;
uniform vec2 sz;
uniform float depth;
in          vec2 vPos;
in          vec2 vUV;
out     vec2 UVfrag;

void main()
{
    UVfrag = vUV;
    //gl_Position = MP * vec4(vPos*sz, depth, 1.0);
    gl_Position = MP * vec4(vPos, 0, 1.0);
}

/*--------------------------------------------------------------------*/
#elif defined FRAGMENT_SHADER
/*--------------------------------------------------------------------*/

uniform sampler2DArray texture0;
uniform int tex;
out     vec4    colour;
in      vec2    UVfrag;

void main()
{
    colour = texture(texture0, vec3(UVfrag, tex)); 
    colour +=vec4(1,1,1,1);
}

#endif
