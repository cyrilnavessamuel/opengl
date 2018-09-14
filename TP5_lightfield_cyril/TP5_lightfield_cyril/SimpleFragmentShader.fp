#version 330 core

// Interpolated values from the vertex shaders
in vec2 fragmentUV;

// Values that stay constant for the whole mesh
uniform sampler2D myTextureSampler;

// Mouse Input Co-ordinates
uniform vec2 mousept;

// Ouput data
out vec3 color;

void main()
{ 
      //color = vec3(fragmentUV,0.);

	  /*Question1
	  //Display only central view
	  //vec2 uv = fragmentUV/9.0;

	  // For change with respect to mouse co-ordinates
	  //vec2 uv = (fragmentUV/9.0)+(mousept/9.0);
	  */

	  /*
	  //Question2 Average all views of lightfield
	  color=vec3(0.0,0.0,0.0);
	  vec2 uv;
	  for(float i=0;i<9.0;i++){
	  for(float j=0;j<9.0;j++){
	  uv= ((fragmentUV/9.0)+vec2(i,j)/9.0);
	  color+=texture(myTextureSampler,uv).rgb;
	  }
	  }
	  color=color/81.0;
	  */

	  /*
	 // Question3 - Refocus by translating the views with respect to the central one
	 float alpha = mousept.y;	 
	 color = vec3(0.,0.,0.);
	 vec2 uv;
	 for (float i = 0; i < 9.0; i++) {
		for(float j = 0; j < 9.0; j++){
			uv = (fragmentUV /9.0) + (vec2(i,j) / 9.0);
			uv = uv + alpha*vec2(i-4, j-4);
			color += texture(myTextureSampler, uv).rgb;
		}
	  }
	  color = color / 81.0;
	 */
	
	  //Bonus 
      color = texture(myTextureSampler, fragmentUV).rgb;

	  //Question 1
	  //color = texture(myTextureSampler, uv).rgb;
      
}