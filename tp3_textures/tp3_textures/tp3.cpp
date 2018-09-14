/* -------------------------------------------------------- */
/* -------------------------------------------------------- */
/*

TP OpenGL - Image Synthesis course

George Drettakis, Adrien Bousseau

mailto: FirstName.LastName@inria.fr

REVES / INRIA

History:
- Adapted to OpenGL 3, 2014-02 (AB)

Documentation:
- OpenGL tutorial, provides detailed instructions but relies on a different representation of matrices and 3D models
http://www.opengl-tutorial.org/beginners-tutorials/
- GLSL reference
https://www.opengl.org/documentation/glsl/


Questions:

* Q1:	- Read how we add a buffer to send the texture coordinates (markers [Q1a] and [Q1b]).
	- Receive and interpolate the texture coordinates in the vertex shader (markers [Q1c]).
	- Display the texture coordinates as color (r = u, g = v, b = 0) in the fragment shader. What can you guess about the parameterization?
	
	**Answer:
	Q1a: Here the vertex buffer data g_uv_buffer_data pointer is declared and then values are filled from g_Verticies values
	glGenBuffers --> generate the buffers from memory location uvbuffer
	Then the buffer is bind into GL_ARRAY_BUFFER target, for to be used as when called by glVertexAttribPointer.

	Q1b: Here the the Vertex Array is enabled for location 2 using glEnableVertexAttribArray and the buffer is bound for uv buffer.
	Vertex Array Attributes are defined with 2, size as 2 and type as GL_FLOAT and non-normalized.

	Q1c:
	Here the values of the uv buffer values are obtained in the vertexshader as -->layout(location =2) in vec2 intexture_coordinates;
	Then this value is passed out as outtexture_coordinates.
	This value is passed into Fragment shader
	The object is flattened and then cut into pieces which is then divided into parts in 2D space.Colour change is applied as
	a function of co-ordinates within the texture space.Areas of the object near to the texture-co-ordinate has high intnsity while the farther
	one has low intensity of colour.

* Q2: Function 'createCheckerBoardTexture' generates a checkerboard texture (it returns the texture OpenGL id)

  - Uncomment [Q2a] to create the texture
  - Uncomment [Q2b] in the cpp file and the pixel shader to send the texture id to your shader
  - Modify your fragment shader to read the texture using the UV coordinates (see course slides about texturing in OpenGL)
  - At [Q2c], change GL_NEAREST to GL_LINEAR, test the difference
  
  **Answer:
  Here the texture(checkerboard) is created which is a identifier: GLuint and then the textureID value
  is then created by binding the texture getting a uniform variable myTextureSampler.
  This texture is finally applied in the fragment shader by passing the texture-coordinates,mytexturesampler and converting to a rgb value.
  When changing from GL_NEAREST to GL_LINEAR we are changing the MIP Mapping be passing a different value of filter:
  GL_NEAREST which return the value of texture element that is nearest to center of pixel.
  GL_LINEAR which then returns a weighted average of the 4 elements that are closest to center being textured.

* Q3:  In the fragment program, apply different Phong parameters to the white and black pixels of the textured model. 
  For example, make the white pixels shiny orange and the black pixels diffuse dark red.

  **Answer:Here Phong parameter is defined by passing the reflection, view direction which are first normalized before being passed.
  Then the color check is performed on the output, if white then orange color(1,0,5,0) is passed and if black then dark red value(1,0,0)
  is passed.The variance in shiny can be obtained by passing different values of spectral constant q.
  
* Q4: Procedural texturing: in the fragment program, compute the surface color as concentric circles of
   alterning green/blue centered on 0,0,0 (Hint: You need to pass the 3D world coordinates
   of the vertices to the fragment progam).

   ** Answer:
   Here 3D World Co-ordinates are passed from the vertexPosition_Modelspace from the vertex shader to the fragment shader.
   Then as we know the radius^2 of the sphere is given by x^2+y^2, we get x and the y values passed into from the 3d world, then radius is rouded to the nearest value.
   The sphere value is then checked for the colours and then changed to alternate green and blue.
   
* Q5: Environment mapping: use the reflection vector to render the texture as a reflected environment.
  You first need to uncomment [Q5] (and comment the preceding line) to use a different texture
  In the fragment shader, you need to convert the 3D reflection vector to 2D spherical coordinates, using the formula (see "panoramic format" http://gl.ict.usc.edu/Data/HighResProbes/)
  theta = 0.5*(1 + atan(R.x,-R.z) / 3.1416);
  phi = acos(R.y) / 3.1416;
  Finally, read the texture with the spherical coordinates

 ** Answer:
 Here a new Texture is created which is environment mapping and then the light probe calculation of incident illumination condition in space is 
 used to calculate the 2d spherical co-ordinates in Panoramic Format .This panaromic vector in 2D is then passed to the texture to get the required texture output.
*/

#ifdef _WIN32
#include <windows.h>        // windows API header (required by gl.h)
#endif

#include "glew/include/GL/glew.h"	// Support for shaders

#ifdef __APPLE__

#include <gl3.h>          // OpenGL header
#include <gl3ext.h>          // OpenGL header
#include <glut.h>        // OpenGL Utility Toolkit header

#else

#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header

#endif

#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "glsl.h" // GLSL Helper functions
#include "Matrix4x4.h" //class to represent matrices
#include "Vertex.h" //class to represent vertices
#include "Trackball.h" // trackball

//path of the shaders
#define SRC_PATH	""

#include <iostream>
using namespace std;

/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window width


/* --------------------- Geometry ------------------- */

//Vertex Array Object
GLuint VertexArrayID;
// This will identify our vertex buffer
GLuint vertexbuffer;
// This will identify our normal buffer
GLuint normalbuffer;
// This will identify our texture coordinate buffer
GLuint uvbuffer;

// This will identify the texture
GLuint texture;

//---- IFS representation ---
typedef struct s_point
{
  float p[3]; //position
  float n[3]; //normal
  float uv[2]; //texture coordinate
} t_point;

t_point        *g_Verticies     = NULL; //list of vertices
unsigned short *g_Indices       = NULL; //list of faces, 3 indices per face
unsigned int    g_NumVerticies  = 0; //number of vertices
unsigned int    g_NumIndices    = 0; //number of indices, i.e. 3 times the number of faces


/*---------------------- Shaders -------------------*/
GLuint g_glslProgram;


/* -------------------------------------------------------- */

void mainKeyboard(unsigned char key, int x, int y) 
{
	if (key == 'q') {
		exit (0);
	} else if (key == ' ') {
		printf("spacebar pressed\n");
	} 

	printf("key '%c' pressed\n",key);
}

/* -------------------------------------------------------- */

void mainMouse(int btn, int state, int x, int y) 
{
	if (state == GLUT_DOWN) {
		trackballButtonPressed(btn,x,y);
	} else if (state == GLUT_UP) {
		trackballButtonReleased(btn);
	}
}

/* -------------------------------------------------------- */

void mainMotion(int x,int y)
{
	trackballUpdate(x,y);
}

/* -------------------------------------------------------- */

void mainReshape(int w,int h)
{
	printf("Resizing window to %d,%d\n",w,h);
	g_W=w;
	g_H=h;
	// set viewport to the entire window
	glViewport(0,0,g_W,g_H);
}


/* -------------------------------------------------------- */

void mainRender()
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST); //[Q1c]
	
	// use our shader
	glUseProgramObjectARB(g_glslProgram);
	
	
	
	//--- Camera ---//
	// View: get trackball transform
	m4x4f View = trackballMatrix();
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	m4x4f Projection = perspectiveMatrix<float>(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Model matrix : an identity matrix (model will be at the origin)
	m4x4f Model;
	Model.eqIdentity();
	
	// Our ModelViewProjection : multiplication of our 3 matrices
	m4x4f MVP = Projection*View*Model; // Remember, matrix multiplication is the other way around
	MVP = MVP.transpose();
	
	// ModelView
	m4x4f MV = View*Model;
	MV = MV.transpose();
	
	// Get a handle for our "MVP" uniform (identify the model-view-projection matrix in the shader)
	GLuint MatrixID = glGetUniformLocation(g_glslProgram, "MVP");
	// Send our transformation to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(MVP[0]));
	
	// Get a handle for our "MV" uniform (identify the view matrix in the shader)
	GLuint MatrixViewID = glGetUniformLocation(g_glslProgram, "MV");
	// Send our transformation to the currently bound shader, in the "MV" uniform
	glUniformMatrix4fv(MatrixViewID, 1, GL_FALSE, &(MV[0]));
	
	// Light direction, expressed in *view* space
	v3f lightDirection = View.mulVector(V3F(0, 0, -1));
	GLuint lightDirectionID = glGetUniformLocation(g_glslProgram, "lightDirection");
	glUniform3fv(lightDirectionID, 1, &(lightDirection[0]));
	
	
	 //--- Texturing, [Q2b]
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(g_glslProgram, "myTextureSampler");
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureID, 0); 
	
	
	//--- Geometry ---//
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	  3,                  // size, 3 coordinates per vertex (x,y,z)
	  GL_FLOAT,           // type
	  GL_FALSE,           // normalized?
	  0,                  // stride
	  (void*)0            // array buffer offset
	);
	
	// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
	    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	    3,                                // size, 3 coordinate per normal
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized? No, it will be normalized in the shader
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
	
	// 3nd attribute buffer : UVs [Q1b]
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
                          2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                          2,                                // size, 2 components for texture coordinates
                          GL_FLOAT,                         // type
                          GL_FALSE,                         // normalized? No
                          0,                                // stride
                          (void*)0                          // array buffer offset
                          );
  
	
	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, g_NumIndices);
 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
	
	
	// swap - this call exchanges the back and front buffer
	// swap is synchronized on the screen vertical sync
	glutSwapBuffers();
}

/* -------------------------------------------------------- */

void idle( void )
{
	// whenever the application has free time, ask for a screen refresh
	glutPostRedisplay();
}

/* -------------------------------------------------------- */

void loadShaders()
{
	const char *fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader.fp");
	const char *vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader.vp");
	g_glslProgram = createGLSLProgram(vp_code,fp_code);
	delete [](fp_code);
	delete [](vp_code);
}

/* -------------------------------------------------------- */

// Load an IFS model
void loadIFS(const char *filename)
{
  // open file
  FILE *f=fopen(filename,"rb");
  if (f == NULL) {
    // error?
    cerr << "[loadIFS] Cannot load " << filename << endl;
  }
  // read vertices
  fread(&g_NumVerticies,sizeof(unsigned int),1,f);
  g_Verticies = new t_point[g_NumVerticies];
  fread(g_Verticies,sizeof(t_point),g_NumVerticies,f);
  // read indices
  fread(&g_NumIndices,sizeof(unsigned int),1,f);
  g_Indices = new unsigned short[g_NumIndices];
  fread(g_Indices,sizeof(unsigned short),g_NumIndices,f);
  // close file
  fclose(f);
  // print mesh info
  cerr << g_NumVerticies << " points " << g_NumIndices/3 << " triangles" << endl;
  // done.
}

/* -------------------------------------------------------- */

void createGeometry()
{
	loadIFS("test.mesh");
	
	// Position
	GLfloat* g_vertex_buffer_data;
	g_vertex_buffer_data = new GLfloat[3*g_NumIndices];
	for(int i=0; i<g_NumIndices; i++){
	    g_vertex_buffer_data[i*3] = g_Verticies[g_Indices[i]].p[0];
	    g_vertex_buffer_data[i*3+1] = g_Verticies[g_Indices[i]].p[1];
	    g_vertex_buffer_data[i*3+2] = g_Verticies[g_Indices[i]].p[2];
	}
	
	// Normals
	GLfloat* g_normal_buffer_data;
	g_normal_buffer_data = new GLfloat[3*g_NumIndices];
	for(int i=0; i<g_NumIndices; i++){
	    g_normal_buffer_data[i*3] = g_Verticies[g_Indices[i]].n[0];
	    g_normal_buffer_data[i*3+1] = g_Verticies[g_Indices[i]].n[1];
	    g_normal_buffer_data[i*3+2] = g_Verticies[g_Indices[i]].n[2];
	}
	
	// UVs [Q1a]
	GLfloat* g_uv_buffer_data;
    g_uv_buffer_data = new GLfloat[2*g_NumIndices];
    for(int i=0; i<g_NumIndices; i++){
        g_uv_buffer_data[i*2] = g_Verticies[g_Indices[i]].uv[0];
        g_uv_buffer_data[i*2+1] = g_Verticies[g_Indices[i]].uv[1];
    }
	

	//--- Send the geometry to OpenGL
	// We need a vertex array object
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, 3*g_NumIndices*sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW);
	
	//same for colors...
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3*g_NumIndices*sizeof(GLfloat), g_normal_buffer_data, GL_STATIC_DRAW);
	
	//same for uv... [Q1a]
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, 2*g_NumIndices*sizeof(GLfloat), g_uv_buffer_data, GL_STATIC_DRAW);
	
	//clean up
	delete [] g_vertex_buffer_data;
	delete [] g_normal_buffer_data;
	//same for uv... [Q1a]
    delete [] g_uv_buffer_data;
}

/* -------------------------------------------------------- */

GLuint createCheckerBoardTexture()
{
  static int res=16;
  GLuint id=0;
  glGenTextures(1,&id);
  glBindTexture(GL_TEXTURE_2D,id);
  unsigned char *data=new unsigned char[res*res*4];
  for (int j=0;j<res;j++) {
    for (int i=0;i<res;i++) {
      data[(i+j*res)*4+0] = ((i+j)&1) ? 255 : 0;
      data[(i+j*res)*4+1] = ((i+j)&1) ? 255 : 0;
      data[(i+j*res)*4+2] = ((i+j)&1) ? 255 : 0;
      data[(i+j*res)*4+3] = 255;
    }
  }
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,res,res,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); //[Q2c] Reverting GL_LINEAR to GL_NEAREST since for Phong shading question
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  delete [](data);
  return (id);
}

GLuint createEnvironmentTexture()
{
  static int res=64;
  GLuint id=0;
  glGenTextures(1,&id);
  glBindTexture(GL_TEXTURE_2D,id);
  unsigned char *data=new unsigned char[res*res*4];
  for (int i=0;i<res;i++) {
    for (int j=0;j<0.5*res;j++) {
      data[(i+j*res)*4+0] = 255-255*j/res;
      data[(i+j*res)*4+1] = 255-255*j/res;
      data[(i+j*res)*4+2] = 200;
      data[(i+j*res)*4+3] = 255;
    }
    for (int j=0.5*res;j<res;j++) {
      data[(i+j*res)*4+0] = 200*j/res;
      data[(i+j*res)*4+1] = 120*j/res;
      data[(i+j*res)*4+2] = 100*j/res;
      data[(i+j*res)*4+3] = 255;
    }
  }
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,res,res,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  delete [](data);
  return (id);
}

/* -------------------------------------------------------- */

void cleanUp()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}

/* -------------------------------------------------------- */

int
main(int argc, char **argv) 
{
	///
	///  glut Window
	///
	// main glut init
	glutInit(&argc, argv);
	// initial window size
	glutInitWindowSize(g_W,g_H); 
	// init display mode
#ifdef __APPLE__
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE |  GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	// create main window
	g_MainWindow=glutCreateWindow("TP2");
	// set main window as current window
	glutSetWindow(g_MainWindow);
	/// setup glut callbacks
	// mouse (whenever a button is pressed)
	glutMouseFunc(mainMouse);
	// motion (whenever the mouse is moved *while* a button is down)
	glutMotionFunc(mainMotion);
	// keyboard (whenever a character key is pressed)
	glutKeyboardFunc(mainKeyboard);
	// display  (whenerver the screen needs to be painted)
	glutDisplayFunc(mainRender);
	// reshape (whenever the window size changes)
	glutReshapeFunc(mainReshape);
	// idle (whenever the application as some free time)
	glutIdleFunc(idle);

	///
	/// Shaders, geometry, texture and camera
	///
	
	//need to init Glew before anything else
#ifdef __APPLE__
    glewExperimental = GL_TRUE;
#endif
	glewInit();
	
	loadShaders();
	
	//Send the geometry to OpenGL
	createGeometry();
	
	// Trackball init (controls the camera)
	trackballInit(g_W,g_H);
	trackballTranslation() = V3F(0.f,0.f,-3.f);
	
	// texture
	//texture = createCheckerBoardTexture(); //[Q2a]
	texture = createEnvironmentTexture(); //[Q5]

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	cleanUp();
}

/* -------------------------------------------------------- */

