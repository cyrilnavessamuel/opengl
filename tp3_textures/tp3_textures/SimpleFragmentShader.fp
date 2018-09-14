#version 330 core

// Values that stay constant for all fragments
uniform vec3 lightDirection; //direction of the light
// Texture sampler [Q2b]
 uniform sampler2D myTextureSampler;

// Interpolated values from the vertex shaders
in vec3 fragmentNormal;
in vec3 viewDirection;
// Interpolated texture coordinates [Q1c]
in vec2 outtexture_coordinates;

//Q4 input 3D world co-ordinates
in vec3 threedworldcoordinates;

// Ouput data
out vec3 color;


void main()
{
      vec3 L = normalize(lightDirection);

      // normal is interpolated between all 3 surrounding vertices, it needs to be normalized before shading computation
      
	  //[Q3 for Phong Shading]
	  vec3 N = normalize(fragmentNormal);
	  vec3 V = normalize(viewDirection);
	  vec3 R = normalize(reflect(L,N));

      // Diffuse shading
      float diffuse = max(dot(N,-L),0.);
     // color = vec3(diffuse,diffuse,diffuse);

	 /*
	  //[Q1C for passing interpolated values of texture co-ordinates]
	  color = vec3(outtexture_coordinates[0], outtexture_coordinates[1],0);
	  */

	  /*
	  //[Q2 for defining checkerboard texture and then applying the texture on the object]
	 color=texture(myTextureSampler,outtexture_coordinates).rgb;
	 */

	 /*
	 //[Q3 for Phong Shading]
	  float q =90.0;
	  float q_shiny = 10.0;
	  float phong=diffuse+pow(max(dot(R,V),0),q);
	  float phong_shiny=diffuse+pow(max(dot(R,V),0),q_shiny);
	  vec3 color_check=texture(myTextureSampler,outtexture_coordinates).rgb;
	  if(color_check==vec3(1,1,1)){
	      color = vec3(1,0.5,0)*phong;
	  }
	  else if(color_check==vec3(0,0,0)){
	  	 color = vec3(1,0,0)*phong_shiny;
	  }
	  */

	  /*
	  //Q4 for Spherical Co-ordinates
	  float radius = pow((pow(threedworldcoordinates.x, 2) + pow(threedworldcoordinates.y, 2)), 0.5);
      radius = mod(radius, 0.1);
      if(radius > 0.04) {
		radius = 0.0;
		}
	  else {
	    radius = 1.0;
		}
      vec3 color_check = vec3(radius);
	   if(color_check==vec3(1.0,1.0,1.0)){
	      color = vec3(0.0,0.0,1.0);
	  }
	  else if(color_check==vec3(0.0,0.0,0.0)){
	  	 color = vec3(0.0,1.0,0.0);
	  }
	  */
	  
	  //Q5 for Environment Mapping
	 float phi = acos(R.y) / 3.1416;
	 float theta = 0.5*(1.0 + atan(R.x,-R.z) / 3.1416);
     vec2 sphered = vec2(theta, phi);
     color = texture(myTextureSampler, sphered).rgb;
	  
}