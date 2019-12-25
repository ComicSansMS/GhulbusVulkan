#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

float plot(vec2 st, float pct){
  return  smoothstep( pct-0.02, pct, st.y) -
          smoothstep( pct, pct+0.02, st.y);
}

float disc(vec2 P, float size){
    return length(P) - size/2;
}

vec4 outline(float distance,    //Signed distance to line
            float linewidth,    //Stroke line width
            float antialias,    //Stroke antialiased area
            vec4 stroke,        //Stroke color
            vec4 fill)          //Fill color
{
    float t = linewidth / 2.0 - antialias;
    float signed_distance = distance;
    float border_distance = abs(signed_distance) - t;
    float alpha = border_distance / antialias;
    
    alpha = exp(-alpha*alpha);
    
    if (border_distance < 0.0)
        return stroke;
    else if (signed_distance < 0.0)
        return mix(fill, stroke, sqrt(alpha));
    else
        return vec4(stroke.rgb, stroke.a*alpha);
}


void main()
{
    //outColor = texture(texSampler, fragTexCoord);
    //outColor = vec4(fragTexCoord.yyy, 1);
    
    /*
    vec2 st = fragTexCoord;

    float y = st.x;

    vec3 color = vec3(y);

    // Plot a line
    float pct = plot(st,y);
    color = (1.0-pct) * color+pct * vec3(0.0,1.0,0.0);

	outColor = vec4(color,1.0);
    */
    /*
    vec2 P = gl_PointCoord.xy - vec2(0.5, 0.5);
    //P = vec2(rotation.x*P.x - rotation.y*P.y,rotation.y*P.x + rotation.x*P.y);
    float v_size = 1.0;
    float size = 0.2;
    vec4 fg_color = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 bg_color = vec4(0.0, 1.0, 0.0, 1.0);
    float linewidth = 0.05;
    float antialias = 0.01;
    float distance = disc(P*v_size, size);
    outColor = outline(distance, linewidth, antialias, fg_color, bg_color);
    */
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
