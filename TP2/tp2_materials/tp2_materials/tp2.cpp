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

* Q1: We provide two functions to load and render a 3D model : 'loadIFS' reads the model from file, and 'createGeometry' convert the geometric data to OpenGL buffers of vertices and normals.
  Finally, 'mainRender' draws the triangles represented by the buffers.

  - Rotate the model with the mouse. What are the colors over the object? (see SimpleFragmentShader.fp)
    Answer: Color of the object varies proportional to the direction of the normal of each fragment. 
	For normal is pointing towards the x axis it will get a red coloring, and towards the y axis it will takes a green values, and 
	while towards the z axis it will be painted in blue.
  - In SimpleVertexShader.fp, uncomment [Q1] and see how it changes the colors of the object. What is the difference?
    Answer:In the previous question, the colors of the object changed proportional to the world coordinates, now
	the color changes proportional to the coordinates system of the camera.
	In other words, the color of the object changes proportional to the position of the camera.

* Q2. - Program a Phong model in the fragment shader
   
   Reminder:
      The Phong model computes the final color as:
         (-L.N)*diffuse_color + ( (R.V)^specular_exponent ) * specular_color
       where L is the incoming light direction, N the surface normals and R the
       reflected *light* direction. L,N and R are normalized (unit length).
       Use the GLSL function `reflect' to compute R in the fragment shader.
	   Answer : Phong model is given by the changesin SimpleFragmentShader.fp
	   float diffuse = max(dot(-L,N),0)
	   vec3 V = normalize(viewDirection)
	   vec3 R = normalize(reflect(L,N))
	   float specular = pow(max(dot(R,V),0),specular_exponent)
	   color = diffuse*vec3(0.8,0.6,0.6)+specular*vec3(0.7,0.2,0.2) 
   
* Q3.

 - Provide the specular exponent as a parameter of your shader. 
 - Use the keyboard callback function to increase ('+') or decrease ('-') the exponent. Use values in [1,2,4,8,16,32,64,128]
  Answer: Changes give by: 
  if (key == '+') {
  // Q3 - The key (+) increases the exponent to 128
  specular_exponent *= 2;
  specular_exponent = min(specular_exponent, 128);
  }
  else if (key == '-') {
  // Q3 - The key (-) decreases the exponent to 2
  specular_exponent /= 2;
  specular_exponent = max(specular_exponent, 1);
  }
  GLuint specularExponentID = glGetUniformLocation(g_glslProgram, "specular_exponent")
  glUniform1f(specularExponentID, specular_exponent)

* Q5.
  - Modify the Phong shader to render the object in a cartoon style. To do so, you need to quantize the diffuse term to be either 0.5 or 1 (ie if diff < 0.5 then diff = 0.5 etc) and and the specular term to 0 or 1
  
  - Draw the contours of the object in black. Note: the contour lies where the surface normal is perpendicular to the view direction. You will need to apply a threshold for the dot product (typically < 0.3)
  Answer:
  Tried to implmenent this using changes in SimpleFragmentShader.fp by this block:
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
* Q4.
  - Animate the light to orbit along a circle.
  Answer: Tried to implement this by following
  v3f lightDirection = V3F(cos(theta), 0.5, sin(theta))
  
  - Fix the light to the camera position.
  

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

//specular exponent
float specular_exponent=1;

//theta
float theta = 0;

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
	}else if (key == '+') {
		// Q3 - The key (+) increases the exponent to 128
		specular_exponent *= 2;
		specular_exponent = min(specular_exponent, 128);
	}
	else if (key == '-') {
		// Q3 - The key (-) decreases the exponent to 2
		specular_exponent /= 2;
		specular_exponent = max(specular_exponent, 1);
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
	glEnable(GL_DEPTH_TEST); 
	
	// use our shader
	glUseProgramObjectARB(g_glslProgram);
	
	theta += 0.001;
	
	
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
	
	// Light direction, expressed in *world* space
	//v3f lightDirection = V3F(0.3, 0.5, -1);
	v3f lightDirection = V3F(cos(theta), 0.5, sin(theta));
	GLuint lightDirectionID = glGetUniformLocation(g_glslProgram, "lightDirection");
	// To view space
	v4f ld4 = V4F(lightDirection[0], lightDirection[1], lightDirection[2], 1.);
	ld4 = MV*ld4;
	//  back to vec3
	lightDirection[0] = ld4[0]; lightDirection[1] = ld4[1]; lightDirection[2] = ld4[2];
	glUniform3fv(lightDirectionID, 1, &(lightDirection[0]));

	GLuint specularExponentID = glGetUniformLocation(g_glslProgram, "specular_exponent");
	glUniform1f(specularExponentID, specular_exponent);
	
	
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
	
	// 2nd attribute buffer : normals
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
  
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, g_NumIndices); // Starting from vertex 0; g_NumIndices vertices total
	
 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
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

/* The 'loadIFS' function loads an Indexed Face Set (see ifs.pdf) in the following variables:

    unsigned int    g_NumVerticies  = 0;      // number of verticies
    t_point        *g_Verticies     = NULL;   // verticies
    unsigned int    g_NumIndices    = 0;      // number of indices (= 3 * number of triangles)
    unsigned short *g_Indices       = NULL;   // indices, 3 per triangles
*/
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
	for(int i=0; i<g_NumIndices; i++){ //for all vertices
	    g_vertex_buffer_data[i*3] = g_Verticies[g_Indices[i]].p[0]; // x coordinate
	    g_vertex_buffer_data[i*3+1] = g_Verticies[g_Indices[i]].p[1]; // y coordinate
	    g_vertex_buffer_data[i*3+2] = g_Verticies[g_Indices[i]].p[2]; // z coordinate
	}
	
	// Normals
	GLfloat* g_normal_buffer_data;
	g_normal_buffer_data = new GLfloat[3*g_NumIndices];
	for(int i=0; i<g_NumIndices; i++){
	    g_normal_buffer_data[i*3] = g_Verticies[g_Indices[i]].n[0];
	    g_normal_buffer_data[i*3+1] = g_Verticies[g_Indices[i]].n[1];
	    g_normal_buffer_data[i*3+2] = g_Verticies[g_Indices[i]].n[2];
	  
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
	//same for normals...
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3*g_NumIndices*sizeof(GLfloat), g_normal_buffer_data, GL_STATIC_DRAW); 
	
	
	//clean up
	delete [] g_vertex_buffer_data;
	delete [] g_normal_buffer_data;
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
	g_MainWindow=glutCreateWindow("TP3");
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
	/// Shaders, geometry and camera
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

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	cleanUp();
}

/* -------------------------------------------------------- */

