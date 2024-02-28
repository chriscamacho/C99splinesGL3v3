//#version 330

/*--------------------------------------------------------------------*/
#if defined VERTEX_SHADER
/*--------------------------------------------------------------------*/

// The per sprite input data
in vec2 in_position;
in vec2 in_size;
in float in_rotation;

out vec2 size;
out float rotation;

void main() {
    // We just pass the values unmodified to the geometry shader
    gl_Position = vec4(in_position, 0, 1);
    size = in_size;
    rotation = in_rotation;
}

/*--------------------------------------------------------------------*/
#elif defined GEOMETRY_SHADER
/*--------------------------------------------------------------------*/

// We are taking single points form the vertex shader
// and emitting 4 new vertices creating a quad/sprites
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 projection;

// Since geometry shader can take multiple values from a vertex
// shader we need to define the inputs from it as arrays.
// In our instance we just take single values (points)
in vec2 size[];
in float rotation[];

// Outputs to fragment shader
out vec2 uv;


void main() {
    // We grab the position value from the vertex shader
    vec2 center = gl_in[0].gl_Position.xy;
    // Calculate the half size of the sprites for easier calculations
    vec2 hsize = size[0] / 2.0;
    // Convert the rotation to radians
    float angle = radians(rotation[0]);

    // Create a 2d rotation matrix
    mat2 rot = mat2(
        cos(angle), sin(angle),
        -sin(angle), cos(angle)
    );

    // Emit a triangle strip creating a quad (4 vertices).
    // Here we need to make sure the rotation is applied before we position the sprite.
    // We just use hardcoded texture coordinates here. If an atlas is used we
    // can pass an additional vec4 for specific texture coordinates.

    // Each EmitVertex() emits values down the shader pipeline just like a single
    // run of a vertex shader, but in geomtry shaders we can do it multiple times!

    // Upper left
    gl_Position = projection * vec4(rot * vec2(-hsize.x, hsize.y) + center, 0.0, 1.0);
    uv = vec2(0, 1);
    EmitVertex();

    // lower left
    gl_Position = projection * vec4(rot * vec2(-hsize.x, -hsize.y) + center, 0.0, 1.0);
    uv = vec2(0, 0);
    EmitVertex();

    // upper right
    gl_Position = projection * vec4(rot * vec2(hsize.x, hsize.y) + center, 0.0, 1.0);
    uv = vec2(1, 1);
    EmitVertex();

    // lower right
    gl_Position = projection * vec4(rot * vec2(hsize.x, -hsize.y) + center, 0.0, 1.0);
    uv = vec2(1, 0);
    EmitVertex();

    // We are done with this triangle strip now
    EndPrimitive();
}

/*--------------------------------------------------------------------*/
#elif defined FRAGMENT_SHADER
/*--------------------------------------------------------------------*/

uniform sampler2DArray texture0;

uniform vec4 in_tint;
uniform int tex;

in vec2 uv;

out vec4 fragColor;

void main() {
    
    fragColor = texture(texture0, vec3(uv, tex)) * in_tint;
    //fragColor = in_tint;

}

#endif
