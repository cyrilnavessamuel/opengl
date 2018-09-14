#version 330 core

// Values that stay constant for all fragments
uniform vec3 lightDirection; //direction of the light
uniform float specular_exponent;

// Interpolated values from the vertex shaders
in vec3 fragmentNormal;
in vec3 viewDirection;


// Ouput data
out vec3 color;

void main()
{
      //Light direction
      vec3 L = normalize(lightDirection);

      // normal is interpolated between all 3 surrounding vertices, it needs to be normalized
      vec3 N = normalize(fragmentNormal);

      //color = 0.5*N + vec3(0.5,0.5,0.5);
	  float diffuse = max(dot(-L,N),0);
	  
	  vec3 V = normalize(viewDirection);
	  vec3 R = normalize(reflect(L,N));

	  float specular = pow(max(dot(R,V),0),specular_exponent);

	  //Block Implemented for Cartoon Shading
	  if(diffuse<0.5){
	  diffuse = 0.5;
	  }
	  else{
	  diffuse = 1;
	  }
	  if(specular<0.5){
	  specular=0;
	  }
	  else{
	  specular=1;
	  }
	  //Block for Cartoon Shading
	  color = diffuse*vec3(0.8,0.6,0.6)+specular*vec3(0.7,0.2,0.2) ;
}